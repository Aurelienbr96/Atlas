//
// Created by Aurélien Brachet on 23/11/2025.
//

#include "server.h"
#include "iostream"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ranges>
#include <sys/event.h>
#include <sys/fcntl.h>

#include "http/routes_registery.h"

#include "http/response.h"
#include "http/request.h"
#include "http/conn.h"
#include "http/http_parser.h"

using namespace std;


static bool set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return false;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}


Server::Server(int port, RouteRegistery& routeRegistery) {
    this->port = port;
    this->routeRegistery = routeRegistery;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket failed");
    }
    sockaddr_in serverAddress{};
    EventLoop eventLoop;
    this->eventLoop = eventLoop;

  // to prevent "address already in use"


    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    this->serverAddress = serverAddress;
    this->serverSocket = serverSocket;
    set_nonblocking(serverSocket);
    cout << "Server listening on port:" << port << endl;

}

Server::~Server() {
    close(this->serverSocket);
}


void Server::run() {
    int res = ::bind(this->serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),sizeof(serverAddress));

    eventLoop.addEvent({
        static_cast<uintptr_t>(this->serverSocket),
        EVFILT_READ,
        EV_ADD | EV_ENABLE,
        0,
        0,
        nullptr
    });


    if (res < 0) {
        perror("bind failed");
    }
    listen(serverSocket, SOMAXCONN);

    std::vector<struct kevent> events(256);
    std::unordered_map<int, Conn> conns;

    while (true) { // accept loop
        // one parser per incoming request
        // int clientSocket = ::accept(this->serverSocket, nullptr, nullptr);

        int n = kevent(eventLoop.getKqFb(), nullptr, 0, events.data(), static_cast<int>(events.size()), nullptr);
        cout << "number of events" << n << endl;
        if (n == -1) {
            if (errno == EINTR) continue;
            std::cerr << "kevent(wait): " << std::strerror(errno) << "\n";
            break;
        }
        for (int i = 0; i < n; i++) {
            const auto& ev = events[i];

      if (ev.flags & EV_ERROR) {
        // EV_ERROR on returned events: ev.data contains errno.
        // For example, EV_DELETE might produce benign errors if the fd is gone.
        int err = static_cast<int>(ev.data);
        if (err != 0) {
          std::cerr << "EV_ERROR ident=" << ev.ident << " filter=" << ev.filter
                    << " err=" << std::strerror(err) << "\n";
        }
        continue;
      }

      int fd = static_cast<int>(ev.ident);

      // Listener: accept as many as possible.
      if (fd == this->serverSocket && ev.filter == EVFILT_READ) {
        // we take as much connection as possible and we crash the loop when accept does not have new client
        while (true) {
          sockaddr_in caddr{};
          socklen_t clen = sizeof(caddr);
          int clientFd = ::accept(this->serverSocket, reinterpret_cast<sockaddr*>(&caddr), &clen);
            cout << "client fd : " << clientFd << endl;
          if (clientFd == -1) {
              cout << "breaking" << endl;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "accept: " << std::strerror(errno) << "\n";
            break;
          }

          if (!set_nonblocking(clientFd)) {
            std::cerr << "set_nonblocking(client): " << std::strerror(errno) << "\n";
            ::close(clientFd);
            continue;
          }
          HttpParser parser;

          conns.emplace(clientFd, Conn{.fd = clientFd,.parser=parser, .out = {}});

          // Read events: EV_CLEAR means "edge-ish": you must drain reads until EAGAIN.
          eventLoop.addEvent({
                             static_cast<uintptr_t>(clientFd),
                             EVFILT_READ,
                             EV_ADD | EV_ENABLE | EV_CLEAR,
                             0,
                             0,
                             nullptr});

          // Don't add WRITE yet; only when we have data to send.

          char ipbuf[INET_ADDRSTRLEN]{};
          inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
          std::cout << "Accepted fd=" << clientFd << " from " << ipbuf << ":" << ntohs(caddr.sin_port) << "\n";
        }
        continue;
      }

      auto it = conns.find(fd);
      if (it == conns.end()) {
        cout << "could not find event" << endl;
        // Could be a stale event after close; ignore.
        continue;
      }
      Conn& c = it->second;
          cout << "found event to read" << endl;

      // Peer closed? EV_EOF may appear on READ or WRITE filters.
      // Still attempt to drain reads (there may be remaining data) then close.
      bool saw_eof = (ev.flags & EV_EOF) != 0;

      if (ev.filter == EVFILT_READ) {
        char buf[4096];
        while (true) {
          ssize_t r = ::read(fd, buf, sizeof(buf));
          if (r > 0) {
            // Echo back: append to outbound buffer.
            c.parser.feed(string(buf, static_cast<size_t>(r)));
            cout << "reading from fd" << string(buf) << endl;
            if (auto rqt = c.parser.constructRequest()) {
              Request request = std::move(*rqt);
              cout << request.getContentLength().value() << endl;
              Response response(c.fd, &eventLoop, &c);
              handleRequest(request, response);
              break;
            }
          } else if (r == 0) {
            // Clean shutdown by peer.
            saw_eof = true;
            break;
          } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "read fd=" << fd << ": " << std::strerror(errno) << "\n";
            saw_eof = true;
            break;
          }
        }

        // If we have data to send, enable WRITE interest.
        if (!c.out.empty()) {
          eventLoop.addEvent({
                             static_cast<uintptr_t>(fd),
                             EVFILT_WRITE,
                             EV_ADD | EV_ENABLE | EV_CLEAR,
                             0,
                             0,
                             nullptr});
        }

        if (saw_eof) {
         //eventLoop.close_conn(kq, conns, fd);
        }
      } else if (ev.filter == EVFILT_WRITE) {
        while (!c.out.empty()) {
          cout << "writing" << endl;
          cout << "writing:" << c.out.data() << endl;
          ssize_t w = ::write(fd, c.out.data(), c.out.size());
          if (w > 0) {
            c.out.erase(0, static_cast<size_t>(w));
          } else if (w == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            break;
          } else {
            std::cerr << "write fd=" << fd << ": " << std::strerror(errno) << "\n";
            saw_eof = true;
            break;
          }
        }

        // If nothing left to write, disable WRITE to avoid wakeups.
        if (c.out.empty()) {
          eventLoop.addEvent({static_cast<uintptr_t>(fd), EVFILT_WRITE, EV_DELETE, 0, 0, nullptr});
          close(c.fd);
        }

        if (saw_eof) {
          //close_conn(kq, conns, fd);
        }
      }
    }
        }
}


void Server::handleRequest(Request& request, Response& res) {
    string& s = request.getPath();
    Handler* handler = this->routeRegistery.getHandler(s);
    if (handler == nullptr) {
        cout << "route not found" << endl;
        return;
    }
    (*handler)(request, res);
    cout << "route found" << endl;
}

int Server::getPort() const {
    return this->port;
}