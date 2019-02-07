#include "BasicLogger.hpp"

BasicLogger::BasicLogger(const std::string& logFilename) : mLogFilename(logFilename) {
    mOutputStream.open(mLogFilename, std::ios::out | std::ios::trunc);

    if (!mOutputStream.is_open()) {
        throw std::runtime_error("Failed to open the file \'" + mLogFilename + "\'");
    }
}

BasicLogger& operator<<(BasicLogger& logger, std::ostream& (*manip)(std::ostream&)) {
    manip(logger.mOutputStream);
    return logger;
}