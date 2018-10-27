#include <functional>
#include <iostream>
#include <iomanip>

#include "inputs/Input.hpp"

void Input::attachWindow(GLFWwindow* window) {
    mWindow = window;
}


void Input::updateMousePosition(double xPos, double yPos) {
    mMouse.delta.x = xPos - mMouse.position.x;
    mMouse.delta.y = yPos - mMouse.position.y;
    mMouse.position = {xPos, yPos};
}

void Input::updateMouseState(int button, int action, int mods) {
    int index = -1;
    switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            index = MouseButton::Left;
        break;

        case GLFW_MOUSE_BUTTON_RIGHT:
            index = MouseButton::Right;
        break;

        case GLFW_MOUSE_BUTTON_MIDDLE:
            index = MouseButton::Middle;
        break;

        default:
        return;
    }

    if (action == GLFW_PRESS) {
        mMouse.button[MouseButton::Left].pressed = true;
        mMouse.button[MouseButton::Left].up = false;
        if (!mMouse.button[MouseButton::Left].down) {
            mMouse.button[MouseButton::Left].down = true;
        } else {
            mMouse.button[MouseButton::Left].down = false;
        }
    } else if (action == GLFW_RELEASE) {
        mMouse.button[MouseButton::Left].pressed = false;
        mMouse.button[MouseButton::Left].down = false;
        if (!mMouse.button[MouseButton::Left].up) {
            mMouse.button[MouseButton::Left].up = true;
        } else {
            mMouse.button[MouseButton::Left].up = false;
        }
    }
}

Mouse& Input::getMouse() {
    return mMouse;
}

void Input::update() {
    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);
    mMouse.delta.x = x - mMouse.position.x;
    mMouse.delta.y = y - mMouse.position.y;
    mMouse.position = {x, y};
    updateMouseButton(MouseButton::Left, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT));
    updateMouseButton(MouseButton::Right, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT));
    updateMouseButton(MouseButton::Middle, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_MIDDLE));
}

void Input::updateMouseButton(int button, int state) {
    if (state == GLFW_PRESS) {
        mMouse.button[button].pressed = true;
        mMouse.button[button].up = false;
        if (!mMouse.button[button].down) {
            mMouse.button[button].down = true;
        } else {
            mMouse.button[button].down = false;
        }
    } else if (state == GLFW_RELEASE) {
        mMouse.button[button].pressed = false;
        mMouse.button[button].down = false;
        if (!mMouse.button[button].up) {
            mMouse.button[button].up = true;
        } else {
            mMouse.button[button].up = false;
        }
    }
}