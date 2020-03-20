#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include <vector>
#include "GameEngine.hpp"

int main() {
    GameEngine application(1440, 900);

    try {
        application.run();
    } catch (const std::exception &exception) {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
