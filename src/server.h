//
// Created by Aurélien Brachet on 23/11/2025.
//

#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h>

#include <set>

#include "event_loop/event_loop.h"
#include "http/request.h"
#include "http/response.h"
#include "http/routes_registery.h"

class Server {
  int port;
  int serverSocket;
  sockaddr_in serverAddress;
  EventLoop& eventLoop;

  RouteRegistery& routeRegistery;
  std::unordered_map<int, Conn> conns;

  void handleRequest(Request& request, Response& response);

 public:
  explicit Server(int port, RouteRegistery& routeRegistery, EventLoop& eventLoop);
  ~Server();
  void run();
  int getPort() const;
};

#endif  // SERVER_H
