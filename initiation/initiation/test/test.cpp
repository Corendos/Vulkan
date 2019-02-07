#include <iostream>
#include <string>
#include "utils/String.hpp"

class A {
    public:
        A() : privateVar(42), publicVar(43) {
        }
        int publicVar;
    private:
        int privateVar;
};

class AAccessor {
    public:
        int getPrivate() { return privateVar; };
        int publicVar;
    private:
        int privateVar;

};

int main() {
    std::wcout << L"testéàç" << std::endl;   
}