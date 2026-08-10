#include "mathlib/mathlib.h"

#include <iostream>

int main()
{
    std::cout
        << "3 + 4 = "
        << MathAdd(3, 4)
        << '\n';

    std::cout
        << "3 * 4 = "
        << MathMultiply(3, 4)
        << '\n';

    return 0;
}
