//
// Created by Aurélien Brachet on 27/02/2026.
//

#include "event_loop.h"

#include <sys/event.h>

#include <iostream>

#include "vector"

EventLoop::EventLoop() {
  const int kq = kqueue();
  this->kq = kq;
}

void EventLoop::addEvent(const Event& event) const {
  struct kevent kev;
  std::cout << "push event" << event.ident << std::endl;
  EV_SET(&kev, event.ident, event.filter, event.flags, event.fflags, event.data, event.udata);
  if (kevent(this->kq, &kev, 1, nullptr, 0, nullptr) == -1) {
    std::cerr << "kevent legacy(ADD/DEL) failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
};

void EventLoop::start() {
  std::vector<struct kevent> events(256);

  while (true) {
    int n = kevent(this->getKqFb(), nullptr, 0, events.data(), static_cast<int>(events.size()),
                   nullptr);
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

void EventLoop::handleEvent(const struct kevent& ev) {
  const int fd = static_cast<int>(ev.ident);
  const auto it = this->callbacks.find(fd);
  if (it == this->callbacks.end()) {
    std::cout << "could not find callbacks for fd" << fd << std::endl;
    return;
  }
  if (ev.filter == EVFILT_READ) {
    it->second.read(ev);
    return;
  }
  if (ev.filter == EVFILT_WRITE) {
    it->second.write(ev);
  }
}

void EventLoop::registerFd(const int fd, Callback read, Callback write) {
  callbacks.insert_or_assign(fd, Channel{.read = read, .write = write});
}

int EventLoop::getKqFb() const { return this->kq; }

/*
* eventLoop.addEvent(
      {static_cast<uintptr_t>(this->serverSocket), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, nullptr});

eventLoop.addEvent({static_cast<uintptr_t>(clientFd), EVFILT_READ,
                              EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, nullptr});
 *
 *
*    eventLoop.addEvent({static_cast<uintptr_t>(clientFd), EVFILT_WRITE,
                                      EV_ADD | EV_ENABLE | EV_CLEAR, 0, 0, nullptr});
 *
 */