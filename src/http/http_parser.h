//
// Created by Aurélien Brachet on 15/02/2026.
//

#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "request.h"
#include "string"
using namespace std;

enum class Status { DidNotStart, StartLineParsingDone, HeaderParsingDone, BodyParsingDone };

class HttpParser {
  Status status = Status::DidNotStart;
  Request request;
  string content;
  string method;
  string path;
  optional<size_t> header_end_index;
  int last_header_search = 0;
  [[nodiscard]] size_t getStartBody() const;

 public:
  optional<Request> constructRequest();
  HttpParser() = default;
  [[nodiscard]] Status getStatus() const;
  void feed(const string& content);
};

#endif  // HTTP_PARSER_H
