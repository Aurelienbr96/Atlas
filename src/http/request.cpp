//
// Created by Aurélien Brachet on 26/11/2025.
//

#include "request.h"

#include <optional>

#include "../utils/string_utils.h"
#include "map"
using namespace std;

Request::Request(string body, std::string method, string path)
    : body(std::move(body)), method(method), path(std::move(path)) {}

void Request::setPath(string path) { this->path = path; }
void Request::setMethod(string method) { this->method = method; }

string &Request::getPath() { return this->path; }
string &Request::getMethod() { return this->method; }

void Request::addHeader(string header, string content) { headers[header] = content; };

optional<int> Request::getContentLength() {
  if (!headers.contains("Content-Length")) {
    return nullopt;
  }
  return optional{stoi(headers["Content-Length"])};
}

void Request::setBody(const string &body) { this->body = body; }
