#ifndef FILEWATCH
#define FILEWATCH

#include <functional>
#include <map>
#include <thread>
#include <atomic>

#include "files/File.hpp"

class FileWatch {
    using FunctionType = std::function<void(void)>;
    public:
        FileWatch() = default;
        FileWatch(FileWatch& other) = delete;
        FileWatch(FileWatch&& other) = delete;

        FileWatch& operator=(FileWatch& other) = delete;
        FileWatch& operator=(FileWatch&& other) = delete;

        void watchFile(std::string& filename, FunctionType f);
        void releaseFile(std::string& filename);

        void launch();
        void stop();

    private:
        std::map<std::string, FunctionType> mCallbacks;
        std::atomic_bool mStop{false};
        std::thread mThread;

        void run();
};

#endif