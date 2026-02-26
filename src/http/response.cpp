//
// Created by Aurélien Brachet on 21/02/2026.
//

#include "response.h"
#include <sys/socket.h>

void Response::end(std::string &msg) {
    send(client_socket, msg.c_str(), msg.size(), 0);
}

