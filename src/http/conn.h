//
// Created by Aurélien Brachet on 01/03/2026.
//

#ifndef CONN_H
#define CONN_H
#include "http_parser.h"

struct Conn {
  int fd;
  HttpParser parser;
  std::string out;  // pending bytes to write
};

#endif  // CONN_H
