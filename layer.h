#pragma once

#include <d3d11.h>
#include "vec.h"
#include "globals.h"
#include <iostream>

#include <thread>
#include <chrono>
#include <unordered_map>
#include <utility>
#include <functional>
#include <cmath>

template <typename TT> struct hash {
    size_t operator()(TT const& tt) const { return std::hash<TT>()(tt); }
};

template <class T> inline void hash_combine(size_t& seed, T const& v) {
    seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const std::pair<T1, T2>& p) const {
        size_t seed = 0;
        hash_combine(seed, p.first);
        hash_combine(seed, p.second);
        return seed;
    }
};

class Layer {
public:
    enum Type {
        Pixel,
        Shape,
        Text,
        Adjustment
    };
    enum BlendMode {
        Normal = 430,
        Multiply,
        Screen,
        Overlay,
        Darken,
        Lighten,
        ColorDodge,
        ColorBurn,
        HardLight,
        SoftLight,
        Difference,
        Exclusion,
        Hue,
        Saturation,
        Color,
        Luminosity
    };

    enum class EffectType {
        NONE,
        BRIGHTNESS_CONTRAST,
        COLOR_BALANCE,
        CURVES,
        EXPOSURE,
        HUE_SATURATION,
        INVERT,
        LEVELS,
        MONOCHROME,
        POSTERIZE,
        THRESHOLD,
        VIBRANCE
    };

    // active layer buffer
    unsigned char* pSysMem = nullptr;

    std::unordered_map<std::pair<int, int>, ID3D11Texture2D*, pair_hash> mask_cluster;

    Layer(const char* name, __int64* groupPtr, vec::vec2 size) {
        strcpy(this->name, name);
        this->groupPtr = groupPtr;
        this->size = size;

        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));


        desc.Width = 64;
        desc.Height = 64;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        this->desc = desc;

        for (int j = 0; j < ceil(size.y / 64); j++)
            for (int i = 0; i < ceil(size.x / 64); i++) {
                if (!pSysMem) pSysMem = new unsigned char[(int)64 * (int)64 * 4];
                for (int y = 0; y < (int)64; ++y) {
                    for (int x = 0; x < (int)64; ++x) {
                        pSysMem[y * (int)64 * 4 + x * 4] = 255;
                        pSysMem[y * (int)64 * 4 + x * 4 + 1] = 255;
                        pSysMem[y * (int)64 * 4 + x * 4 + 2] = 255;
                        pSysMem[y * (int)64 * 4 + x * 4 + 3] = 255;
                    }
                }

                createSector({ i,j }, this->mask_cluster);
            }
    }

    Layer(std::istream& is) {
        deserialize(is);
    }

    virtual ~Layer() {
        delete[] name;
        delete[] pSysMem;

        for (auto& t : this->mask_cluster) if (t.second) t.second->Release();
    }

    void createSector(std::pair<int,int> pos, std::unordered_map<std::pair<int, int>, ID3D11Texture2D*, pair_hash> &cluster, bool fill = false) {
        D3D11_SUBRESOURCE_DATA initData = { pSysMem, (unsigned int)64 * 4, 0 };
        g::pd3dDevice->CreateTexture2D(&this->desc, fill ? &initData : nullptr, &cluster[pos]);
    }

    [[nodiscard]] ID3D11Texture2D* getMaskAtCoord(vec::vec2 coords, bool g = true, bool r = false) {
        if (!g) coords = coords - this->position;
        int j = (int)floorf((coords.x) / 64.f), i = (int)floorf((coords.y) / 64.f);
        if (!this->mask_cluster[{j, i}] && r) createSector({ j, i }, this->mask_cluster, true);
        return this->mask_cluster[{j, i}];
    }

    [[nodiscard]] ID3D11Texture2D* getMask() const {
        //TODO: create a mask thumbnail
        return nullptr;
    }

    void setBlendMode(BlendMode blendMode) {
        this->blendMode = blendMode;
    }

    [[nodiscard]] BlendMode getBlendMode() const {
        return this->blendMode;
    }

    void setPosition(vec::vec2 position) {
        this->position = position;
    }

    [[nodiscard]] vec::vec2 getPosition() const {
        return this->position;
    }

    void setOpacity(float opacity) {
        this->opacity = opacity;
    }

    [[nodiscard]] float getOpacity() const {
        return this->opacity;
    }

    void setVisible(bool visible) {
        this->visible = visible;
    }

    [[nodiscard]] bool isVisible() const {
        return this->visible;
    }

    void setLocked(bool locked) {
        this->locked = locked;
    }

    [[nodiscard]] bool isLocked() const {
        return this->locked;
    }

    void setName(char* name) {
        strcpy(this->name, name);
    }

    [[nodiscard]] const char* getName() const {
        return this->name;
    }

    void setGroup(__int64* groupPtr) {
        this->groupPtr = groupPtr;
    }

    [[nodiscard]] __int64* getGroup() const {
        return this->groupPtr;
    }

    [[nodiscard]] Type getType() const {
        return this->type;
    }

    void setSize(vec::vec2 size) {
        this->size = size;

        //TODO: resize texture
    }

    [[nodiscard]] vec::vec2 getSize() const {
        return this->size;
    }

    virtual void serialize(std::ostream& os) = 0;

