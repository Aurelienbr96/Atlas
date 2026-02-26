//
// Created by Aurélien Brachet on 15/02/2026.
//

//
// Created by Aurélien Brachet on 16/11/2025.
//
#include <gtest/gtest.h>
#include "http_parser.h"
#include "request.h"


TEST(HttpParser, shouldParseHeader) {
    HttpParser httpParser;
    httpParser.feed("POST /users HTTP/1.1\r\nHost: example.com\r\nContent-L");
    EXPECT_EQ(httpParser.getStatus(), Status::DidNotStart);
    httpParser.feed("ength: 11\r\n\r\nHello ");
    EXPECT_EQ(httpParser.getStatus(), Status::HeaderParsingDone);
    httpParser.feed("World");
    EXPECT_EQ(httpParser.getStatus(), Status::BodyParsingDone);
    Request req;
   if (auto rtr = httpParser.constructRequest()) {
       req = std::move(*rtr);
   }
    EXPECT_EQ(req.getPath(), "/users");
}
