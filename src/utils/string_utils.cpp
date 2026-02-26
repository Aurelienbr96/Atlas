//
// Created by Aurélien Brachet on 26/11/2025.
//

#include "string_utils.h"


#include <vector>
#include <string>

std::vector<std::string> StringUtils::split(const std::string &str, const std::string& delimiter) {
    std::string copy = str;
    std::vector<std::string> result;

    auto found = copy.find(delimiter);

    while (found != std::string::npos) {
        result.push_back(copy.substr(0, found));
        copy = copy.substr(found + delimiter.length());
        found = copy.find(delimiter);
    }

    result.push_back(copy);

    return result;
}
