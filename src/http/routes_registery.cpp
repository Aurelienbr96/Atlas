//
// Created by Aurélien Brachet on 21/02/2026.
//

#include "routes_registery.h"

#include <utility>

void RouteRegistery::get(const string& path, Handler handler) {
  routes[path]["GET"] = std::move(handler);
}

void RouteRegistery::post(const string& path, Handler handler) {
  routes[path]["POST"] = std::move(handler);
}

const Handler* RouteRegistery::getHandler(const string& path, const string& method) {
  const auto path_it = this->routes.find(path);
  if (path_it == routes.end()) {
    return nullptr;
  }
  const auto handler_it = path_it->second.find(method);
  if (handler_it == path_it->second.end()) {
    return nullptr;
  }
  return &handler_it->second;
}
