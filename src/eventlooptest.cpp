//
// Created by Aurélien Brachet on 26/02/2026.
//

// kqueue_echo_server.cpp
// Minimal non-blocking TCP echo server using kqueue on macOS.
// Build:  clang++ -std=c++20 -O2 -Wall -Wextra kqueue_echo_server.cpp -o kqecho
// Run:    ./kqecho 127.0.0.1 5555
//
// Notes:
// - Uses EV_CLEAR for READ events so you must drain the socket until EAGAIN.
// - Enables WRITE interest only when there is pending outbound data.
// - Handles EV_EOF (peer closed) and EV_ERROR.

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

static bool set_nonblocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) return false;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

static void add_event(int kq, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags,
                      intptr_t data, void* udata) {
  struct kevent kev;
  EV_SET(&kev, ident, filter, flags, fflags, data, udata);
  if (kevent(kq, &kev, 1, nullptr, 0, nullptr) == -1) {
    std::cerr << "kevent(ADD/DEL) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
}

struct Conn {
  int fd;
  std::string out;  // pending bytes to write
};

static int make_listener(const std::string& ip, uint16_t port) {
  int fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    std::cerr << "socket: " << std::strerror(errno) << "\n";
    std::exit(1);
  }

  int yes = 1;
  // to prevent "address already in use"
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    std::cerr << "setsockopt(SO_REUSEADDR): " << std::strerror(errno) << "\n";
    std::exit(1);
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
    std::cerr << "inet_pton failed for '" << ip << "'\n";
    std::exit(1);
  }

  if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
    std::cerr << "bind: " << std::strerror(errno) << "\n";
    std::exit(1);
  }

  if (listen(fd, 128) == -1) {
    std::cerr << "listen: " << std::strerror(errno) << "\n";
    std::exit(1);
  }

  if (!set_nonblocking(fd)) {
    std::cerr << "set_nonblocking(listener): " << std::strerror(errno) << "\n";
    std::exit(1);
  }

  return fd;
}

static void close_conn(int kq, std::unordered_map<int, Conn>& conns, int fd) {
  // Remove filters (safe even if already gone; errors are handled by kernel as EV_ERROR on delete
  // sometimes).
  add_event(kq, static_cast<uintptr_t>(fd), EVFILT_READ, EV_DELETE, 0, 0, nullptr);
  add_event(kq, static_cast<uintptr_t>(fd), EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
  ::close(fd);
  conns.erase(fd);
}

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <ip> <port>\n";
    return 1;
  }

  std::string ip = argv[1];
  uint16_t port = static_cast<uint16_t>(std::stoi(argv[2]));

  int listener = make_listener(ip, port);

  int kq = kqueue();
  if (kq == -1) {
    std::cerr << "kqueue: " << std::strerror(errno) << "\n";
    return 1;
  }

  // Register listener readability (incoming accepts).
  add_event(kq, static_cast<uintptr_t>(listener), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);

  std::unordered_map<int, Conn> conns;
  std::vector<struct kevent> events(256);

  std::cout << "Listening on " << ip << ":" << port << "\n";

  while (true) {
    // we received N event ?
    int n = kevent(kq, nullptr, 0, events.data(), static_cast<int>(events.size()), nullptr);
    if (n == -1) {
      if (errno == EINTR) continue;
      std::cerr << "kevent(wait): " << std::strerror(errno) << "\n";
      break;
    }

    // we map over our event
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
      if (fd == listener && ev.filter == EVFILT_READ) {
        // we take as much connection as possible and we crash the loop when accept does not have
        // new client
        while (true) {
          sockaddr_in caddr{};
          socklen_t clen = sizeof(caddr);
          int cfd = ::accept(listener, reinterpret_cast<sockaddr*>(&caddr), &clen);
          if (cfd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            std::cerr << "accept: " << std::strerror(errno) << "\n";
            break;
          }

          if (!set_nonblocking(cfd)) {
            std::cerr << "set_nonblocking(client): " << std::strerror(errno) << "\n";
            ::close(cfd);
            continue;
          }

          conns.emplace(cfd, Conn{.fd = cfd, .out = {}});

          // Read events: EV_CLEAR means "edge-ish": you must drain reads until EAGAIN.
          add_event(kq, static_cast<uintptr_t>(cfd), EVFILT_READ, EV_ADD | EV_ENABLE | EV_CLEAR, 0,
                    0, nullptr);

          // Don't add WRITE yet; only when we have data to send.

          char ipbuf[INET_ADDRSTRLEN]{};
          inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
          std::cout << "Accepted fd=" << cfd << " from " << ipbuf << ":" << ntohs(caddr.sin_port)
                    << "\n";
        }
        continue;
      }

      auto it = conns.find(fd);
      if (it == conns.end()) {
        // Could be a stale event after close; ignore.
        continue;
      }
      Conn& c = it->second;

      // Peer closed? EV_EOF may appear on READ or WRITE filters.
      // Still attempt to drain reads (there may be remaining data) then close.
      bool saw_eof = (ev.flags & EV_EOF) != 0;

      if (ev.filter == EVFILT_READ) {
        char buf[4096];
        while (true) {
          ssize_t r = ::read(fd, buf, sizeof(buf));
          if (r > 0) {
            // Echo back: append to outbound buffer.
            c.out.append(buf, static_cast<size_t>(r));
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
          add_event(kq, static_cast<uintptr_t>(fd), EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0,
                    0, nullptr);
        }

        if (saw_eof) {
          close_conn(kq, conns, fd);
        }
      } else if (ev.filter == EVFILT_WRITE) {
        while (!c.out.empty()) {
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
          add_event(kq, static_cast<uintptr_t>(fd), EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        }

        if (saw_eof) {
          close_conn(kq, conns, fd);
        }
      }
    }
  }

  ::close(listener);
  ::close(kq);
  return 0;
}