protected:
    Type type;
    char* name = new char[256];

    vec::vec2 position = {0, 0};
    vec::vec2 size{};

    D3D11_TEXTURE2D_DESC desc = {};

    bool visible = true;
    float opacity = 1.0f;

    BlendMode blendMode = BlendMode::Normal;

    bool locked = false;

    __int64* groupPtr = nullptr;

private:
    virtual void deserialize(std::istream& is) {
        std::streampos pos = is.tellg();


        is.read((char *)&type, sizeof(Type));
        is.read((char *)&size, sizeof(vec::vec2));
        is.read((char *)&blendMode, sizeof(BlendMode));

        is.read((char *) &groupPtr, sizeof(__int64));

        char name[256];

        is.read(reinterpret_cast<char*>(name), sizeof(name));
        strcpy(this->name, name);

        is.seekg(pos);
    };
};

class PixelLayer : public Layer {
public:
    struct Effect {
        EffectType type;
        int UID;
        void* data;
    };

    std::unordered_map<std::pair<int, int>, ID3D11Texture2D*, pair_hash> texture_cluster;
    std::unordered_map<std::pair<int, int>, ID3D11Texture2D*, pair_hash> FXTexture_cluster;

    PixelLayer(const char* name, __int64* groupPtr, vec::vec2 size, vec::vec2 canva_sz) : Layer(name, groupPtr, size) {
        this->type = Type::Pixel;

        memset(pSysMem, 0, 64 * 64 * 4);

        for (int j = 0; j < ceil(size.y / 64); j++)
            for (int i = 0; i < ceil(size.x / 64); i++) {
                createSector({ i,j }, this->texture_cluster);
                createSector({ i,j }, this->FXTexture_cluster);
            }

        D3D11_TEXTURE2D_DESC thumbnailDesc = desc;

        thumbnailDesc.Width = (int)(canva_sz.x / 4.0f);
        thumbnailDesc.Height = (int)(canva_sz.y / 4.0f);

        thumbnailDesc.Usage = D3D11_USAGE_DEFAULT;
        thumbnailDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        thumbnailDesc.CPUAccessFlags = 0;

        g::pd3dDevice->CreateTexture2D(&thumbnailDesc, nullptr, &this->thumbnail);
    }

    explicit PixelLayer(std::istream& is) : Layer(is) {

        deserialize(is);

        memset(pSysMem, 0, 64 * 64 * 4);

        for (int j = 0; j < ceil(size.y / 64); j++)
            for (int i = 0; i < ceil(size.x / 64); i++) {
                createSector({ i,j }, this->FXTexture_cluster);
            }

        D3D11_TEXTURE2D_DESC thumbnailDesc = this->desc;

        thumbnailDesc.Usage = D3D11_USAGE_DEFAULT;
        thumbnailDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        thumbnailDesc.CPUAccessFlags = 0;
        thumbnailDesc.Width = (int)(this->size.x  / 4.0f);
        thumbnailDesc.Height = (int)(this->size.y / 4.0f);

        g::pd3dDevice->CreateTexture2D(&thumbnailDesc, nullptr, &this->thumbnail);
    }

