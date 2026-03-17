//
// Created by Aurélien Brachet on 17/03/2026.
//

#ifndef MACOS_EVENT_LOOP_H
#define MACOS_EVENT_LOOP_H

#include "event_loop/event_loop.h"

class MacOsEventLoop : public EventLoop {
  int kq;
  std::unordered_map<int, Channel> callbacks;
  void handleEvent(const struct kevent& ev);

 public:
  MacOsEventLoop();
  void pushReadEvent(int fd) override;
  void pushWriteEvent(int fd) override;
  void registerFd(int fd, Callback read, Callback write) override;
  void start() override;
};

#endif  // MACOS_EVENT_LOOP_H
