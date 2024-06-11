#include <d3dcompiler.h>
#include <FreeImage.h>
#include <cmath>
#include "canvasManager.h"
#include "shaderManager.h"

extern ShaderManager* shaderManager;

Canvas::Canvas(vec::vec2 size) {
    canvasSize = size;

    D3D11_SAMPLER_DESC s_desc;

    ZeroMemory(&s_desc, sizeof(s_desc));
    s_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    s_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    s_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    s_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    s_desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    s_desc.MinLOD = 0;
    s_desc.MaxLOD = D3D11_FLOAT32_MAX;

    dev->CreateSamplerState(&s_desc, &sampler);

    D3D11_TEXTURE2D_DESC desc = {};
    ZeroMemory(&desc, sizeof(desc));

    desc.Width = canvasSize.x;
    desc.Height = canvasSize.y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    dev->CreateTexture2D( &desc, nullptr, &viewport_canvas );
    dev->CreateRenderTargetView( viewport_canvas, nullptr, &viewport_RTV );
    dev->CreateShaderResourceView( viewport_canvas, nullptr, &viewport_SRV );

    dev->CreateTexture2D( &desc, nullptr, &main_canvas );
    dev->CreateRenderTargetView( main_canvas, nullptr, &main_RTV );
    dev->CreateShaderResourceView( main_canvas, nullptr, &main_SRV );

    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(canvasSize.x);
    viewport.Height = static_cast<float>(canvasSize.y);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D11_BUFFER_DESC cBufferDesc;

    ZeroMemory(&cBufferDesc, sizeof(cBufferDesc));
    cBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    cBufferDesc.ByteWidth = sizeof(Buffer);
    cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cBufferDesc.CPUAccessFlags = 0;
    cBufferDesc.MiscFlags = 0;

    if(FAILED(dev->CreateBuffer(&cBufferDesc, nullptr, &cBuffer))) {
        throw std::runtime_error("Failed to create constant buffer");
    }

    b.textureSize[0] = canvasSize.x;
    b.textureSize[1] = canvasSize.y;

    ctx->UpdateSubresource(cBuffer, 0, nullptr, &b, 0, 0);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));

    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    dev->CreateBlendState(&blendDesc, &pBlendState);

    selectionManager = new SelectionManager();
}

Canvas::~Canvas() {
    for (Layer* layer : layers) {
        delete layer;
    }

    sampler->Release();

    main_RTV->Release();
    main_SRV->Release();
    main_canvas->Release();

    viewport_RTV->Release();
    viewport_SRV->Release();
    viewport_canvas->Release();

    cBuffer->Release();

    lc = 1;

    delete selectionManager;
}

