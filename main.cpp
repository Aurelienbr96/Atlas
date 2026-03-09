#include "iostream"
#include "src/http/request.h"
#include "src/http/response.h"
#include "src/http/routes_registery.h"
#include "src/server.h"

using namespace std;
int main() {
  RouteRegistery router;
  router.addHandler("/users", [](const Request &req, Response &res) {
    cout << "user route" << endl;
    std::string response = "HTTP/1.1 200 OK\r\n\r\nHello";
    res.end(response);
  });

  router.addHandler("/upload", [](const Request &req, Response &res) {
    cout << "upload route" << endl;
  });

  Server server(8080, router);
  server.run();
  cout << "listening on port " << server.getPort() << endl;
  return 0;
}