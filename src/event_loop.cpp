//
// Created by Aurélien Brachet on 27/02/2026.
//

#include "event_loop.h"
#include <sys/event.h>
#include <iostream>

EventLoop::EventLoop() {
    const int kq = kqueue();
    this->kq = kq;
}

void EventLoop::addEvent(const Event& event) {
        struct kevent kev;
        std::cout << "push event" << event.ident << std::endl;
        EV_SET(&kev, event.ident, event.filter, event.flags, event.fflags, event.data, event.udata);
        if (kevent(this->kq, &kev, 1, nullptr, 0, nullptr) == -1) {
            std::cerr << "kevent(ADD/DEL) failed: " << std::strerror(errno) << "\n";
            std::exit(1);
        }
};

int EventLoop::getKqFb() {
    return this->kq;
}



