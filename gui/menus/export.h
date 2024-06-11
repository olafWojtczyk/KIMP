#include "../../managers/projectManager.h"
#include "FreeImage.h"

extern ProjectManager* projectManager;

namespace menus {
    void quickExport() {
        projectManager->exportType = ProjectManager::ExportType::PNG;

        auto exportTex = projectManager->getActiveProject().project->canvas->main_canvas;

        ID3D11Texture2D* pStagingTexture;

        D3D11_TEXTURE2D_DESC desc;
        exportTex->GetDesc(&desc);

        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;

        g::pd3dDevice->CreateTexture2D(&desc, nullptr, &pStagingTexture);
        g::pd3dDeviceContext->CopyResource(pStagingTexture, exportTex);

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        g::pd3dDeviceContext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

        BYTE* data = reinterpret_cast<BYTE*>(mappedResource.pData);

        FIBITMAP* bitmap;

        bitmap = FreeImage_ConvertFromRawBits(data, desc.Width, desc.Height, mappedResource.RowPitch, 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK, TRUE);

        OPENFILENAMEW ofn;
        WCHAR* szFile;
        HWND hwnd = nullptr;
        HANDLE hf;

        szFile = new WCHAR[32767];
        ZeroMemory(szFile, sizeof(WCHAR) * 32767);

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = 32767;
        ofn.lpstrFilter = L"PNG (*.png)\0*.png\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = nullptr;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        ofn.lpstrDefExt = L"png";

        if (GetSaveFileNameW(&ofn) == true)
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string narrowStr = converter.to_bytes(ofn.lpstrFile);
            const char* path = narrowStr.c_str();

            FreeImage_Save(FIF_PNG, bitmap, path, 0);

            CloseHandle(hf);
        }

        delete[] szFile;

        g::pd3dDeviceContext->Unmap(pStagingTexture, 0);

        pStagingTexture->Release();

        FreeImage_Unload(bitmap);
    }
}