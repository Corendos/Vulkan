#ifndef FILEWATCH
#define FILEWATCH

#include <functional>
#include <map>
#include <thread>
#include <atomic>
#include <sys/inotify.h>

#include "files/File.hpp"

class FileWatch {
    using FunctionType = std::function<void(void)>;

    struct DirectoryWatcher {
        int inotifyWatchHandler;
        std::map<std::string, FunctionType> callbacks;
    };

    public:
        FileWatch() = default;
        FileWatch(FileWatch& other) = delete;
        FileWatch(FileWatch&& other) = delete;

        FileWatch& operator=(FileWatch& other) = delete;
        FileWatch& operator=(FileWatch&& other) = delete;

        void watchFile(const std::string& directory, const std::string& filename, FunctionType f);
        void releaseFile(const std::string& directory, const std::string& filename);

        void launch();
        void stop();

    private:
        std::map<std::string, DirectoryWatcher> mDirectoriesWatcher;
        std::atomic_bool mStop{false};
        std::thread mThread;

        int mInotifyHandler;

        void run();

        constexpr static size_t BufferSize = 1024 * sizeof(inotify_event);
};

#endif