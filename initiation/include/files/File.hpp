#ifndef FILEHPP
#define FILEHPP

#include <string>
#include <vector>
#include <fstream>

class File {
    public:
        enum OpenMode {
            Input = std::ios_base::in,
            Output = std::ios_base::out,
            Binary = std::ios_base::binary,
            Append = std::ios_base::app,
            Trucate = std::ios_base::trunc,
        };

        File(std::string filename, OpenMode openMode);
        File(File& other);
        File(File&& other);

        File& operator=(File& other);
        File& operator=(File&& other);

        template <typename T>
        void write(T& value);

        std::vector<char> read();

        static std::vector<char> readFile(std::string filename);

    private:
        std::string mFilename;
        std::fstream mFileHandler;
        OpenMode mOpenMode;
};

template <typename T>
void File::write(T& value) {
    if (!(mOpenMode & OpenMode::Output)) {
        throw std::runtime_error("Can't write if the \"OpenMode::Output\" flag is missing");
    }
    mFileHandler << value;
}

#endif