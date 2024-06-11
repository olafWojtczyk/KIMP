#pragma once

#include <vector>
#include <d3d11.h>
#include "imgui.h"
#include <string>

#include "../layer.h"
#include "../vec.h"

#include "../globals.h"
#include "selectionManager.h"

class Canvas {
public:
    explicit Canvas(vec::vec2 size);
    ~Canvas();

    void addLayer(Layer::Type type, __int64* groupPtr, vec::vec2 size, Layer::EffectType effectType = Layer::EffectType::NONE);
    void addImageLayer(const char* filename);
    void removeLayer(const char* name);

    Layer* getLayer(const char* name);
    std::vector<Layer*> getLayers() { return layers; }

    void addGroup();
    void removeGroup();

    vec::vec2 getSize() { return canvasSize; }
    void setSize(vec::vec2 size) {
        canvasSize = size;
        b.textureSize[0] = canvasSize.x;
        b.textureSize[1] = canvasSize.y;

        ctx->UpdateSubresource(cBuffer, 0, nullptr, &b, 0, 0);

        for (Layer* layer : layers) {
            layer->setSize(size);
        }
    }

    void setActiveLayer(Layer* layer) {
        activeLayer = layer;
    }

    void setActiveLayer(const char* name) {
        for (Layer* layer : layers) {
            if (strcmp(layer->getName(), name) == 0) {
                activeLayer = layer;
            }
        }
    }

    Layer* getActiveLayer() {
        return activeLayer;
    }

    void swapLayers(int i1, int i2) {
        std::swap(layers[i1], layers[i2]);
    }

    void render();
    vec::vec4 getColor(vec::vec2, Layer*);
    void setColor(vec::vec2, vec::vec4, Layer*);
    ID3D11Texture2D *renderRegion(ID3D11Texture2D *originalTexture, vec::vec2 start, vec::vec2 end);


    void serialize(std::ostream& os);
    void deserialize(std::istream& is);

    bool isNameUnique(const std::string& name);

    ID3D11Texture2D* viewport_canvas = nullptr;
    ID3D11ShaderResourceView* viewport_SRV = nullptr;

    ID3D11Texture2D* main_canvas = nullptr;
    ID3D11ShaderResourceView* main_SRV = nullptr;

    ImVec4 fgColor = ImVec4(1, 1, 1, 1);
    ImVec4 bgColor = ImVec4(0, 0, 0, 1);

    vec::vec2 offset = {0, 0};
    float scaleFactor = 1.0f;

    SelectionManager* selectionManager = nullptr;

    struct Effect {
        ID3D11PixelShader* shader;
        void* data;
        size_t dataSize;
    };

    std::vector<Effect> effectQueue;
private:
    int lc = 1;

    std::vector<Layer*> layers;

    Layer* activeLayer = nullptr;

    vec::vec2 canvasSize;

    ID3D11SamplerState* sampler = nullptr;
    ID3D11BlendState* pBlendState = nullptr;

    ID3D11RenderTargetView* viewport_RTV = nullptr;
    ID3D11RenderTargetView* main_RTV = nullptr;
    ID3D11RenderTargetView* thumbnail_RTV = nullptr;

    ID3D11DeviceContext* ctx = g::pd3dDeviceContext;
    ID3D11Device* dev = g::pd3dDevice;

    D3D11_VIEWPORT viewport;

    ID3D11Buffer *cBuffer = nullptr;

    struct Buffer {
        float textureSize[2];
        float padding[2];
    };

    Buffer b;

    std::vector<int> adjustmentIndices;
    std::vector<Layer*> adjustmentLayers;
};