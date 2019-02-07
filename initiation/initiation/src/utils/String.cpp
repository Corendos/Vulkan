#include "utils/String.hpp"

std::vector<std::string> String::split(const std::string& str, const char separator) {
    std::vector<std::string> strParts;
    std::string::const_iterator beg = str.begin();
    for (auto it = str.begin();it != str.end();++it) {
        if (*it == separator) {
            strParts.push_back(std::string(beg, it));
            beg = it;
        }
    }
    strParts.push_back(std::string(beg, str.end()));
    return strParts;
}