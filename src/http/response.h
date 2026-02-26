//
// Created by Aurélien Brachet on 21/02/2026.
//

#ifndef RESPONSE_H
#define RESPONSE_H

#include "string"

class Response {
    int client_socket;
public:
    Response(int client_socket): client_socket(client_socket) {};
    void end(std::string &msg);
};



#endif //RESPONSE_H