void Canvas::render() {
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ctx->ClearRenderTargetView(viewport_RTV, clearColor);
    ctx->ClearRenderTargetView(main_RTV, clearColor);


    static auto old_vp = viewport;
    ctx->OMSetBlendState(pBlendState, nullptr, 0xffffffff);

    viewport = { 0, 0, canvasSize.x, canvasSize.y };
    ctx->RSSetViewports(1, &viewport);

    ctx->VSSetShader(shaderManager->getShader(500)->vertexShader, nullptr, 0);

    ctx->PSSetShader(shaderManager->getShader(401)->pixelShader, nullptr, 0);

    ctx->PSSetSamplers(0, 1, &sampler);

    ctx->PSSetConstantBuffers(0, 1, &cBuffer);

    ctx->OMSetRenderTargets(1, &viewport_RTV, nullptr);

    ctx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->Draw(3, 0);

    ctx->ClearRenderTargetView(main_RTV, clearColor);

    int layerIndex = 0;

    adjustmentIndices.clear();
    adjustmentLayers.clear();

    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        Layer* layer = *it;
        if(layer->getType() == Layer::Type::Adjustment && layer->isVisible()) {
            adjustmentIndices.push_back(layerIndex);
            adjustmentLayers.push_back(layer);
        }
        layerIndex++;
    }

    layerIndex = 0;

    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        Layer* layer = *it;

        if(layer->getType() == Layer::Type::Pixel) {
            PixelLayer* pl = dynamic_cast<PixelLayer*>(layer);

            auto stack = pl->getEffectStack();
            stack->clear();

            int i = 0;
            for (int index : adjustmentIndices) {
                auto al = dynamic_cast<AdjustmentLayer*>(adjustmentLayers[i]);
                if(index > layerIndex) {
                    pl->addEffect(al->getAdjustmentType(), al->getAdjustmentData());
                }

                i++;
            }

            auto pos = layer->getPosition(), size = layer->getSize();

            viewport = { 0, 0, canvasSize.x / 4, canvasSize.y / 4 };
            ctx->RSSetViewports(1, &viewport);

            ctx->PSSetShader(shaderManager->getShader(401)->pixelShader, nullptr, 0);

            dev->CreateRenderTargetView(pl->getThumbnail(), nullptr, &thumbnail_RTV);
            ctx->ClearRenderTargetView(thumbnail_RTV, clearColor);

            ctx->OMSetRenderTargets(1, &thumbnail_RTV, nullptr);
            ctx->Draw(3, 0);

            ctx->PSSetShader(shaderManager->getShader(400)->pixelShader, nullptr, 0);

            D3D11_TEXTURE2D_DESC desc = {};
            ZeroMemory(&desc, sizeof(desc));

            desc.Width = layer->getSize().x;
            desc.Height = layer->getSize().y;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            int clusterIndex = 0;
            for (auto& t : pl->texture_cluster) {
                {
                    auto f = pos + vec::vec2{ (float)t.first.first * 64, (float)t.first.second * 64 };
                    if (f > vec::vec2{ canvasSize.x, canvasSize.y } || vec::vec2() > f)
                        continue;
                }

                ID3D11ShaderResourceView *tex_SRV;
                ID3D11ShaderResourceView *mask_SRV = nullptr;

                dev->CreateShaderResourceView(t.second, nullptr, &tex_SRV);
                if (layer->mask_cluster[t.first]) dev->CreateShaderResourceView(layer->mask_cluster[t.first], nullptr, &mask_SRV);


                ctx->PSSetShaderResources(0, 1, &tex_SRV);

                if (layer == activeLayer) {
                    for (auto effect: effectQueue) {
                        ctx->PSSetShader(effect.shader, nullptr, 0);

                        if (effect.data) {
                            ID3D11Buffer *constantBuffer = nullptr;

                            D3D11_BUFFER_DESC bd = {};
                            bd.Usage = D3D11_USAGE_DEFAULT;
                            bd.ByteWidth = sizeof(float) * 4 * effect.dataSize;
                            bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                            bd.CPUAccessFlags = 0;

                            dev->CreateBuffer(&bd, nullptr, &constantBuffer);

                            ctx->PSSetConstantBuffers(0, 1, &constantBuffer);
                            ctx->UpdateSubresource(constantBuffer, 0, nullptr, effect.data, 0, 0);

                            ID3D11RenderTargetView *tex_RTV = nullptr;
                            ID3D11Texture2D *tex = nullptr;

                            D3D11_TEXTURE2D_DESC desc{};
                            ZeroMemory(&desc, sizeof(desc));

                            desc.Width = 64;
                            desc.Height = 64;
                            desc.MipLevels = 1;
                            desc.ArraySize = 1;
                            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                            desc.SampleDesc.Count = 1;
                            desc.SampleDesc.Quality = 0;
                            desc.Usage = D3D11_USAGE_DEFAULT;
                            desc.BindFlags = D3D11_BIND_RENDER_TARGET;
                            desc.CPUAccessFlags = 0;
                            desc.MiscFlags = 0;

                            dev->CreateTexture2D(&desc, nullptr, &tex);
                            dev->CreateRenderTargetView(tex, nullptr, &tex_RTV);

                        //    ctx->CopyResource(tex, t.second);

                            viewport = {0, 0, 64, 64};
                            ctx->RSSetViewports(1, &viewport);
                            ctx->OMSetRenderTargets(1, &tex_RTV, nullptr);

                            ctx->Draw(3, 0);

                            ctx->CopyResource(t.second, tex);


                            {
                                desc.BindFlags = 0;
                                desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                                desc.Usage = D3D11_USAGE_STAGING;

                                ID3D11Texture2D* pStagingTexture = nullptr;
                                dev->CreateTexture2D(&desc, nullptr, &pStagingTexture);

                                ctx->CopyResource(pStagingTexture, tex);

                                D3D11_MAPPED_SUBRESOURCE mappedResource, mappedResource2;
                                ctx->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
                                ctx->Map(t.second, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource2);
                                memcpy(mappedResource2.pData, mappedResource.pData, mappedResource.RowPitch * 64);
                                ctx->Unmap(pStagingTexture, 0);
                                ctx->Unmap(t.second, 0);
                                pStagingTexture->Release();
                            }
                            tex->Release();
                            tex_RTV->Release();
                            constantBuffer->Release();
                        }

                        if (clusterIndex == pl->texture_cluster.size() - 1) {
                            delete effect.data;
                            if (!effectQueue.empty()) effectQueue.erase(effectQueue.begin());
                        }
                    }
                }

                auto& FXcluster = pl->FXTexture_cluster;
                ID3D11Texture2D* FXTexture = FXcluster[t.first];

                if (!FXTexture) {
                    pl->createSector(t.first, FXcluster);
                    FXTexture = FXcluster[t.first];
                }

                ctx->CopyResource(FXTexture, t.second);

                for (PixelLayer::Effect &effect: *stack) {

                    ID3D11Buffer *cb = nullptr;
                    unsigned int size;

                    switch (effect.type) {
                        case PixelLayer::EffectType::BRIGHTNESS_CONTRAST:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::COLOR_BALANCE:
                            size = 3;
                            break;
                        case PixelLayer::EffectType::CURVES:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::EXPOSURE:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::HUE_SATURATION:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::INVERT:
                            size = 0;
                            break;
                        case PixelLayer::EffectType::LEVELS:
                            size = 5;
                            break;
                        case PixelLayer::EffectType::MONOCHROME:
                            size = 0;
                            break;
                        case PixelLayer::EffectType::POSTERIZE:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::THRESHOLD:
                            size = 1;
                            break;
                        case PixelLayer::EffectType::VIBRANCE:
                            size = 1;
                            break;
                    }

                    if (effect.data) { pl->createConstantBuffer(size, &cb, effect.data); }

                    auto shader = shaderManager->getShader(static_cast<int>(effect.type) - 1 + 410)->pixelShader;

                    g::pd3dDeviceContext->PSSetShader(shader, nullptr, 0);

                    ID3D11Texture2D *renderFX;
                    ID3D11ShaderResourceView *effTex_SRV;

                    desc.Width = 64;
                    desc.Height = 64;
                    desc.MipLevels = 1;
                    desc.ArraySize = 1;
                    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    desc.SampleDesc.Count = 1;
                    desc.SampleDesc.Quality = 0;
                    desc.Usage = D3D11_USAGE_DEFAULT;
                    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
                    desc.CPUAccessFlags = 0;
                    desc.MiscFlags = 0;

                    dev->CreateTexture2D(&desc, nullptr, &renderFX);

                    dev->CreateShaderResourceView(FXTexture, nullptr, &effTex_SRV);

                    g::pd3dDeviceContext->PSSetShaderResources(0, 1, &effTex_SRV);

                    if (size > 0 && effect.data) { g::pd3dDeviceContext->PSSetConstantBuffers(0, 1, &cb); }

                    ID3D11RenderTargetView *RTV = nullptr;
                    g::pd3dDevice->CreateRenderTargetView(renderFX, nullptr, &RTV);

                    g::pd3dDeviceContext->OMSetRenderTargets(1, &RTV, nullptr);
                    g::pd3dDeviceContext->ClearRenderTargetView(RTV, clearColor);

                    viewport = {0, 0, 64, 64};
                    ctx->RSSetViewports(1, &viewport);

                    g::pd3dDeviceContext->Draw(3, 0);

                    ctx->CopyResource(FXTexture, renderFX);

                    if (size > 0 && effect.data) { cb->Release(); }

                    if (RTV) RTV->Release();
                    if (effTex_SRV) effTex_SRV->Release();
                    if (renderFX) renderFX->Release();
                }

                // workaround for effect SRV not rendering

                ID3D11Texture2D *effectTex = nullptr;
                ID3D11ShaderResourceView *effectTex_SRV;

                // on around line 389-390 the thing just crashes sometimes
                auto hs = dev->CreateTexture2D(&desc, nullptr, &effectTex);
                if (hs) throw;
                ctx->CopyResource(effectTex, FXTexture);

                dev->CreateShaderResourceView(FXTexture, nullptr, &effectTex_SRV);

                ctx->PSSetShader(shaderManager->getShader(400)->pixelShader, nullptr, 0);

                ctx->PSSetShaderResources(0, 1, &effectTex_SRV);
                ctx->PSSetShaderResources(1, 1, &mask_SRV);

                if (layer->isVisible()) {
                    ctx->PSSetShader(shaderManager->getShader(layer->getBlendMode())->pixelShader, nullptr, 0);
                    viewport = {t.first.first * 64 + pos.x, t.first.second * 64 + pos.y, 64, 64};
                    ctx->RSSetViewports(1, &viewport);
                    ctx->OMSetRenderTargets(1, &main_RTV, nullptr);
                    ctx->Draw(3, 0);

                    ctx->OMSetRenderTargets(1, &viewport_RTV, nullptr);
                    ctx->Draw(3, 0);
                }

                ctx->OMSetRenderTargets(1, &thumbnail_RTV, nullptr);

                viewport = {pos.x / 4, pos.y / 4, size.x / 4, size.y / 4};
                ctx->RSSetViewports(1, &viewport);

                ctx->Draw(3, 0);

                if (effectTex) effectTex->Release();
                if (effectTex_SRV) effectTex_SRV->Release();
                if (mask_SRV) mask_SRV->Release();
                if (tex_SRV) tex_SRV->Release();

                clusterIndex++;
            }

            if (thumbnail_RTV) thumbnail_RTV->Release();
            stack->clear();
        }

        layerIndex++;
    }

    ctx->OMSetRenderTargets(1, &g::mainRenderTargetView, nullptr);
    viewport = old_vp;
}

