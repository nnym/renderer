#include <optional>

#ifndef GAME_ENGINE_QUEUEFAMILYINDICES_HPP
#define GAME_ENGINE_QUEUEFAMILYINDICES_HPP

#endif //GAME_ENGINE_QUEUEFAMILYINDICES_HPP

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete();
};
