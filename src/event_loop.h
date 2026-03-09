//
// Created by Aurélien Brachet on 27/02/2026.
//

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include <cstdint>

/*
 *
    *ident → what object (file descriptor, process, signal, etc.)

    filter → what kind of event

    * EVFILT_READ = Triggered when data is available to read.
    *
    * sockets, pipes, files
    * EV_SET(&ev, socket_fd, EVFILT_READ, EV_ADD, 0, 0, client); = wake me up when the socket has data
    *
    * EVFILT_WRITE = triggered when you can write without bloquing
    * EV_SET(&ev, socket_fd, EVFILT_WRITE, EV_ADD, 0, 0, client);

    flags → how the event behaves

    fflags → extra details

 * flags = these control event registration behavior
 * EV_ADD = add an event to the kernel
 * EV_DELETE = remove an event from the kernel
 * EV_ENABLE = enable a previously disabled event
 * EV_DISABLE = Temporarily disables an event without deleting it.
 * EV_ONESHOT = event fired and automatically removed
 * EV_CLEAR = event is reset after being triggered
 * EV_ERROR = if fails
 * EV_EOF = socket/pipe/EOF
 *
 */

struct Event {
    uintptr_t ident;    // Unsigned integer large enough to store a pointer.
    // Same size as a pointer (4 bytes on 32-bit, 8 bytes on 64-bit).
    // Often used as an identifier (file descriptor, pointer, etc.).

    int16_t filter;     // Signed 16-bit integer (exactly 2 bytes).
    // Small numeric value (e.g., event type).
    // Range: -32768 to 32767.

    uint16_t flags;     // Unsigned 16-bit integer (exactly 2 bytes).
    // Often used as bit flags or options.
    // Range: 0 to 65535.

    uint32_t fflags;    // Unsigned 32-bit integer (exactly 4 bytes).
    // Additional flags or filter-specific options.
    // Range: 0 to 4,294,967,295.

    intptr_t data;      // Signed integer large enough to store a pointer.
    // Same size as a pointer.
    // Often used for numeric values returned by the OS
    // or pointer conversions. Can be negative.

    void* udata;        // Generic pointer (pointer to any type).
    // Used to attach user-defined data to the event.
    // Must be cast back to the correct type before use.
};

class EventLoop {
    int kq;
public:
    EventLoop();
    int getKqFb();
    void addEvent(const Event& event);
};

#endif //EVENT_LOOP_H
