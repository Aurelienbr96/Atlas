//
// Created by Aurélien Brachet on 17/03/2026.
//

#ifndef EVENT_LOOP_FACTORY_H
#define EVENT_LOOP_FACTORY_H

#include "event_loop.h"

class EventLoopFactory {
 public:
  static EventLoop* create();
};

#endif  // EVENT_LOOP_FACTORY_H
