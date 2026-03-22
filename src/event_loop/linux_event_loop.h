//
// Created by Aurélien Brachet on 17/03/2026.
//

#ifndef LINUX_EVENT_LOOP_H
#define LINUX_EVENT_LOOP_H

#include <sys/epoll.h>

#include <cstdint>
#include <unordered_map>

#include "event_loop.h"

class LinuxEventLoop : public EventLoop {
  int epfd;
  std::unordered_map<int, Channel> callbacks;
  std::unordered_map<int, uint32_t> fd_masks;
  void handleEvent(const epoll_event& ev);

 public:
  LinuxEventLoop();
  void updateMask(int fd, uint32_t bit, bool add);
  void pushReadEvent(int fd);
  void pushWriteEvent(int fd);
  void registerFd(int fd, Callback read, Callback write);
  void start();
};

#endif  // LINUX_EVENT_LOOP_H
