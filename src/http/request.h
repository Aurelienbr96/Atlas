//
// Created by Aurélien Brachet on 26/11/2025.
//

#ifndef REQUEST_H
#define REQUEST_H
#include <string>

#include "map"
using namespace std;

enum class Method { GET, POST, PUT };

class Request {
  map<string, string> headers;
  string body;
  Method method;
  string path;

 public:
  Request(string body, Method method, string path);
  Request() = default;
  void addHeader(string header, string content);
  void addPath(string path);
  void addBody(const string& body);

  string& getPath();
  optional<int> getContentLength();
};

#endif  // REQUEST_H
