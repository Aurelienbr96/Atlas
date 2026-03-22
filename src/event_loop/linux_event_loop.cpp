//
// Created by Aurélien Brachet on 17/03/2026.
//

#include "linux_event_loop.h"

#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

LinuxEventLoop::LinuxEventLoop() {
  epfd = epoll_create1(0);
  if (epfd == -1) {
    std::cerr << "epoll_create1 failed: " << std::strerror(errno) << "\n";
    std::exit(1);
  }
}

// Helper to handle the ADD vs MOD logic in epoll
void LinuxEventLoop::updateMask(int fd, uint32_t bit, bool add) {
  uint32_t& current_mask = fd_masks[fd];
  uint32_t old_mask = current_mask;

  if (add)
    current_mask |= bit;
  else
    current_mask &= ~bit;

  if (current_mask == old_mask) return;  // No change needed

  epoll_event ev{};
  ev.data.fd = fd;
  ev.events = current_mask;

  // If it was 0, it wasn't in epoll yet.
  // If it's the first time we see this FD, use ADD; otherwise MOD.
  int op = (old_mask == 0) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

  if (epoll_ctl(epfd, op, fd, &ev) == -1) {
    // Fallback: if ADD failed because it exists, try MOD
    if (errno == EEXIST) {
      epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    } else {
      std::cerr << "epoll_ctl failed for fd " << fd << ": " << std::strerror(errno) << "\n";
    }
  }
}

void LinuxEventLoop::pushReadEvent(int fd) {
  // Level-triggered is standard for servers
  updateMask(fd, EPOLLIN | EPOLLET, true);
}

void LinuxEventLoop::pushWriteEvent(int fd) {
  // EPOLLET (Edge Triggered) matches your Mac EV_CLEAR logic
  updateMask(fd, EPOLLOUT | EPOLLET, true);
}

void LinuxEventLoop::registerFd(int fd, Callback read, Callback write) {
  callbacks.insert_or_assign(fd, Channel{.read = read, .write = write});
}

void LinuxEventLoop::start() {
  std::vector<epoll_event> events(256);
  while (true) {
    int n = epoll_wait(epfd, events.data(), static_cast<int>(events.size()), -1);

    if (n == -1) {
      if (errno == EINTR) continue;
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
  if (it == callbacks.end()) return;

  // 1. Handle Errors/Hang-ups first
  if (ev.events & (EPOLLHUP | EPOLLERR)) {
    // Clean up state
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
    fd_masks.erase(fd);
    callbacks.erase(it);
    close(fd);
    return;
  }

  // 2. READ (Don't 'return' so we can process WRITE in the same loop)
  if (ev.events & EPOLLIN) {
    it->second.read();
  }

  // 3. WRITE
  if (ev.events & EPOLLOUT) {
    it->second.write();
  }
}
