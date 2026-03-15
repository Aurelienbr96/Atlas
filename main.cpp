#include "iostream"
#include "src/http/request.h"
#include "src/http/response.h"
#include "src/http/routes_registery.h"
#include "src/server.h"

int main() {
  RouteRegistery router;
  EventLoop eventLoop;

  router.addHandler("/users", [](const Request &req, Response &res) {
    std::string body = "Hello";
    res.setHttpStatus(HttpStatus::OK);

    res.end(body);
  });

  router.addHandler("/upload",
                    [](const Request &req, Response &res) { cout << "upload route" << endl; });

  Server server(8080, router, eventLoop);

  server.run();
  eventLoop.start();
  cout << "listening on port " << server.getPort() << endl;
  return 0;
}
