#ifndef STRING
#define STRING

#include <vector>
#include <string>

class String {
    public:
        static std::vector<std::string> split(const std::string& str, const char separator);
};

#endif