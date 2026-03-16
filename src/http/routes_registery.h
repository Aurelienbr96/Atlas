//
// Created by Aurélien Brachet on 21/02/2026.
//

#ifndef ROUTES_REGISTERY_H
#define ROUTES_REGISTERY_H
#include <unordered_map>

#include "functional"
#include "request.h"
#include "response.h"

using Handler = std::function<void(const Request&, Response&)>;

using namespace std;

class RouteRegistery {
  std::unordered_map<string, std::unordered_map<string, Handler>> routes;

 public:
  void get(const string& path, Handler);
  void post(const string& path, Handler);

  const Handler* getHandler(const string& path, const string& method);
};

#endif  // ROUTES_REGISTERY_H
