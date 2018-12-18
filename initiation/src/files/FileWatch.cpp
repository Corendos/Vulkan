#include <chrono>
#include <iostream>

#include "files/FileWatch.hpp"

void FileWatch::watchFile(std::string& filename, FunctionType f) {
    mCallbacks[filename] = f;
}

void FileWatch::releaseFile(std::string& filename) {
    mCallbacks.erase(filename);
}

void FileWatch::launch() {
    mThread = std::thread(&FileWatch::run, this);
}

void FileWatch::run() {
    while(!mStop) {
        for (auto& pair : mCallbacks) {
            
        }

        std::cout << "Checking files..." << std::endl;

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1000ms);
    }
}

void FileWatch::stop() {
    mStop = true;
    mThread.join();
}