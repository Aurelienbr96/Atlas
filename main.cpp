

#include <nlohmann/json.hpp>

#include "iostream"
#include "src/http/request.h"
#include "src/http/response.h"
#include "src/http/routes_registery.h"
#include "src/server.h"

int main() {
  RouteRegistery router;
  EventLoop eventLoop;

  router.post("/users", [](const Request &req, Response &res) {
    nlohmann::json body = {{"message", "Hello"}, {"status", "ok"}};

    res.setHttpStatus(HttpStatus::OK);

    res.end(body.dump());
  });

  router.post("/upload", [](const Request &req, Response &res) { cout << "upload route" << endl; });

  Server server(8080, router, eventLoop);

  server.run();
  eventLoop.start();
  cout << "listening on port " << server.getPort() << endl;
  return 0;
}
