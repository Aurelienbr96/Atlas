#include "http_parser.h"

#include <iostream>
#include <optional>

#include "http/request.h"

int CRLF_SIZE = 2;
std::string CRLF = "\r\n";
std::string CRLF_2 = "\r\n\r\n";

optional<Request> HttpParser::constructRequest() {
  if (status != Status::BodyParsingDone) {
    return nullopt;
  }

  auto idxOpt = getCurrentRequestSize();
  if (!idxOpt) return nullopt;

  size_t idx = *idxOpt;

  auto result = std::optional<Request>{std::move(request)};
  resetState(idx);
  return result;
}

optional<size_t> HttpParser::getCurrentRequestSize() {
  if (const auto contentLength = this->request.getContentLength()) {
    return header_end_index.value() + 4 + contentLength.value();
  }
  return nullopt;
}

Request* HttpParser::getRequest() { return &this->request; }

size_t HttpParser::getStartBody() const { return header_end_index.value() + 4; }

Status HttpParser::getStatus() const { return this->status; }

void HttpParser::resetState(size_t idx) {
  status = Status::DidNotStart;
  request = Request{};
  content = content.erase(0, idx);
  path.clear();
  header_end_index.reset();
  last_header_search = 0;
}
void HttpParser::parserHeader(size_t* idx, size_t* bodyStartIdx) {
  size_t middleIdx = content.find(":", header_end_index.value());
  std::string headeKey =
      content.substr(header_end_index.value(), middleIdx - header_end_index.value());
  std::string headerValue = content.substr(middleIdx + 2, *idx - middleIdx - 2);
  request.addHeader(headeKey, headerValue);
  header_end_index = *idx + 2;
  *idx = content.find("\r\n", header_end_index.value());
  *bodyStartIdx = content.find("\r\n\r\n");
}
bool HttpParser::parseStartLine() {
  if (header_end_index.has_value() && status != Status::DidNotStart) {
    return true;
  }
  int idx = content.find("\r\n");

  if (idx == std::string::npos) return false;
  auto methodIdx = content.find(" ");
  auto protocolIdx = content.find(" ", methodIdx + 1);

  auto method = content.substr(0, methodIdx);
  auto path = content.substr(methodIdx + 1, protocolIdx - methodIdx - 1);
  std::cout << "we are here" << std::endl;
  status = Status::StartLineParsingDone;
  header_end_index = idx + CRLF_SIZE;
  request.setMethod(method);
  request.setPath(path);
  return true;
}
bool HttpParser::parseHeader() {
  if (status != Status::StartLineParsingDone) {
    return true;
  }
  size_t idx = content.find(CRLF, header_end_index.value());
  size_t bodyStartIdx = content.find(CRLF_2);

  while (idx != std::string::npos) {
    parserHeader(&idx, &bodyStartIdx);
    if (idx == bodyStartIdx) {
      parserHeader(&idx, &bodyStartIdx);
      status = Status::HeaderParsingDone;
      header_end_index = header_end_index.value() + CRLF_SIZE;
      return true;
    }
  }
  return false;
}
bool HttpParser::parseBody() {
  if (status != Status::HeaderParsingDone) {
    return false;
  }
  auto contentLength = request.getContentLength();
  status = Status::BodyParsingDone;
  if (!contentLength.has_value()) {
    return true;
  }

  request.setBody(content.substr(header_end_index.value(), contentLength.value()));
  return true;
}
void HttpParser::feed(const string& c) {
  content += c;

  if (!parseStartLine()) {
    return;
  }
  if (!parseHeader()) {
    return;
  }
  if (!parseBody()) {
    return;
  }
}
