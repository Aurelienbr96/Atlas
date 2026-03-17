//
// Created by Aurélien Brachet on 17/03/2026.
//

#include "macos_event_loop.h"

#include <sys/event.h>

#include <iostream>

#include "vector"

MacOsEventLoop::MacOsEventLoop() {
  const int kq = kqueue();
  this->kq = kq;
}

void MacOsEventLoop::pushReadEvent(int fd) {
  struct kevent kev;
  EV_SET(&kev, static_cast<uintptr_t>(fd), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr);

  if (kevent(this->kq, &kev, 1, nullptr, 0, nullptr) == -1) {
    std::cerr << "kevent read (ADD/DEL) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
};

void MacOsEventLoop::pushWriteEvent(int fd) {
  struct kevent kev;
  EV_SET(&kev, static_cast<uintptr_t>(fd), EVFILT_WRITE, EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0,
         nullptr);

  if (kevent(this->kq, &kev, 1, nullptr, 0, nullptr) == -1) {
    std::cerr << "kevent write (ADD/DEL) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
};

void MacOsEventLoop::start() {
  std::vector<struct kevent> events(256);

  while (true) {
    int n = kevent(kq, nullptr, 0, events.data(), static_cast<int>(events.size()), nullptr);
    std::cout << "number of events" << n << std::endl;
    if (n == -1) {
      if (errno == EINTR) continue;
      std::cerr << "kevent(wait): " << std::strerror(errno) << "\n";
      break;
    }
    for (int i = 0; i < n; i++) {
      const auto& ev = events[i];
      this->handleEvent(ev);
    }
  }
}

void MacOsEventLoop::handleEvent(const struct kevent& ev) {
  const int fd = static_cast<int>(ev.ident);
  const auto it = this->callbacks.find(fd);
  if (it == this->callbacks.end()) {
    std::cout << "could not find callbacks for fd" << fd << std::endl;
    return;
  }
  if (ev.filter == EVFILT_READ) {
    it->second.read();
    return;
  }
  if (ev.filter == EVFILT_WRITE) {
    it->second.write();
  }
}

void MacOsEventLoop::registerFd(const int fd, Callback read, Callback write) {
  callbacks.insert_or_assign(fd, Channel{.read = read, .write = write});
}