ID3D11Texture2D* Canvas::renderRegion(ID3D11Texture2D* originalTexture, vec::vec2 start, vec::vec2 end) {
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = end.x - start.x;
    desc.Height = end.y - start.y;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    ID3D11Texture2D* texture = nullptr;

    dev->CreateTexture2D(&desc, nullptr, &texture);

    D3D11_BOX box;
    box.left = start.x;
    box.top = start.y;
    box.right = end.x;
    box.bottom = end.y;
    box.front = 0;
    box.back = 1;

    ctx->CopySubresourceRegion(texture, 0, 0, 0, 0, originalTexture, 0, &box);

    return texture;
}

vec::vec4 Canvas::getColor(vec::vec2 coords, Layer* l) {
    vec::vec4 color = vec::vec4({0, 0, 0, 0});

    if(l->getType() == Layer::Type::Pixel) {
        PixelLayer* pl = dynamic_cast<PixelLayer*>(l);

        if (!pl->getTextureAtCoord(coords, false)) {
            !pl->getTextureAtCoord(coords, false, true);
            return color;
        }

        D3D11_TEXTURE2D_DESC desc;
        pl->getTextureAtCoord(coords, false)->GetDesc(&desc);

        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.Usage = D3D11_USAGE_STAGING;

        ID3D11Texture2D *pStagingTexture = nullptr;
        dev->CreateTexture2D(&desc, nullptr, &pStagingTexture);

        ctx->CopyResource(pStagingTexture, pl->getTextureAtCoord(coords, false));

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ctx->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

        BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
        unsigned int rowSize = mappedResource.RowPitch;

        coords = coords - l->getPosition();
        coords = coords - vec::vec2{ (float)((int)coords.x / 64) * 64, (float)((int)coords.y / 64) * 64 };

        if((int)coords.x >= 0 && (int)coords.x < desc.Width && (int)coords.y >= 0 && (int)coords.y < desc.Height) {
            color = vec::vec4({
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 0)] / 255.0f,
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 1)] / 255.0f,
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 2)] / 255.0f,
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 3)] / 255.0f
            });
        }

        ctx->Unmap(pStagingTexture, 0);
        pStagingTexture->Release();
    }

    return color;
}

