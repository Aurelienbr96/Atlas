//
// Created by Aurélien Brachet on 17/03/2026.
//

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <functional>

using Callback = std::function<void()>;

struct Channel {
  Callback read;
  Callback write;
};

class EventLoop {
 public:
  virtual ~EventLoop() = default;
  virtual void pushReadEvent(int fd) = 0;
  virtual void pushWriteEvent(int fd) = 0;
  virtual void registerFd(int fd, Callback read, Callback write) = 0;
  virtual void start() = 0;
};

#endif  // EVENT_LOOP_H
