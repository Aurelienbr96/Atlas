//
// Created by Aurélien Brachet on 21/02/2026.
//

#ifndef ROUTES_REGISTERY_H
#define ROUTES_REGISTERY_H
#include <unordered_map>
#include "functional"
#include "request.h"
#include "response.h"

using Handler = void(*)(const Request&, Response&);

using namespace std;
//using Handler = std::function<void(const Request&, Response&)>;

class RouteRegistery {
    std::unordered_map<string, Handler> routes;
public:
    void addHandler(const string& method, Handler);
    Handler* getHandler(const string& method);
};



#endif //ROUTES_REGISTERY_H