    ~PixelLayer() {
        for (auto& t : this->texture_cluster)
            t.second->Release();
        for (auto& t : FXTexture_cluster)
            t.second->Release();
        this->thumbnail->Release();
        
    }
    
    // coords : the GLOBAL coordinate of the screen
    [[nodiscard]] ID3D11Texture2D* getTextureAtCoord(vec::vec2 coords, bool g = true, bool r = false) {
        if (!g) coords = coords - this->position;
        int j = (int)floorf((coords.x) / 64.f), i = (int)floorf((coords.y) / 64.f);
        if (!this->texture_cluster[{j, i}] && r)
            createSector({ j,i }, this->texture_cluster);
        return this->texture_cluster[{j, i}];
    }

    [[nodiscard]] ID3D11Texture2D* getFXTextureAtCoord(vec::vec2 coords, bool g = true, bool r = false) {
        if (!g) coords = coords - this->position;
        int j = (int)floorf((coords.x) / 64.f), i = (int)floorf((coords.y) / 64.f);
        if (!this->FXTexture_cluster[{j, i}] && r) createSector({ j,i }, this->FXTexture_cluster);
        return this->FXTexture_cluster[{j, i}];
    }

    [[nodiscard]] ID3D11Texture2D* getThumbnail() const {
        return this->thumbnail;
    }

    void setThumbnail(ID3D11Texture2D* thumbnail) const {
        g::pd3dDeviceContext->CopyResource(this->thumbnail, thumbnail);
    }

    void addEffect(EffectType type, void* data) {
        Effect effect = { type, UID, data };
        effectStack.push_back(effect);

        UID++;
    }

    void removeEffect(int UID) {
        for (int i = 0; i < effectStack.size(); i++) {
            if (effectStack[i].UID == UID) {
                effectStack.erase(effectStack.begin() + i);
                break;
            }
        }
    }

    std::vector<Effect>* getEffectStack() {
        return &effectStack;
    }

    void setEffectStack(std::vector<Effect> effectStack) {
        effectStack = std::move(effectStack);
    }

    void createConstantBuffer(unsigned int size, ID3D11Buffer** constantBuffer, void* data) {
        if(size > 0) {
            D3D11_BUFFER_DESC bd = {};
            bd.Usage = D3D11_USAGE_DEFAULT;
            bd.ByteWidth = sizeof(float) * 4 * size;
            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bd.CPUAccessFlags = 0;

            g::pd3dDevice->CreateBuffer(&bd, nullptr, constantBuffer);
            g::pd3dDeviceContext->UpdateSubresource(*constantBuffer, 0, nullptr, data, 0, 0);
        }
    }


    void serialize(std::ostream& os) override {
        os.write((char *) &this->size, sizeof(vec::vec2));
        os.write((char *) &this->type, sizeof(Type));
        os.write((char *) &this->blendMode, sizeof(BlendMode));

        os.write((char *) &this->groupPtr, sizeof(__int64));

        char nameBuffer[256] = {0};
        strcpy(nameBuffer, this->name);

        os.write(nameBuffer, sizeof(nameBuffer));

        os.write((char *) &this->position, sizeof(vec::vec2));

        os.write((char *) &this->opacity, sizeof(float));

        os.write((char *) &this->visible, sizeof(bool));
        os.write((char *) &this->locked, sizeof(bool));

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        for (auto& i : this->texture_cluster) {
            auto& tex = i.second;
            D3D11_TEXTURE2D_DESC desc;
            tex->GetDesc(&desc);

            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage = D3D11_USAGE_STAGING;

            ID3D11Texture2D* pStagingTexture = nullptr;
            g::pd3dDevice->CreateTexture2D(&desc, nullptr, &pStagingTexture);

            g::pd3dDeviceContext->CopyResource(pStagingTexture, tex);

            g::pd3dDeviceContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

            os.write((char*)&i.first, sizeof(unsigned int) * 2);
            os.write((char*)&mappedResource.RowPitch, sizeof(unsigned int));
            os.write((char*)mappedResource.pData, mappedResource.RowPitch * desc.Height);

            g::pd3dDeviceContext->Unmap(pStagingTexture, 0);

            pStagingTexture->Release();
        }

        os.write("MASK", 4);

        for (auto& i : this->mask_cluster) {
            auto& tex = i.second;
            D3D11_TEXTURE2D_DESC desc;
            tex->GetDesc(&desc);

            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage = D3D11_USAGE_STAGING;
            ID3D11Texture2D* pStagingTexture = nullptr;

            g::pd3dDevice->CreateTexture2D(&desc, nullptr, &pStagingTexture);

            g::pd3dDeviceContext->CopyResource(pStagingTexture, tex);

            g::pd3dDeviceContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

            os.write((char*)&i.first, sizeof(unsigned int) * 2);
            os.write((char*)&mappedResource.RowPitch, sizeof(unsigned int));
            os.write((char*)mappedResource.pData, mappedResource.RowPitch * desc.Height);

            g::pd3dDeviceContext->Unmap(pStagingTexture, 0);

            pStagingTexture->Release();
        }
        os.write("END", 4);
    }
private:
    int UID = 0;

