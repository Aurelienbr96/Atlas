//
// Created by Aurélien Brachet on 17/03/2026.
//

#ifndef LINUX_EVENT_LOOP_H
#define LINUX_EVENT_LOOP_H

#include <sys/epoll.h>

#include <functional>
#include <unordered_map>

#include "event_loop.h"

class LinuxEventLoop {
 public:
  struct Channel {
    Callback read;
    Callback write;
  };

  LinuxEventLoop();

  void pushReadEvent(int fd);
  void pushWriteEvent(int fd);

  void registerFd(int fd, Callback read, Callback write);

  void start();

 private:
  void handleEvent(const epoll_event& ev);

  int epfd;
  std::unordered_map<int, Channel> callbacks;
};

#endif  // LINUX_EVENT_LOOP_H
