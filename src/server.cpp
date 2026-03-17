//
// Created by Aurélien Brachet on 23/11/2025.
//

#include "server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <ranges>

#include "http/conn.h"
#include "http/http_parser.h"
#include "http/request.h"
#include "http/response.h"
#include "http/routes_registery.h"

using namespace std;

static bool set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return false;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

Server::Server(int port, RouteRegistery& routeRegistery, EventLoop* eventLoop)
    : port(port), eventLoop(eventLoop), routeRegistery(routeRegistery) {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket < 0) {
    perror("socket failed");
  }
  sockaddr_in serverAddress{};

  // to prevent "address already in use"

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(port);
  this->serverAddress = serverAddress;
  this->serverSocket = serverSocket;
  set_nonblocking(serverSocket);
  cout << "Server listening on port:" << port << endl;
}

Server::~Server() { close(this->serverSocket); }

void Server::run() {
  int opt = 1;
  if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    perror("setsockopt SO_REUSEADDR failed");
  }

  int res = ::bind(this->serverSocket, reinterpret_cast<sockaddr*>(&serverAddress),
                   sizeof(serverAddress));

  if (res < 0) {
    perror("bind failed");
  }
  listen(serverSocket, SOMAXCONN);

  eventLoop->pushReadEvent(serverSocket);

  eventLoop->registerFd(
      this->serverSocket,
      [this]() {
        while (true) {
          sockaddr_in caddr{};
          socklen_t clen = sizeof(caddr);
          int clientFd = ::accept(this->serverSocket, reinterpret_cast<sockaddr*>(&caddr), &clen);
          cout << "client fd : " << clientFd << endl;
          if (clientFd == -1) {
            cout << "breaking" << endl;
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "accept failed: " << strerror(errno) << "\n";
            break;
          }

          if (!set_nonblocking(clientFd)) {
            std::cerr << "accept failed: " << strerror(errno) << "\n";
            ::close(clientFd);
            continue;
          }
          HttpParser parser;

          conns.emplace(clientFd, Conn{.fd = clientFd, .parser = parser, .out = {}});

          // Read events: EV_CLEAR means "edge-ish": you must drain reads until EAGAIN.
          eventLoop->pushReadEvent(clientFd);

          eventLoop->registerFd(
              clientFd,
              [this, clientFd]() {
                // bool saw_eof = (ev.flags & EV_EOF) != 0;
                auto it = conns.find(clientFd);
                if (it == conns.end()) {
                  cout << "could not find event" << endl;
                  // Could be a stale event after close; ignore.
                  return;
                }
                Conn& c = it->second;
                cout << "found event to read" << endl;

                char buf[4096];
                while (true) {
                  ssize_t r = ::read(clientFd, buf, sizeof(buf));
                  if (r > 0) {
                    // Echo back: append to outbound buffer.
                    c.parser.feed(string(buf, static_cast<size_t>(r)));
                    cout << "reading from fd" << string(buf) << endl;
                    if (auto rqt = c.parser.constructRequest()) {
                      Request request = std::move(*rqt);
                      cout << request.getContentLength().value() << endl;
                      Response response(c.fd, eventLoop, &c);
                      handleRequest(request, response);
                      break;
                    }
                  } else if (r == 0) {
                    // Clean shutdown by peer.
                    // saw_eof = true;
                    break;
                  } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    std::cerr << "read fd=" << clientFd << strerror(errno) << "\n";
                    // saw_eof = true;
                    break;
                  }
                }

                // If we have data to send, enable WRITE interest.
                if (!c.out.empty()) {
                  eventLoop->pushWriteEvent(clientFd);
                }

                // if (saw_eof) {
                //   // eventLoop.close_conn(kq, conns, fd);
                // }
              },
              [this, clientFd]() {
                auto it = conns.find(clientFd);
                if (it == conns.end()) {
                  cout << "could not find event" << endl;
                  // Could be a stale event after close; ignore.
                  return;
                }
                Conn& c = it->second;

                while (!c.out.empty()) {
                  ssize_t w = ::write(clientFd, c.out.data(), c.out.size());
                  if (w > 0) {
                    c.out.erase(0, static_cast<size_t>(w));
                  } else if (w == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    break;
                  } else {
                    // std::cerr << "write fd=" << fd << ": " << std::strerror(errno) << "\n";
                    // saw_eof = true;
                    break;
                  }
                }
              });

          // Don't add WRITE yet; only when we have data to send.

          char ipbuf[INET_ADDRSTRLEN]{};
          inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
          std::cout << "Accepted fd=" << clientFd << " from " << ipbuf << ":"
                    << ntohs(caddr.sin_port) << "\n";
        }
      },
      []() {});
}

void Server::handleRequest(Request& request, Response& res) {
  const string& path = request.getPath();
  const string& method = request.getMethod();

  const Handler* handler = this->routeRegistery.getHandler(path, method);

  if (handler == nullptr) {
    res.setHttpStatus(HttpStatus::NotFound);
    res.end();
    return;
  }
  (*handler)(request, res);
  cout << "route found" << endl;
}

int Server::getPort() const { return this->port; }