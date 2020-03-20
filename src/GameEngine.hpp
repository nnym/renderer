#include <vulkan/vulkan.h>
#include <vector>
#include <GLFW/glfw3.h>

#include "QueueFamilyIndices.hpp"

#ifndef GAME_ENGINE_GAMEENGINE_HPP
#define GAME_ENGINE_GAMEENGINE_HPP

#endif //GAME_ENGINE_GAMEENGINE_HPP

class GameEngine {
public:
    GameEngine(int width, int height);

    ~GameEngine();

    void run();

private:
    void initWindow();

    void initVulkan();

    void mainLoop();

    void selectPhysicalDevice();

    static int rateDevice(VkPhysicalDevice &physicalDevice);

    static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void setupDebugMessenger();

    static void checkExtensions(const std::vector<const char *> &supportedExtensions,
                                const std::vector<VkExtensionProperties> &extensions);

    bool areValidationLayersSupported();

    std::vector<const char *> getRequiredExtensions();

    static uint32_t debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkFlags messageType,
                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

    const int WIDTH;
    const int HEIGHT;
    GLFWwindow *window{};
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    const std::vector<const char *> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
    };

    void createInstance();

    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice &physicalDevice);

    void cleanup();

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
};