//
// Created by Aurélien Brachet on 21/02/2026.
//

#ifndef RESPONSE_H
#define RESPONSE_H

#include "conn.h"
#include "string"
#include "../event_loop.h"

class Response {
    int client_socket;
    EventLoop* eventLoop;
    Conn* conn;
public:
    explicit Response(int client_socket,  EventLoop* eventLoop,  Conn* conn);
    void end(std::string &msg);
};



#endif //RESPONSE_H
