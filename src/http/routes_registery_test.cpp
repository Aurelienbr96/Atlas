//
// Created by Aurélien Brachet on 16/03/2026.
//

#include "routes_registery.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "response.h"

TEST(RoutesRegistryTest, shouldRegisterGetRoute) {
  RouteRegistery routes;

  routes.get("/test", [](const Request& req, Response& res) {});

  auto handler = routes.getHandler("GET", "/test");
  EXPECT_NE(handler, nullptr);
}

TEST(RoutesRegistryTest, shouldRegisterPostRoute) {
  RouteRegistery routes;

  routes.post("/test", [](const Request& req, Response& res) {});

  auto handler = routes.getHandler("POST", "/test");
  EXPECT_NE(handler, nullptr);
}

TEST(RoutesRegistryTest, shouldReturnNullptrIfRouteNotFound) {
  RouteRegistery routes;

  routes.post("/test", [](const Request& req, Response& res) {});

  auto handler = routes.getHandler("GET", "/test");
  EXPECT_EQ(handler, nullptr);
}

TEST(RoutesRegistryTest, shouldReturnNullptrIfRouteNotFound2) {
  RouteRegistery routes;

  routes.post("/test2", [](const Request& req, Response& res) {});

  auto handler = routes.getHandler("GET", "/test2");
  EXPECT_EQ(handler, nullptr);
}
