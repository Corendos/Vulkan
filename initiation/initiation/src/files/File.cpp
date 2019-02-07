#include <cassert>
#include "files/File.hpp"

File::File(std::string filename, OpenMode openMode) :
    mFilename(filename), mOpenMode(openMode) {
    mFileHandler.open(filename, static_cast<std::ios_base::openmode>(openMode));
}

File::File(File& other) : mOpenMode(other.mOpenMode), mFilename(other.mFilename) {
    mFileHandler.open(mFilename, static_cast<std::ios_base::openmode>(mOpenMode));
}

File::File(File&& other) :
    mOpenMode(std::move(other.mOpenMode)),
    mFilename(std::move(other.mFilename)),
    mFileHandler(std::move(other.mFileHandler)) {}

File& File::operator=(File& other) {
    mOpenMode = other.mOpenMode;
    mFilename = other.mFilename;
    mFileHandler.open(mFilename, static_cast<std::ios_base::openmode>(mOpenMode));

    return *this;
}

File& File::operator=(File&& other) {
    mOpenMode = std::move(other.mOpenMode);
    mFileHandler = std::move(other.mFileHandler);
    mFilename = std::move(other.mFilename);

    return *this;
}

std::vector<char> File::read() {
    if (!(mOpenMode & OpenMode::Output)) {
        throw std::runtime_error("Can't write if the \"OpenMode::Input\" flag is missing");
    }
    mFileHandler.seekg(0, std::ios_base::end);
    size_t fileSize = static_cast<size_t>(mFileHandler.tellg());
    std::vector<char> buffer(fileSize);
    mFileHandler.seekg(0);
    mFileHandler.read(buffer.data(), fileSize);

    return buffer;
}

std::vector<char> File::readFile(std::string filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file : \"" + filename + "\"");
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}