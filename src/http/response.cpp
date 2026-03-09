//
// Created by Aurélien Brachet on 21/02/2026.
//

#include "response.h"

#include <iostream>
#include <unistd.h>
#include <sys/event.h>
#include <sys/socket.h>

Response::Response(int client_socket, EventLoop* eventLoop, Conn* conn):  client_socket(client_socket), eventLoop(eventLoop), conn(conn) {
}


void Response::end(std::string &msg) {
    std::cout << "endliiiing" << std::endl;
    std::cout << "sending message: " << msg << std::endl;
    this->conn->out.append(msg);

    eventLoop->addEvent({
                    static_cast<uintptr_t>(client_socket),
                    EVFILT_WRITE,
                    EV_ADD | EV_ENABLE | EV_CLEAR,
                    0,
                     0,
                    nullptr
        });

    //send(client_socket, msg.c_str(), msg.size(), 0);
}

