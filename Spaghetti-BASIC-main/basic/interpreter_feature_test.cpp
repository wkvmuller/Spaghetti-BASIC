// Transpiled from interpreter_feature_test.bas
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>

int main() {
    double A[4] = {1, 2, 3, 4};
    for (int i = 0; i < 4; ++i) {
        std::cout << "A[" << i << "] = " << A[i] << std::endl;
    }

    double B[4];
    for (int i = 0; i < 4; ++i) {
        B[i] = A[i] * 2;
    }

    for (int i = 0; i < 4; ++i) {
        std::cout << "B[" << i << "] = " << B[i] << std::endl;
    }

    for (int i = 0; i <= 2; ++i) {
        std::cout << "FOR LOOP I = " << i << std::endl;
    }

    double rnd = std::rand() / (double)RAND_MAX;
    std::srand(1234);
    std::cout << "RND() = " << rnd << std::endl;
    std::cout << "TIME() = " << static_cast<int32_t>(std::time(0)) << std::endl;
    std::cout << "SIN(45) = " << std::sin(45) << std::endl;
    std::string Y = std::string("HELLO").substr(1,3);
    std::cout << "MID$ = " << Y << std::endl;
    return 0;
}
