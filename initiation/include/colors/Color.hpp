#ifndef COLOR
#define COLOR

#include <cstdint>
#include <string>

template <typename T>
struct Color3;

template<typename T>
Color3<T> colorFromHexa(std::string hexa);

template <typename T>
struct Color3 {
    T r;
    T g;
    T b;

    Color3() {}

    Color3(T red, T green, T blue) {
        r = red;
        g = green;
        b = blue;
    }

    Color3(std::string hexa) {
        *this = colorFromHexa<T>(hexa);
    }
};

template<>
inline Color3<float> colorFromHexa(std::string hexa) {
    Color3<float> c;
    unsigned long color = std::stoul(hexa, nullptr, 16);
    uint8_t red = (0xFF0000 & color) >> 16;
    uint8_t green = (0x00FF00 & color) >> 8;
    uint8_t blue = 0x0000FF & color;
    c.r = (float)red / 255.0f;
    c.g = (float)green / 255.0f;
    c.b = (float)blue / 255.0f;
    return c;
}

template<>
inline Color3<uint8_t> colorFromHexa(std::string hexa) {
    Color3<uint8_t> c;
    unsigned long color = std::stoul(hexa, nullptr, 16);
    c.r = (0xFF0000 & color) >> 16;
    c.g = (0x00FF00 & color) >> 8;
    c.b = 0x0000FF & color;
    return c;
}

using Color3i = Color3<uint8_t>;
using Color3f = Color3<float>;

#endif