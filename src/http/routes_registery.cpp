//
// Created by Aurélien Brachet on 21/02/2026.
//

#include "routes_registery.h"

void RouteRegistery::addHandler(const string& method, Handler handler) { routes[method] = handler; }

Handler* RouteRegistery::getHandler(const string& method) {
  const auto it = this->routes.find(method);
  if (it == routes.end()) {
    return nullptr;
  }
  return &it->second;
}
