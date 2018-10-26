#ifndef COLOR
#define COLOR

#include <cstdint>

template <typename T>
struct Color3 {
    T r;
    T g;
    T b;

    Color3(T red, T green, T blue) {
        r = red;
        g = green;
        b = blue;
    }
};

using Color3i = Color3<uint8_t>;
using Color3f = Color3<float>;

#endif