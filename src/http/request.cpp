//
// Created by Aurélien Brachet on 26/11/2025.
//

#include "request.h"
#include "map"
#include "../utils/string_utils.h"
#include "iostream"
using namespace std;

Request::Request(string body, Method method, string path): body(std::move(body)), method(method), path(std::move(path))  {}

void Request::addPath(string path) {
    this->path = path;
}

string &Request::getPath() {
    return this->path;
}


void Request::addHeader(string header,string content) {
    headers[header] = content;
};

optional<int> Request::getContentLength() {
    if (!headers.contains("Content-Length")) {
        return nullopt;
    }
    return optional{stoi(headers["Content-Length"])};
}

void Request::addBody(const string& body) {
    this->body = body;
}

