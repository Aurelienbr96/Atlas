#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include <optional>

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

  void parserHeader(size_t* idx, size_t* bodyStartIdx);
  void resetState(size_t idx);
  optional<size_t> getCurrentRequestSize();

 public:
  optional<Request> constructRequest();
  Request* getRequest();
  HttpParser() = default;
  [[nodiscard]] Status getStatus() const;
  void feed(const string& content);
};

#endif  // HTTP_PARSER_H