    std::vector<Effect> effectStack;

    ID3D11Texture2D* thumbnail;

    void deserialize(std::istream& is) override {
        is.read((char *) &this->size, sizeof(vec::vec2));
        is.read((char *) &this->type, sizeof(Type));
        is.read((char *) &this->blendMode, sizeof(BlendMode));

        is.read((char *) &this->groupPtr, sizeof(__int64));

        char name[256];

        is.read(reinterpret_cast<char*>(name), sizeof(name));

        strcpy(this->name, name);

        is.read((char *) &this->position, sizeof(vec::vec2));

        is.read((char *) &this->opacity, sizeof(float));

        is.read((char *) &this->visible, sizeof(bool));
        is.read((char *) &this->locked, sizeof(bool));

        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = 64;
        desc.Height = 64;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        this->desc = desc;

        D3D11_TEXTURE2D_DESC descS = {};
        ZeroMemory(&desc, sizeof(descS));

        descS = this->desc;

        descS.BindFlags = 0;
        descS.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descS.Usage = D3D11_USAGE_STAGING;

        static char f[4];
        unsigned int texRowPitch;
        unsigned char* texData = new unsigned char[64*64];
        std::pair<int, int> lpos;

        while (true) {  // while it doesnt hit the word "MASK"
            auto g = is.tellg();is.read(f, 4);is.seekg(g);
            if (memcmp(f, "MASK", 4)) break;
            is.read((char*)&lpos, sizeof(unsigned int) * 2);
            is.read((char*)&texRowPitch, sizeof(unsigned int));
            is.read((char*)texData, texRowPitch * 64);
            D3D11_SUBRESOURCE_DATA texInitData = { texData, texRowPitch, 0 };

            ID3D11Texture2D* pStagingTexture = nullptr;

            g::pd3dDevice->CreateTexture2D(&descS, &texInitData, &pStagingTexture);
            g::pd3dDevice->CreateTexture2D(&this->desc, nullptr, &this->texture_cluster[lpos]);

            g::pd3dDeviceContext->CopyResource(this->texture_cluster[lpos], pStagingTexture);

            pStagingTexture->Release();
        }

        unsigned int maskRowPitch;
        unsigned char* maskData = new unsigned char[64*64];


        while (true) {  // while it doesnt hit the word "MASK"
            auto g = is.tellg(); is.read(f, 3); is.seekg(g);
            if (memcmp(f, "END", 3)) break;
            is.read((char*)&lpos, sizeof(unsigned int) * 2);
            is.read((char*)&maskRowPitch, sizeof(unsigned int));
            is.read(reinterpret_cast<char*>(maskData), maskRowPitch * 64);
            D3D11_SUBRESOURCE_DATA maskInitData = { maskData, maskRowPitch, 0 };
            ID3D11Texture2D* pStagingTexture = nullptr;

            g::pd3dDevice->CreateTexture2D(&descS, &maskInitData, &pStagingTexture);
            g::pd3dDevice->CreateTexture2D(&this->desc, nullptr, &this->mask_cluster[lpos]);

            g::pd3dDeviceContext->CopyResource(this->mask_cluster[lpos], pStagingTexture);
            pStagingTexture->Release();
        }

        delete[] texData;
        delete[] maskData;
    }
};

