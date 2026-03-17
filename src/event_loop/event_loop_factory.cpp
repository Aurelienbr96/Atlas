#include "event_loop_factory.h"

#ifdef __APPLE__
#include "event_loop/macos_event_loop.h"
#elif __linux__
#include "event_loop/linux_event_loop.h"
#endif

EventLoop* EventLoopFactory::create() {
#ifdef __APPLE__
  return new MacOsEventLoop();
#elif __linux__
  return new LinuxEventLoop();
#else
#error "Unsupported platform"
#endif
}