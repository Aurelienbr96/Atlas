//
// Created by Aurélien Brachet on 26/11/2025.
//

#ifndef REQUEST_H
#define REQUEST_H
#include <optional>
#include <string>

#include "map"

class Request {
  std::map<std::string, std::string> headers;
  std::string body;
  std::string method;
  std::string path;

 public:
  Request(std::string body, std::string method, std::string path);
  Request() = default;
  void addHeader(std::string header, std::string content);

  void setBody(const std::string& body);
  void setPath(std::string path);
  void setMethod(std::string body);

  std::string& getPath();
  std::string& getMethod();
  std::optional<int> getContentLength();
};

#endif  // REQUEST_H
