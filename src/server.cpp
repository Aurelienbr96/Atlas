//
// Created by Aurélien Brachet on 23/11/2025.
//

#include "server.h"
#include "iostream"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ranges>
#include <sys/event.h>
#include <sys/fcntl.h>

#include "http/routes_registery.h"

#include "http/response.h"
#include "http/request.h"
#include "map"
#include "http/http_parser.h"

using namespace std;


static bool set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
}

static void add_event(int kq, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata) {
    struct kevent kev;
    EV_SET(&kev, ident, filter, flags, fflags, data, udata);
    if (kevent(kq, &kev, 1, nullptr, 0, nullptr) == -1) {
        std::cerr << "kevent(ADD/DEL) failed: " << std::strerror(errno) << "\n";
        std::exit(1);
    }
}

Server::Server(int port, RouteRegistery& routeRegistery) {
    this->port = port;
    this->routeRegistery = routeRegistery;

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket failed");
    }
    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);
    this->serverAddress = serverAddress;
    this->serverSocket = serverSocket;
    set_nonblocking(serverSocket);
    cout << "Server listening on port:" << port << endl;

}

Server::~Server() {
    close(this->serverSocket);
}


void Server::run() {
    int res = ::bind(this->serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),sizeof(serverAddress));
    int kq = kqueue();
    if (kq == -1) {
        std::cerr << "kqueue: " << std::strerror(errno) << "\n";
        return;
    }

    add_event(kq,
            static_cast<uintptr_t>(this->serverSocket),
            EVFILT_READ,
            EV_ADD | EV_ENABLE,
            0,
            0,
            nullptr);


    if (res < 0) {
        perror("bind failed");
    }
    listen(serverSocket, SOMAXCONN);

    while (true) { // accept loop
        // one parser per incoming request
        HttpParser parser;
        int clientSocket = ::accept(this->serverSocket, nullptr, nullptr);
        set_nonblocking(clientSocket);
        connClients.insert(clientSocket);

        while (true) { // read loop
            char buffer[4096] = {0}; // 4kb
            int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytes <= 0) {
                perror("recv failed");
            }
            parser.feed(string(buffer, bytes));
            if (auto rqt = parser.constructRequest()) {
                Request request = std::move(*rqt);
                cout << request.getContentLength().value() << endl;
                Response response(clientSocket);
                handleRequest(request, response);
                break;
            }
        }


        close(clientSocket);
        connClients.erase(clientSocket);
    }
}

void Server::handleRequest(Request& request, Response& res) {
    string& s = request.getPath();
    cout << s << endl;
    Handler* handler = this->routeRegistery.getHandler(s);
    if (handler == nullptr) {
        cout << "route not found" << endl;
        return;
    }
    (*handler)(request, res);
    cout << "route found" << endl;
}

int Server::getPort() const {
    return this->port;
}