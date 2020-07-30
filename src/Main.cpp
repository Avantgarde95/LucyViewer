#include <iostream>
#include <stdexcept>

#include <xmmintrin.h>
#include <pmmintrin.h>

#include "App.hpp"

int main(int argc, char *argv[]) try {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " (Models' directory)\n";
        return 0;
        //argv[1] = "../../../object/StarLab";
    }

    // From Embree document Chapter 8.
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    App app(argc, argv);
    app.exec(); // NOLINT(readability-static-accessed-through-instance)

    return 0;
}
catch (const std::exception &error) {
    std::cout << "Error: " << error.what();
    std::cin.get();
}
