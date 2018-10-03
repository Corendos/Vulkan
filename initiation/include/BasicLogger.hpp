#ifndef BASIC_LOADER
#define BASIC_LOADER

#include <fstream>
#include <functional>
#include <string>

class BasicLogger {
    public:
        BasicLogger(const std::string& logFilename);

        template <typename T>
        friend BasicLogger& operator<<(BasicLogger& logger, const T& t);
        friend BasicLogger& operator<<(BasicLogger& logger, std::ostream& (*manip)(std::ostream&));

    private:
        std::string mLogFilename;
        std::ofstream mOutputStream;
};

template <typename T>
BasicLogger& operator<<(BasicLogger& logger, const T& t) {
    logger.mOutputStream << t;
    return logger;
}

#endif