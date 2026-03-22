#include "http_parser.h"

#include <gtest/gtest.h>

#include "request.h"

TEST(HttpParser, shouldParseHeader) {
  HttpParser httpParser;
  httpParser.feed("POST /users HTTP/1.1\r\nHost: example.com\r\nContent-L");
  EXPECT_EQ(httpParser.getRequest()->getMethod(), "POST");

  EXPECT_EQ(httpParser.getRequest()->getPath(), "/users");
}
