#ifndef PATH
#define PATH

#include<string>

class Path {
    public:
        Path(const std::string& strPath);
        Path(Path& other);
        Path(Path&& other);
        Path& operator=(Path& other);
        Path& operator=(Path&& other);

        Path parent();

        static Path join(const Path& path1, const Path& path2);

    private:
        std::string mStrPath;
};

#endif