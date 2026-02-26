//
// Created by Aurélien Brachet on 26/11/2025.
//

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>


class StringUtils {
public:
    static std::vector<std::string> split(const std::string &str, const std::string& delimiter);
};



#endif //STRING_UTILS_H
