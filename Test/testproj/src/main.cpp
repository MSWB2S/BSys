#include <iostream>
#include "util.h"
#include "testlib.h"

int main() {
    std::cout << "Hello from main.cpp!" << std::endl;
    std::cout << "Util says: " << utilFunction() << std::endl;
    std::cout << "TestLib says: " << testLibFunction() << std::endl;

#ifdef MAIN_FILE_DEFINE
    std::cout << "MAIN_FILE_DEFINE is active." << std::endl;
#endif
#ifdef TEST_DEFAULT_DEFINE
    std::cout << "TEST_DEFAULT_DEFINE is active." << std::endl;
#endif

    return 0;
}