void Canvas::setColor(vec::vec2 coords, vec::vec4 color, Layer* l) {
    if (l->getType() == Layer::Type::Pixel) {
        PixelLayer* pl = dynamic_cast<PixelLayer*>(l);

        auto tx = pl->getTextureAtCoord(coords, false, true);

        ID3D11Texture2D *t = nullptr;

        D3D11_TEXTURE2D_DESC desc = {};
        ZeroMemory(&desc, sizeof(desc));

        desc.Width = 64;
        desc.Height = 64;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        desc.MiscFlags = 0;

        dev->CreateTexture2D(&desc, nullptr, &t);
        ctx->CopyResource(t, tx);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ctx->Map(t, 0, D3D11_MAP_READ_WRITE, 0, &mappedResource);

        BYTE* mappedData = reinterpret_cast<BYTE*>(mappedResource.pData);
        unsigned int rowSize = mappedResource.RowPitch;

        coords = coords - l->getPosition();
        coords = coords - vec::vec2{ (float)((int)floorf(coords.x) / 64) * 64, (float)((int)floorf(coords.y) / 64) * 64 };
        if (coords.x < 0) coords.x += 64;
        if (coords.y < 0) coords.y += 64;
        if ((int)coords.x >= 0 && (int)coords.x < 64 && (int)coords.y >= 0 && (int)coords.y < 64) {
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 0)] = color.x * 255;
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 1)] = color.y * 255;
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 2)] = color.z * 255;
              mappedData[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 3)] = color.w * 255;
        }

        ctx->Unmap(t, 0);

        ctx->CopyResource(tx, t);

        t->Release();
    }

    return;
}

