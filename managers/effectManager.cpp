#include "effectManager.h"
#include "shaderManager.h"

extern ShaderManager* shaderManager;

void EffectManager::pushEffect(FX fx, Data data) {
    std::unordered_map<FX, Shader*> shaderMap = {
            {FX::KERNEL, shaderManager->getShader(450)},
            {FX::GAUSSIAN_BLUR, shaderManager->getShader(451)},
            {FX::NOISE, shaderManager->getShader(452)}
    };

    auto canvas = projectManager->getActiveProject().project->canvas;
    auto queue = &canvas->effectQueue;

    auto shader = shaderMap[fx];

    queue->push_back({ shader->pixelShader, data.data, data.dataSize });
}