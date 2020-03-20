#include <vector>
#include <optional>
#include <map>
#include <iostream>
#include <cstring>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "GameEngine.hpp"

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                      const VkAllocationCallbacks *pAllocator,
                                      VkDebugUtilsMessengerEXT *pMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                   const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance,
                                                                            "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) {
        func(instance, messenger, pAllocator);
    }
}

GameEngine::GameEngine(const int WIDTH, const int HEIGHT) : WIDTH(1440), HEIGHT(900) {};

void GameEngine::run() {
    this->initWindow();
    this->initVulkan();
    this->mainLoop();
    this->cleanup();
}

GameEngine::~GameEngine() = default;

void GameEngine::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    this->window = glfwCreateWindow(this->WIDTH, this->HEIGHT, "thing", nullptr, nullptr);
}

void GameEngine::mainLoop() {
    while (!glfwWindowShouldClose(this->window)) {
        glfwPollEvents();
    }
}

void GameEngine::initVulkan() {
    this->createInstance();
    this->setupDebugMessenger();
}

void GameEngine::selectPhysicalDevice() {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0) {
        throw std::runtime_error("Vulkan-supporting GPU not found");
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    vkEnumeratePhysicalDevices(this->instance, &physicalDeviceCount, physicalDevices.data());

    std::multimap<int, VkPhysicalDevice> scores;

    for (VkPhysicalDevice device : physicalDevices) {
        scores.insert(std::make_pair(rateDevice(device), device));
    }

    if (scores.rbegin()->first > 0) {
        physicalDevice = scores.rbegin()->second;
    } else {
        throw std::runtime_error("suitable GPU not found");
    }
}

int GameEngine::rateDevice(VkPhysicalDevice &physicalDevice) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    if (!deviceFeatures.geometryShader || !indices.isComplete()) {
        return 0;
    }

    int score = 0;

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D + deviceProperties.limits.maxImageDimension3D;

    return score;
}

void GameEngine::setupDebugMessenger() {
    if (!this->enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (createDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Debug messenger setup failed.");
    }
}

void GameEngine::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr;
    createInfo.flags = 0;
}

void GameEngine::checkExtensions(const std::vector<const char *> &supportedExtensions,
                            const std::vector<VkExtensionProperties> &extensions) {
    std::vector<std::string> unsupportedExtensions;

    if (!supportedExtensions.empty()) {
        std::cout << supportedExtensions.size() << " supported extensions:" << std::endl;
    }

    for (VkExtensionProperties extension : extensions) {
        bool supported = false;

        for (const char *supportedExtension : supportedExtensions) {
            if (!strcmp(supportedExtension, extension.extensionName)) {
                std::cout << "\t" << supportedExtension << std::endl;
                supported = true;
                break;
            }
        }

        if (!supported) {
            unsupportedExtensions.emplace_back(extension.extensionName);
        }
    }

    std::cout << std::endl;

    if (!unsupportedExtensions.empty()) {
        std::cout << unsupportedExtensions.size() << " unsupported extensions:" << std::endl;

        for (const std::string &extension : unsupportedExtensions) {
            std::cout << "\t" << extension << std::endl;
        }

        std::cout << std::endl;
    }
}

bool GameEngine::areValidationLayersSupported() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : this->validationLayers) {
        bool layerFound = false;

        for (const VkLayerProperties layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char *> GameEngine::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (this->enableValidationLayers) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VKAPI_ATTR uint32_t VKAPI_CALL GameEngine::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkFlags messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void GameEngine::createInstance() {
    if (this->enableValidationLayers && !areValidationLayersSupported()) {
        throw std::runtime_error("requested validation layers are unavailable");
    }

    VkApplicationInfo info;
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "thing";
    info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    info.pEngineName = "thing";
    info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &info;

    std::vector<const char *> glfwExtensions = getRequiredExtensions();

    createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();

    createInfo.enabledLayerCount = 0;

    if (this->enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    checkExtensions(glfwExtensions, extensions);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

    if (this->enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(this->validationLayers.size());
        createInfo.ppEnabledLayerNames = this->validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;

        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS) {
        throw std::runtime_error("Instantiation failed.");
    }
}

QueueFamilyIndices GameEngine::findQueueFamilies(VkPhysicalDevice &physicalDevice) {
    QueueFamilyIndices indices{};

    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

    int index = 0;

    for (const VkQueueFamilyProperties properties : queueFamilyProperties) {
        if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = index;
        }

        if (indices.isComplete()) {
            break;
        }

        index++;
    }

    return indices;
}

void GameEngine::cleanup() {
    if (this->enableValidationLayers) {
        destroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
    }

    vkDestroyInstance(this->instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
}

