#ifndef PROFILER
#define PROFILER

#include <chrono>
#include <array>
#include <string>
#include <sstream>

struct Timestamp {
    std::chrono::duration<int64_t, std::micro> duration;
    std::string name;
};

template <unsigned int pointCount>
class Profiler {
    public:
        void init();
        void addTimestamp(std::string name);
        std::string toString();

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> mInitTime;
        std::array<Timestamp, pointCount> mTimestamps;
        uint32_t mIndex{0};
};

template <unsigned int pointCount>
void Profiler<pointCount>::init() {
    mInitTime = std::chrono::high_resolution_clock::now();
}

template <unsigned int pointCount>
void Profiler<pointCount>::addTimestamp(std::string name) {
    std::chrono::duration<int64_t, std::micro> duration = std::chrono::duration_cast<std::chrono::duration<int64_t, std::micro>>(std::chrono::high_resolution_clock::now() - mInitTime);
    mTimestamps[mIndex++] = {
        duration,
        std::move(name)
    };
    mInitTime = std::chrono::high_resolution_clock::now();
}

template <unsigned int pointCount>
std::string Profiler<pointCount>::toString() {
    std::ostringstream os;
    for (size_t i{0};i < mIndex;++i) {
        os << mTimestamps[i].name << ": " << mTimestamps[i].duration.count() << "µs" << std::endl;
    }
    return os.str();
}

#endif