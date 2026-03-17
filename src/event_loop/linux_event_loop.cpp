//
// Created by Aurélien Brachet on 17/03/2026.
//

#include "linux_event_loop.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

#include "event_loop.h"

LinuxEventLoop::LinuxEventLoop() {
  epfd = epoll_create1(0);
  if (epfd == -1) {
    std::cerr << "epoll_create1 failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
}

void LinuxEventLoop::pushReadEvent(int fd) {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.events = EPOLLIN;  // level-triggered (like your server socket)

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    std::cerr << "epoll_ctl ADD (read) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
}

void LinuxEventLoop::pushWriteEvent(int fd) {
  epoll_event ev{};
  ev.data.fd = fd;
  ev.events = EPOLLOUT | EPOLLET;  // edge-triggered (like your EV_CLEAR)

  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    std::cerr << "epoll_ctl ADD (write) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
}

void LinuxEventLoop::registerFd(int fd, Callback read, Callback write) {
  callbacks.insert_or_assign(fd, Channel{.read = read, .write = write});
}

void LinuxEventLoop::start() {
  std::vector<epoll_event> events(256);

  while (true) {
    int n = epoll_wait(epfd, events.data(), static_cast<int>(events.size()), -1);

    std::cout << "number of events " << n << std::endl;

    if (n == -1) {
      if (errno == EINTR) continue;
      std::cerr << "epoll_wait failed: " << std::strerror(errno) << "\n";
      break;
    }

    for (int i = 0; i < n; i++) {
      handleEvent(events[i]);
    }
  }
}

void LinuxEventLoop::handleEvent(const epoll_event& ev) {
  int fd = ev.data.fd;

  auto it = callbacks.find(fd);
  if (it == callbacks.end()) {
    std::cout << "could not find callbacks for fd " << fd << std::endl;
    return;
  }

  // READ
  if (ev.events & EPOLLIN) {
    it->second.read();
  }

  // WRITE
  if (ev.events & EPOLLOUT) {
    it->second.write();
  }

  // Optional: detect disconnects (very useful)
  if (ev.events & (EPOLLHUP | EPOLLERR)) {
    std::cout << "fd closed or error: " << fd << std::endl;
    close(fd);
    callbacks.erase(fd);
  }
}