void Canvas::addLayer(Layer::Type type, __int64* groupPtr, vec::vec2 size, Layer::EffectType effectType) {
    int activeLayerIndex = 0;

    for(int i = 0; i < layers.size(); ++i) {
        if(layers[i] == activeLayer) {
            activeLayerIndex = i;
            break;
        }
    }

    bool breakL = false;

    while (true) {
        std::string name = "Layer " + std::to_string(lc);

        Layer* layer = nullptr;

        switch (type) {
            case Layer::Type::Pixel:
                if(isNameUnique(name)) {
                    layer = new PixelLayer(name.c_str(), groupPtr, size, canvasSize);
                    breakL = true;
                } else {
                    lc++;
                }
                break;
            case Layer::Adjustment:
                if(effectType != Layer::EffectType::NONE) {
                    if(isNameUnique(name)) {
                        layer = new AdjustmentLayer(name.c_str(), groupPtr, size, effectType);
                        breakL = true;
                    } else {
                        lc++;
                    }
                } else {
                    breakL = true;
                }
                break;
            case Layer::Shape:
                break;
            case Layer::Text:
                break;
        }

        if (layer != nullptr) {
            layers.insert(layers.begin() + activeLayerIndex, layer);
        }
        setActiveLayer(layer);

        if(breakL) break;
    }
}

