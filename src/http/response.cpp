//
// Created by Aurélien Brachet on 21/02/2026.
//

#include "response.h"

#include <format>

#include "event_loop/event_loop.h"

Response::Response(int client_socket, EventLoop* eventLoop, Conn* conn)
    : client_socket(client_socket), eventLoop(eventLoop), conn(conn) {}

void Response::end() { end(""); }

void Response::end(const std::string& msg) {
  std::string response =
      std::format("HTTP/1.1 {} {}\r\nContent-Length:{}\r\nConnection: keep-alive\r\n\r\n{}",
                  http_response_status.code, http_response_status.phrase, msg.size(), msg);
  this->conn->out.append(response);

  eventLoop->pushWriteEvent(client_socket);
}

void Response::setHttpStatus(HttpStatusInfo code) { http_response_status = code; }
