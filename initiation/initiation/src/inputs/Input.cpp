#include <functional>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

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

Mouse Input::getMouse() {
    mMouseMutex.lock();
    Mouse returned = mMouse;
    mMouseMutex.unlock();
    return returned;
}

void Input::update() {
    mMouseMutex.lock();
    double x, y;
    glfwGetCursorPos(mWindow, &x, &y);
    mMouse.delta.x = x - mMouse.position.x;
    mMouse.delta.y = y - mMouse.position.y;
    mMouse.position = {x, y};
    updateMouseButton(MouseButton::Left, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_LEFT));
    updateMouseButton(MouseButton::Right, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_RIGHT));
    updateMouseButton(MouseButton::Middle, glfwGetMouseButton(mWindow, GLFW_MOUSE_BUTTON_MIDDLE));
    mMouseMutex.unlock();
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

void Input::start() {
    mThread = std::thread(&Input::run, this);
}

void Input::run() {
    while(!mShouldStop) {
        auto startTime = std::chrono::high_resolution_clock::now();
        update();
        auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration<double>(endTime - startTime);
        auto target = std::chrono::duration<double>(1.0 / UPDATE_FREQUENCY);
        if (elapsed < target) {
            std::this_thread::sleep_for(target - elapsed);
        }
    }
}

void Input::stop() {
    mShouldStop = true;
    mThread.join();
}