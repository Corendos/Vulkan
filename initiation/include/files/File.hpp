#ifndef FILEHPP
#define FILEHPP

#include <string>
#include <vector>

class File {
    public:
        static std::vector<char> readFile(std::string filename);
};

#endif