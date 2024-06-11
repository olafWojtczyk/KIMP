#pragma once

#include "projectManager.h"

#include <string>
#include <unordered_map>

extern ProjectManager* projectManager;

class EffectManager {
public:
    struct Data {
        void* data;
        size_t dataSize;
    };

    enum class FX {
        KERNEL,
        GAUSSIAN_BLUR,
        NOISE
    };

    void pushEffect(FX fx, Data data);
};
