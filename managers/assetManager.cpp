#include "assetManager.h"
#include "../resource.h"
#include "../globals.h"
#include "../vec.h"

#include <windows.h>
#include <FreeImage.h>

void AssetManager::loadImages() {
    FreeImage_Initialise();

    for (int i = 600; i < 700; i++) {
        HRSRC hRes = FindResource(nullptr, MAKEINTRESOURCE(i), RT_RCDATA);
        if (hRes) {
            HGLOBAL hData = LoadResource(nullptr, hRes);
            if (hData) {
                char* pData = reinterpret_cast<char*>(LockResource(hData));
                if (pData) {
                    DWORD dwSize = SizeofResource(nullptr, hRes);

                    FIMEMORY* fiMem = FreeImage_OpenMemory(reinterpret_cast<BYTE*>(pData), dwSize);
                    if (fiMem) {
                        FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem);
                        if (fif != FIF_UNKNOWN) {
                            FIBITMAP* fiBitmap(nullptr);

                            fiBitmap = FreeImage_LoadFromMemory(fif, fiMem);

                            FreeImage_FlipVertical(fiBitmap);

                            fiBitmap = FreeImage_ConvertToRGBA16(fiBitmap);
                            fiBitmap = FreeImage_ConvertTo32Bits(fiBitmap);

                            vec::vec2 size = { (float)FreeImage_GetWidth(fiBitmap), (float)FreeImage_GetHeight(fiBitmap) };

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
                            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                            desc.MiscFlags = 0;

                            unsigned char* pSysMem = new unsigned char[(int)size.x * (int)size.y * 4];

                            memcpy(pSysMem, FreeImage_GetBits(fiBitmap), (int)size.x * (int)size.y * 4);

                            D3D11_SUBRESOURCE_DATA initData = { pSysMem, (unsigned int)size.x * 4, 0 };

                            g::pd3dDevice->CreateTexture2D(&desc, &initData, &g::imageTexture[i-600]);
                            g::pd3dDevice->CreateShaderResourceView(g::imageTexture[i-600], nullptr, &g::imageTextureSRV[i-600]);

                            g::loadedImages++;

                            delete[] pSysMem;

                            FreeImage_Unload(fiBitmap);
                        }
                        FreeImage_CloseMemory(fiMem);
                    }
                }
            }
        }
    }

    FreeImage_DeInitialise();
}
