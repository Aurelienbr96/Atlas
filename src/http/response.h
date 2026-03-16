//
// Created by Aurélien Brachet on 21/02/2026.
//

#ifndef RESPONSE_H
#define RESPONSE_H

#include "../event_loop.h"
#include "conn.h"
#include "http_status.h"
#include "string"

class Response {
  int client_socket;
  EventLoop* eventLoop;
  Conn* conn;
  HttpStatusInfo http_response_status = HttpStatus::OK;

 public:
  explicit Response(int client_socket, EventLoop* eventLoop, Conn* conn);
  Response() = default;
  void setHttpStatus(HttpStatusInfo code);
  void end();
  void end(const std::string& msg);
};

#endif  // RESPONSE_H
