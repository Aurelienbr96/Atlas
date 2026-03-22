

#include <nlohmann/json.hpp>

#include "event_loop/event_loop_factory.h"
#include "http/http_status.h"
#include "iostream"
#include "src/http/request.h"
#include "src/http/response.h"
#include "src/http/routes_registery.h"
#include "src/server.h"

int main() {
  RouteRegistery router;

  auto eventLoop = EventLoopFactory::create();

  router.post("/users", [](const Request& req, Response& res) {
    nlohmann::json body = {{"message", "Hello"}, {"status", "ok"}};

    res.setHttpStatus(HttpStatus::OK);

    res.end(body.dump());
  });

  router.post("/upload", [](const Request& req, Response& res) {
    res.setHttpStatus(HttpStatus::OK);
    cout << "upload route" << endl;
    res.end();
  });

  Server server(8080, router, eventLoop);

  server.run();
  eventLoop->start();
  cout << "listening on port " << server.getPort() << endl;
  return 0;
}
