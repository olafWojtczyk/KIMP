#pragma once

#include <d3d11.h>
#include <vector>
#include "vec.h"

namespace g {

    inline ID3D11Device* pd3dDevice = nullptr;
    inline ID3D11DeviceContext* pd3dDeviceContext = nullptr;
    inline IDXGISwapChain* pSwapChain = nullptr;
    inline ID3D11RenderTargetView* mainRenderTargetView = nullptr;

    inline UINT resizeWidth = 0, resizeHeight = 0;

    inline bool bInitialized = false;

    inline bool isEditingMask = false;

    inline std::vector<ID3D11Texture2D*> imageTexture(100, nullptr);
    inline std::vector<ID3D11ShaderResourceView*> imageTextureSRV(100, nullptr);
    inline unsigned int loadedImages = 0;

    inline int activeTool = -1;

    inline bool canvasHovered = false;
    inline bool viewportHovered = false;

    inline std::vector<ID3D11ShaderResourceView*> endFrameReleaseStackSRV;
    inline std::vector<const char*> popupStack;
}
