#include "src/server.h"
#include "iostream"
using namespace std;
int main() {
    RouteRegistery router;
    router.addHandler("/users", [](const Request& req, Response& res) {
        cout << "user route" << endl;
        std::string response = "HTTP/1.1 200 OK\r\n\r\nHello";
        res.end(response);
        res.end(response);
        res.end(response);
        res.end(response);
    });

    router.addHandler("/upload", [](const Request& req, Response& res) {
       cout << "upload route" << endl;
   });

    Server server(8080, router);
    server.run();
    cout << "listening on port " << server.getPort() << endl;
    return 0;
}