void Canvas::addImageLayer(const char* filename) {
    FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
    FIBITMAP* image(0);
    DXGI_SAMPLE_DESC s;
    unsigned int width(0), height(0), bpp(0);
    s.Count = 1;

    fif = FreeImage_GetFileType(filename, 0);
    if (fif == FIF_UNKNOWN)
        fif = FreeImage_GetFIFFromFilename(filename);
    if (fif == FIF_UNKNOWN) {
        return;
    }

    if (FreeImage_FIFSupportsReading(fif))
        image = FreeImage_Load(fif, filename);
    if (!image) {
        return;
    }

    //FreeImage_FlipVertical(image);

    bpp = FreeImage_GetBPP(image);
    width = FreeImage_GetWidth(image);
    height = FreeImage_GetHeight(image);

    if ((bpp == 0) || (width == 0) || (height == 0)) {
        FreeImage_Unload(image);
        return;
    }
    FreeImage_FlipVertical(image);

    image = FreeImage_ConvertToRGBA16(image);
    image = FreeImage_ConvertTo32Bits(image);
    bpp = FreeImage_GetBPP(image);

    this->addLayer(Layer::Type::Pixel, nullptr, { (float)width, (float)height });
    auto topLayer = this->getActiveLayer();

    D3D11_MAPPED_SUBRESOURCE res = {FreeImage_GetBits(image), (unsigned int)width * 4, 0};

    PixelLayer* pl = dynamic_cast<PixelLayer*>(topLayer);

    for (auto& t : pl->texture_cluster) {
        int x = t.first.first;
        int y = t.first.second;

        g::pd3dDeviceContext->Map(t.second, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

        uint32_t rowSize = (unsigned int)width * 4;
        BYTE* mappedData = reinterpret_cast<BYTE*>(res.pData);
        for (int i = 0; i < 64 * 64; i++) {
            RGBQUAD l = { 0 };
            int x = t.first.first * 64 + i % 64, y = t.first.second * 64 + i / 64;
            if (x < width && y < height)   FreeImage_GetPixelColor(image, x, y, &l);
            memcpy(mappedData, &l, sizeof(RGBQUAD));
            mappedData += sizeof(RGBQUAD);
        }

        g::pd3dDeviceContext->Unmap(t.second, 0);
    }
    FreeImage_Unload(image);
}

void Canvas::removeLayer(const char* name) {
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        if (strcmp((*it)->getName(), name) == 0) {

            if((*it)->getType() == Layer::Type::Adjustment) {
                auto al = dynamic_cast<AdjustmentLayer*>(*it);

                auto i = std::find(adjustmentLayers.begin(), adjustmentLayers.end(), al);
                auto j = std::find(adjustmentIndices.begin(), adjustmentIndices.end(), i - adjustmentLayers.begin());

                adjustmentLayers.erase(i);
                adjustmentIndices.erase(j);

            }

            auto nextIt = it + 1;
            if(*it == activeLayer) {
                if(layers.size() > 1) {
                    if(nextIt != layers.end()) {
                        setActiveLayer(*nextIt);
                    } else {
                        setActiveLayer(*(it - 1));
                    }
                }
            }
            delete *it;
            layers.erase(it);
            break;
        }
    }
}

void Canvas::serialize(std::ostream& os) {
    os.write((char*)&canvasSize, sizeof(vec::vec2));

    int n = layers.size();
    os.write((char*)&n, sizeof(int));

    for(Layer* layer : layers) {
        layer->serialize(os);
    }
}

void Canvas::deserialize(std::istream& is) {
    for(Layer* layer : layers) {
        delete layer;
    }

    layers.clear();

    is.read((char*)&canvasSize, sizeof(vec::vec2));

    int n;
    is.read((char*)&n, sizeof(int));

    for(int i = 0; i < n; ++i) {
        Layer::Type type;

        auto pos = is.tellg();
        is.read((char *) &type, sizeof(Layer::Type));
        is.seekg(pos);

        Layer* layer;

        switch (type) {
            case Layer::Type::Pixel:
                layer = new PixelLayer(is);
                break;
            case Layer::Type::Adjustment:
                layer = new AdjustmentLayer(is);
                break;
            case Layer::Type::Shape:
                break;
            case Layer::Type::Text:
                break;
        }

        layers.push_back(layer);
    }

    setActiveLayer(layers[0]);
}

Layer *Canvas::getLayer(const char *name) {
    for (Layer* layer : layers) {
        if (strcmp(layer->getName(), name) == 0) {
            return layer;
        }
    }
    return nullptr;
}

bool Canvas::isNameUnique(const std::string& name) {
    for(auto layer : layers) {
        std::string lName = layer->getName();

        if(lName == name) {
            return false;
        }
    }

    return true;
}