class AdjustmentLayer : public Layer {
public:
    AdjustmentLayer(const char* name, __int64* groupPtr, vec::vec2 size, EffectType adjustmentType) : adjustmentType(adjustmentType), Layer(name, groupPtr, size) {
        this->type = Type::Adjustment;

        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = size.x;
        desc.Height = size.y;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        this->desc = desc;

        switch(adjustmentType) {
            case EffectType::BRIGHTNESS_CONTRAST:
                dataSize = sizeof(BrightnessContrastData);
                break;
            case EffectType::COLOR_BALANCE:
                dataSize = sizeof(ColorBalanceData);
                break;
            case EffectType::CURVES:
                dataSize = sizeof(CurvesData);
                break;
            case EffectType::EXPOSURE:
                dataSize = sizeof(ExposureData);
                break;
            case EffectType::HUE_SATURATION:
                dataSize = sizeof(HueSaturationData);
                break;
            case EffectType::INVERT:
                dataSize = 0;
                break;
            case EffectType::LEVELS:
                dataSize = sizeof(LevelsData);
                break;
            case EffectType::MONOCHROME:
                dataSize = 0;
                break;
            case EffectType::POSTERIZE:
                dataSize = sizeof(PosterizeData);
                break;
            case EffectType::THRESHOLD:
                dataSize = sizeof(ThresholdData);
                break;
            case EffectType::VIBRANCE:
                dataSize = sizeof(VibranceData);
                break;
        }
    }

    AdjustmentLayer(std::istream& is) : Layer(is) {
        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = size.x;
        desc.Height = size.y;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        this->desc = desc;

        deserialize(is);
    }

    ~AdjustmentLayer() override {
        if (adjustmentData != nullptr) {
            delete adjustmentDataBuffer;
        }
    }

