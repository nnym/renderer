#include "GameEngine.hpp"

bool QueueFamilyIndices::isComplete() {
    return graphicsFamily.has_value();
}
