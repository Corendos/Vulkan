#ifdef __linux__

#include <chrono>
#include <iostream>
#include <algorithm>
#include <errno.h>
#include <unistd.h>

#include "linux/FileWatchLinux.hpp"

void FileWatch::watchFile(const std::string& directory, const std::string& filename, FunctionType f) {
    if (mDirectoriesWatcher.find(directory) != mDirectoriesWatcher.end()) {
        mDirectoriesWatcher[directory].callbacks[filename] = f;
    } else {
        int wd = inotify_add_watch(mInotifyHandler, directory.c_str(), IN_MODIFY);
        DirectoryWatcher dw = {wd};
        dw.callbacks[filename] = f;
        mDirectoriesWatcher[directory] = std::move(dw);
    }
}

void FileWatch::releaseFile(const std::string& directory, const std::string& filename) {
    if (mDirectoriesWatcher.find(directory) != mDirectoriesWatcher.end()) {
        if (mDirectoriesWatcher[directory].callbacks.erase(filename)) {
            if (mDirectoriesWatcher[directory].callbacks.empty()) {
                inotify_rm_watch(mInotifyHandler, mDirectoriesWatcher[directory].inotifyWatchHandler);
                mDirectoriesWatcher.erase(directory);
            }
        }
    }
}

void FileWatch::launch() {
    mInotifyHandler = inotify_init();

    if (mInotifyHandler < 0) {
        perror("inotify_init");
    }

    mThread = std::thread(&FileWatch::run, this);
}

void FileWatch::run() {
    return; // TEMPORARY
    while(!mStop) {
        char* buffer[BufferSize];
        int length = read(mInotifyHandler, buffer, BufferSize);

        if (length < 0) {
            perror("read");
        }

        int i = 0;
        while (i < length) {
            struct inotify_event* eventPtr = (struct inotify_event*)&buffer[i];
            /* Find if the directory is monitored */
            auto it = std::find_if(mDirectoriesWatcher.begin(), mDirectoriesWatcher.end(),
                [&eventPtr](auto& pair) {
                return pair.second.inotifyWatchHandler == eventPtr->wd;
            });
            if (it != mDirectoriesWatcher.end() && eventPtr->len) {
                /* Find if the file is being monitored */
                auto it2 = std::find_if(it->second.callbacks.begin(), it->second.callbacks.end(),
                    [&eventPtr](auto& pair) {
                        std::string name(eventPtr->name);
                    return pair.first == name;
                });
                if (it2 != it->second.callbacks.end()) {
                    std::cout << it->first << eventPtr->name << " modified" << std::endl;
                }
            }
            i += sizeof(inotify_event) + eventPtr->len;
        }
    }
}

void FileWatch::stop() {
    for (auto& pair : mDirectoriesWatcher) {
        for (auto& pair2 : pair.second.callbacks) {
            releaseFile(pair.first, pair2.first);
        }
    }
    mStop = true;
    mThread.join();
}

#endif