    void renderProperties() {
        if(adjustmentType == EffectType::BRIGHTNESS_CONTRAST) {

            ImGui::DragFloat("##Contrast", &brightnessContrastData.contrast, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##Brightness", &brightnessContrastData.brightness, 0.01f, -1.0f, 1.0f);

            adjustmentData = &brightnessContrastData;
        } else if (adjustmentType == EffectType::COLOR_BALANCE) {
            static int tone = 1;

            float* cyanRedShift = tone == 0 ? colorBalanceData.shadows + 1 : tone == 1 ? colorBalanceData.midtones + 1 : colorBalanceData.highlights + 1;
            float* magentaGreenShift = tone == 0 ? colorBalanceData.shadows + 2 : tone == 1 ? colorBalanceData.midtones + 2 : colorBalanceData.highlights + 2;
            float* yellowBlueShift = tone == 0 ? colorBalanceData.shadows + 3 : tone == 1 ? colorBalanceData.midtones + 3 : colorBalanceData.highlights + 3;

            ImGui::DragFloat("##CBCyanRedShift", cyanRedShift, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##CBCrimsonGreenShift", magentaGreenShift, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##CBYellowBlueShift", yellowBlueShift, 0.01f, -1.0f, 1.0f);

            ImGui::RadioButton("Shadows##CB", &tone, 0); ImGui::SameLine();
            ImGui::RadioButton("Midtones##CB", &tone, 1); ImGui::SameLine();
            ImGui::RadioButton("Highlights##CB", &tone, 2);

            adjustmentData = &colorBalanceData;

        } else if (adjustmentType == EffectType::CURVES) {

        } else if (adjustmentType == EffectType::EXPOSURE) {
            ImGui::DragFloat("##exposureLight", &exposureData.light, 0.1f, -10.0f, 10.0f);
            ImGui::DragFloat("##exposureShift", &exposureData.shift, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##exposureGamma", &exposureData.gamma, 0.01f, 0.0f, 1.0f);

            adjustmentData = &exposureData;
        } else if (adjustmentType == EffectType::HUE_SATURATION) {
            static float hue = hueSaturationData.hue * 360.0f;

            ImGui::DragFloat("##HSHue", &hue, 1.0f, -180.0f, 180.0f);
            ImGui::DragFloat("##HSSat", &hueSaturationData.saturation, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##HSVal", &hueSaturationData.value, 0.01f, -1.0f, 1.0f);

            hueSaturationData.hue = hue / 360.0f;

            adjustmentData = &hueSaturationData;
        } else if (adjustmentType == EffectType::LEVELS) {
            static int channel = 0;

            float* blackPoint = channel == 0 ? &levelsData.RGB.blackPoint : channel == 1 ? &levelsData.R.blackPoint : channel == 2 ? &levelsData.G.blackPoint : &levelsData.B.blackPoint;
            float* midtone = channel == 0 ? &levelsData.RGB.midtone : channel == 1 ? &levelsData.R.midtone : channel == 2 ? &levelsData.G.midtone : &levelsData.B.midtone;
            float* whitePoint = channel == 0 ? &levelsData.RGB.whitePoint : channel == 1 ? &levelsData.R.whitePoint : channel == 2 ? &levelsData.G.whitePoint : &levelsData.B.whitePoint;

            float* bThreshold = channel == 0 ? &levelsData.RGB.bThreshold : channel == 1 ? &levelsData.R.bThreshold : channel == 2 ? &levelsData.G.bThreshold : &levelsData.B.bThreshold;
            float* tThreshold = channel == 0 ? &levelsData.RGB.tThreshold : channel == 1 ? &levelsData.R.tThreshold : channel == 2 ? &levelsData.G.tThreshold : &levelsData.B.tThreshold;

            ImGui::SliderInt("##LevelsChannel", &channel, 0, 3, "RGB\0Red\0Green\0Blue\0");

            ImGui::DragFloat("##LevelsBlackPoint", blackPoint, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("##LevelsMidtone", midtone, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("##LevelsWhitePoint", whitePoint, 0.01f, 0.0f, 1.0f);

            ImGui::DragFloat("##LevelsBlackThreshold", bThreshold, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("##LevelsWhiteThreshold", tThreshold, 0.01f, 0.0f, 1.0f);

        } else if (adjustmentType == EffectType::POSTERIZE) {
            ImGui::DragFloat("##PosterizeLevels", &posterizeData.levels, 0.1f, 1.0f, 100.0f);

            adjustmentData = &posterizeData;
        } else if (adjustmentType == EffectType::THRESHOLD) {
            ImGui::DragFloat("##Threshold", &thresholdData.threshold, 0.01f, 0.0f, 1.0f);

            adjustmentData = &thresholdData;

        } else if (adjustmentType == EffectType::VIBRANCE) {
            ImGui::DragFloat("##Vibrance", &vibranceData.vibrance, 0.01f, -1.0f, 1.0f);
            ImGui::DragFloat("##VibranceSaturation", &vibranceData.saturation, 0.01f, -1.0f, 1.0f);

            adjustmentData = &vibranceData;
        }
    }

    void* getAdjustmentData() {
        return adjustmentData;
    }

    EffectType getAdjustmentType() {
        return adjustmentType;
    }

    void serialize(std::ostream& os) override {
        os.write((char *) &type, sizeof(Type));
        os.write((char *) &size, sizeof(vec::vec2));
        os.write((char *) &blendMode, sizeof(BlendMode));

        os.write((char *) &groupPtr, sizeof(__int64));

        char nameBuffer[256] = {0};
        strcpy(nameBuffer, name);

        os.write(nameBuffer, sizeof(nameBuffer));

        os.write((char *) &position, sizeof(vec::vec2));

        os.write((char *) &opacity, sizeof(float));

        os.write((char *) &visible, sizeof(bool));
        os.write((char *) &locked, sizeof(bool));

        os.write((char *) &adjustmentType, sizeof(EffectType));
        os.write((char *) adjustmentData, dataSize);

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        os.write("MASK", 4);

        for (auto& i : this->mask_cluster) {
            auto& tex = i.second;
            D3D11_TEXTURE2D_DESC desc;
            tex->GetDesc(&desc);

            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
            desc.Usage = D3D11_USAGE_STAGING;
            ID3D11Texture2D* pStagingTexture = nullptr;

            g::pd3dDevice->CreateTexture2D(&desc, nullptr, &pStagingTexture);

            g::pd3dDeviceContext->CopyResource(pStagingTexture, tex);

            g::pd3dDeviceContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

            os.write((char*)&i.first, sizeof(unsigned int) * 2);
            os.write((char*)&mappedResource.RowPitch, sizeof(unsigned int));
            os.write((char*)mappedResource.pData, mappedResource.RowPitch * desc.Height);

            g::pd3dDeviceContext->Unmap(pStagingTexture, 0);

            pStagingTexture->Release();
        }
        os.write("END", 4);
    }

private:
    char* adjustmentDataBuffer = nullptr;
    void* adjustmentData = nullptr;
    EffectType adjustmentType;

    struct channel {
        float blackPoint;
        float midtone;
        float whitePoint;
        float bThreshold;
        float tThreshold;
    };

    struct BrightnessContrastData {
        float contrast;
        float brightness;
        float padding[2];
    };
    struct ColorBalanceData {
        float shadows[3];
        float midtones[3];
        float highlights[3];
        float padding[3];
    };
    struct CurvesData {
        int n;
        int channels;
        float padding[2];
    };
    struct ExposureData {
        float light;
        float shift;
        float gamma;
        float padding;
    };
    struct HueSaturationData {
        float hue;
        float saturation;
        float value;
        float padding;
    };
    struct LevelsData {
        channel RGB;
        channel R;
        channel G;
        channel B;
    };
    struct PosterizeData {
        float levels;
        float padding[3];
    };
    struct ThresholdData {
        float threshold;
        float padding[3];
    };
    struct VibranceData {
        float saturation;
        float vibrance;
        float padding[2];
    };

    BrightnessContrastData brightnessContrastData = {0.0f, 0.0f};
    ColorBalanceData colorBalanceData = {0.0f, 0.0f, 0.0f};

    ExposureData exposureData = {0.0f, 0.0f, 1.0f};
    HueSaturationData hueSaturationData = {0.0f, 0.0f, 0.0f};
    LevelsData levelsData = {
            {0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 0.0f, 1.0f}
    };
    PosterizeData posterizeData = {1.0f};
    ThresholdData thresholdData = {0.5f};
    VibranceData vibranceData = {0.0f, 0.0f};

    size_t dataSize = 0;

    void deserialize(std::istream& is) override {
        is.read((char *) &this->size, sizeof(vec::vec2));
        is.read((char *) &this->type, sizeof(Type));
        is.read((char *) &this->blendMode, sizeof(BlendMode));

        is.read((char *) &this->groupPtr, sizeof(__int64));

        char name[256];

        is.read(reinterpret_cast<char*>(name), sizeof(name));

        strcpy(this->name, name);

        is.read((char *) &this->position, sizeof(vec::vec2));

        is.read((char *) &this->opacity, sizeof(float));

        is.read((char *) &this->visible, sizeof(bool));
        is.read((char *) &this->locked, sizeof(bool));

        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = 64;
        desc.Height = 64;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;

        this->desc = desc;

        D3D11_TEXTURE2D_DESC descS = {};
        ZeroMemory(&desc, sizeof(descS));

        descS = this->desc;

        descS.BindFlags = 0;
        descS.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        descS.Usage = D3D11_USAGE_STAGING;

        static char f[4];
        std::pair<int, int> lpos;

        unsigned int maskRowPitch;
        unsigned char* maskData = new unsigned char[64*64];

        while (true) {  // while it doesnt hit the word "MASK"
            auto g = is.tellg(); is.read(f, 3); is.seekg(g);
            if (memcmp(f, "END", 3)) break;
            is.read((char*)&lpos, sizeof(unsigned int) * 2);
            is.read((char*)&maskRowPitch, sizeof(unsigned int));
            is.read(reinterpret_cast<char*>(maskData), maskRowPitch * 64);
            D3D11_SUBRESOURCE_DATA maskInitData = { maskData, maskRowPitch, 0 };
            ID3D11Texture2D* pStagingTexture = nullptr;

            g::pd3dDevice->CreateTexture2D(&descS, &maskInitData, &pStagingTexture);
            g::pd3dDevice->CreateTexture2D(&this->desc, nullptr, &this->mask_cluster[lpos]);

            g::pd3dDeviceContext->CopyResource(this->mask_cluster[lpos], pStagingTexture);
            pStagingTexture->Release();
        }

        delete[] maskData;
    }
};