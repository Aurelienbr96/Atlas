//
// Created by Aurélien Brachet on 15/02/2026.
//

#include "http_parser.h"

#include "iostream"
#include "ranges"

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

void HttpParser::feed(const string& newContent) {
  content.append(newContent);
  if (!header_end_index.has_value()) {
    const size_t header_end = content.find("\r\n\r\n", last_header_search);
    if (header_end != string::npos) {
      header_end_index = header_end;
    } else {
      last_header_search = static_cast<int>(content.size() - 1);
    }
  }

  if (header_end_index.has_value()) {  // parse headers
    string c = content.substr(0, header_end_index.value());

    for (const auto& part : std::views::split(c, std::string_view("\r\n"))) {
      if (status == Status::DidNotStart) {
        auto first_space = content.find(' ');
        auto second_space = content.find(' ', first_space + 1);
        string method = content.substr(0, first_space);
        string path = content.substr(first_space + 1, second_space - first_space - 1);

        request.setPath(path);
        request.setMethod(method);

        status = Status::StartLineParsingDone;
        continue;
      }

      string line(part.begin(), part.end());
      int it = 0;
      string headerKey;
      string headerValue;

      for (const auto& p : views::split(line, string_view(":"))) {
        if (it == 0) {
          headerKey = string(p.begin(), p.end());
          it++;
          continue;
        }
        headerValue = string(p.begin(), p.end());
      }
      request.addHeader(headerKey, headerValue);
      status = Status::HeaderParsingDone;
    }
  }

  if (header_end_index.has_value() && status == Status::HeaderParsingDone) {
    const size_t start_body = getStartBody();
    if (const auto contentLength = request.getContentLength()) {
      if (content.size() - start_body >= static_cast<size_t>(*contentLength)) {
        const string body = content.substr(start_body, *contentLength);

        request.setBody(body);
        status = Status::BodyParsingDone;
      }
    }
  }
}
