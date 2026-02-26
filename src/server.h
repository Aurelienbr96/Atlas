//
// Created by Aurélien Brachet on 23/11/2025.
//

#ifndef SERVER_H
#define SERVER_H
#include <functional>
#include <set>
#include <netinet/in.h>
#include "http/request.h"
#include "http/routes_registery.h"
#include "http/response.h"

class Server {
    int port;
    int serverSocket;
    sockaddr_in serverAddress;

    std::set<int> connClients;
    RouteRegistery routeRegistery;

    void handleRequest(Request& request, Response& response);
public:
    explicit Server(int port, RouteRegistery& routeRegistery);
    ~Server();
    void run();
    int getPort() const;
};



#endif //SERVER_H
