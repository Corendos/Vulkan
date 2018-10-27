#ifndef MOUSE
#define MOUSE

enum MouseButton {
    Right,
    Left,
    Middle,
    MAX_ENUM
};

struct Position {
    double x{0.0};
    double y{0.0};
};

struct ButtonState {
    bool pressed{false};
    bool up{false};
    bool down{false};
};

struct Mouse {
    Position position{};
    Position delta{};

    ButtonState button[MouseButton::MAX_ENUM]{};
};

#endif