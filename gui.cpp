#include "gui.h"
#include "resource.h"
#include "imgui_internal.h"
#include "managers/fileManager.h"
#include "FreeImage.h"
#include "managers/effectManager.h"

extern ProjectManager* projectManager;
extern EffectManager* effectManager;

void gui::init() {
    wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"KIMP", nullptr };
    ::RegisterClassExW(&wc);

    hwnd = ::CreateWindowW(wc.lpszClassName, L"Krzak Image Manipulation Program", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDevice(hwnd)) {
        DestroyDevice();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof(darkMode));

    hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon)
    {
        SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)hIcon);
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    io = &ImGui::GetIO(); (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g::pd3dDevice, g::pd3dDeviceContext);



    HRSRC hRes = FindResource(nullptr, MAKEINTRESOURCE(MAINMENUBAR_XML), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(nullptr, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            if (pData) {
                DWORD dwSize = SizeofResource(nullptr, hRes);
                tinyxml2::XMLError eResult = mainMenuBar.Parse(static_cast<char*>(pData), dwSize);

                if (eResult != tinyxml2::XML_SUCCESS) {
                    throw std::runtime_error("Error: Failed to load UI layout");
                }
            }
        }
    }

    hRes = FindResource(nullptr, MAKEINTRESOURCE(EDITORLAYOUT_XML), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(nullptr, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            if (pData) {
                DWORD dwSize = SizeofResource(nullptr, hRes);
                tinyxml2::XMLError eResult = editorLayout.Parse(static_cast<char*>(pData), dwSize);

                if (eResult != tinyxml2::XML_SUCCESS) {
                    throw std::runtime_error("Error: Failed to load UI layout");
                }
            }
        }
    }

    hRes = FindResource(nullptr, MAKEINTRESOURCE(FONT_MAIN), RT_RCDATA);
    if (hRes) {
        HGLOBAL hData = LoadResource(nullptr, hRes);
        if (hData) {
            void* pData = LockResource(hData);
            if (pData) {
                DWORD dwSize = SizeofResource(nullptr, hRes);
                char* pData2 = new char[dwSize];
                memcpy(pData2, pData, dwSize);
                ImGui::GetIO().Fonts->AddFontFromMemoryTTF(pData2, dwSize, 13);

            }
         //   FreeResource(hData);
        }
    }
    g::bInitialized = true;
}


void LayoutItem::destroy() {
    delete item;
    if (window) {
        window->destroy();
        delete window;
    }
    delete this;
}

void gui::render() {
    if (IsIconic(hwnd)) {
        return;
    }

    if (g::resizeWidth != 0 && g::resizeHeight != 0)
    {
        DestroyRTV();
        g::pSwapChain->ResizeBuffers(0, g::resizeWidth, g::resizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g::resizeWidth = g::resizeHeight = 0;
        CreateRTV();
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.11f, 0.12f, 0.13f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if(ImGui::BeginMainMenuBar()) {
        tinyxml2::XMLElement* root = mainMenuBar.FirstChildElement("MainMenuBar");
        if (root) {
            for (tinyxml2::XMLElement* e = root->FirstChildElement("Menu"); e!= nullptr; e = e->NextSiblingElement("Menu")) {
                menuManager->processMenu(e, nullptr, true);
            }
        }
        ImGui::EndMainMenuBar();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    DXGI_SWAP_CHAIN_DESC desc;
    g::pSwapChain->GetDesc(&desc);

    UINT bufferWidth = desc.BufferDesc.Width;
    UINT bufferHeight = desc.BufferDesc.Height;

    ImGui::SetNextWindowSize(ImVec2((float)bufferWidth, (float)bufferHeight - (ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2) + 1), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2 - 1), ImGuiCond_Always, ImVec2(0.0f, 0.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.12f, 0.13f, 0.14f, 1.0f));

    if(ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs)) {
        tinyxml2::XMLElement* root = editorLayout.FirstChildElement("editorLayout");
        ImGuiID dockspace_id = ImGui::GetID("Main");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoUndocking| ImGuiDockNodeFlags_NoResize);
        if (root) {
            for (tinyxml2::XMLElement* e = root->FirstChildElement(); e != nullptr; e = e->NextSiblingElement()) {
                layout->processLayout(e, nullptr, true);
            }
        }
        ImGui::End();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    static bool nameSet = false;

    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(160, 0), ImGuiCond_Always);
    if(ImGui::BeginPopupModal("New File", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char name[256];
        if(!nameSet) {
            std::string defaultName = "Untitled-" + std::to_string(projectManager->getProjectCount()+1);
            strncpy(name, defaultName.c_str(), sizeof(name));
            nameSet = true;
        }

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f),"Project settings");
        ImGui::Separator();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9, 9));

        ImGui::InputText("##projectName", name, IM_ARRAYSIZE(name));

        ImGui::PopStyleVar();

        ImGui::Separator();

        ImGui::PopStyleColor();

        ImGui::BeginGroup();
        static int w = 640;
        static int h = 480;

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9, 7));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x*0.4f);
        ImGui::Dummy(ImVec2(0, 0));

        bool ivW = false, ivH = false;

        if(w < 1) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
            ivW = true;
        }

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Width");
        ImGui::InputInt("##projectWidth", &w, 0);

        if(ivW) {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

        if(h < 1) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
            ivH = true;
        }

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Height");
        ImGui::InputInt("##projectHeight", &h, 0);

        if(ivH) {
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }

        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::PopStyleColor();
        ImGui::EndGroup();

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(0, 0));

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Orientation");
        static int orientation = 0;
        static int prev_orientation = -1;

        prev_orientation = orientation;

        ImGui::RadioButton("##portrait", &orientation, 0); ImGui::SameLine();
        ImGui::RadioButton("##landscape", &orientation, 1);

        if (prev_orientation != orientation) {
            int temp = w;
            w = h;
            h = temp;
        }

        w >= h ? orientation = 1 : orientation = 0;

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 13.0f);

        if (w > 0 && h > 0 && name[0] != '\0') {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.45f, 0.90f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.99f, 1.0f));

        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.17f, 0.35f, 1.0f));
        }

        ImGui::BeginDisabled( !(w > 0 && h > 0 && name[0] != '\0') );

        if(ImGui::Button("Create", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f))) {
            if (w > 0 && h > 0 && name[0] != '\0') {
                ImGui::CloseCurrentPopup();
                nameSet = false;

                vec::vec2 dim = { (float)w, (float)h };

                projectManager->newProject(dim, name);
                projectManager->getActiveProject().project->canvas->addLayer(Layer::Type::Pixel, nullptr, dim);
            }
        }

        ImGui::EndDisabled();

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.25f, 0.26f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

        if (ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvail().x, 26.0f))) {
            ImGui::CloseCurrentPopup();
            nameSet = false;
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PopStyleColor();

        ImGui::PopStyleVar();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(700, 450), ImGuiCond_Always);
    if(ImGui::BeginPopupModal("Export", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        if(ImGui::BeginChild("##exportViewport", ImVec2(ImGui::GetContentRegionAvail().x / 5 * 4, 0), false, ImGuiWindowFlags_None )) {

            auto canvas = projectManager->getActiveProject().project->canvas;

            ImVec2 windowSize = ImGui::GetWindowSize();

            float scale;
            if(canvas->getSize().x > canvas->getSize().y) {
                scale = (windowSize.x - 100) / canvas->getSize().x;
            } else {
                scale = (windowSize.y - 100) / canvas->getSize().y;
            }

            ImVec2 imageSize = {canvas->getSize().x * scale, canvas->getSize().y * scale};

            vec::vec2 offset = projectManager->getActiveProject().project->canvas->offset;

            ImVec2 centerPos = ImVec2(
                    (windowSize.x - imageSize.x) * 0.5f + offset.x,
                    (windowSize.y - imageSize.y) * 0.5f + offset.y
            );

            ImGui::SetCursorPos(centerPos);

            canvas->render();

            ID3D11ShaderResourceView* SRV = canvas->viewport_SRV;

            ImGui::Image((void*)SRV, ImVec2(canvas->getSize().x * scale, canvas->getSize().y * scale));

            ImGui::EndChild();
        }

        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::BeginGroup();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.13f, 0.14f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.13f, 0.14f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

        float w = ImGui::GetContentRegionAvail().x / 4 * 3;

        ImGui::Dummy({0, 10});
        ImGui::Dummy({(ImGui::GetContentRegionAvail().x - w) / 2, 0}); ImGui::SameLine();

        ImGui::BeginGroup();

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Format");
        ImGui::Dummy({0, 3});
        ImGui::PushItemWidth(w);
        ImGui::Combo("##exportFormat", (int*)&projectManager->exportType, "PNG\0JPG\0BMP\0TGA\0");
        ImGui::PopItemWidth();

        ImGui::EndGroup();

        ImGui::Dummy({0, 10});
        ImGui::Dummy({(ImGui::GetContentRegionAvail().x - w) / 2, 0}); ImGui::SameLine();

        ImGui::BeginGroup();

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Image size");
        ImGui::Dummy({0, 3});
        ImGui::PushItemWidth(w);

        static unsigned int width = projectManager->getActiveProject().project->canvas->getSize().x;
        static unsigned int height = projectManager->getActiveProject().project->canvas->getSize().y;

        static float scale = 1.0f;

        static unsigned int prev_width = width;
        static unsigned int prev_height = height;

        static bool locked = false;

        ImGui::DragInt("##exportWidth", (int*)&width, 1.0f, 0, UINT32_MAX);

        if (!ImGui::IsItemActive() && !locked) {
            if (width != prev_width) {
                float aspect_ratio = (float)prev_width / (float)prev_height;
                height = width / aspect_ratio;
                prev_height = height;
                prev_width = width;
                scale = (float)width / projectManager->getActiveProject().project->canvas->getSize().x;
            }
        }

        ImGui::Dummy({0, 3});
        ImGui::DragInt("##exportHeight", (int*)&height, 1.0f, 0, UINT32_MAX);

        if (!ImGui::IsItemActive() && !locked) {
            if (height != prev_height) {
                float aspect_ratio = (float)prev_width / (float)prev_height;
                width = height * aspect_ratio;
                prev_width = width;
                prev_height = height;
                scale = (float)height / projectManager->getActiveProject().project->canvas->getSize().y;
            }
        }

        ImGui::Dummy({0, 10});

        static int iScale = scale * 100.0f;

        if(!ImGui::IsItemActive() && !ImGui::IsItemHovered() && !locked) {
            if (iScale != (int)(scale * 100.0f)) {
                iScale = (int)(scale * 100.0f);
            }
        }

        ImGui::DragInt("##exportScale", &iScale, 1, 0, UINT32_MAX, "%d%%", ImGuiSliderFlags_AlwaysClamp);

        if(!ImGui::IsItemActive()) {
            if (iScale != (int)(scale * 100.0f)) {
                scale = iScale / 100.0f;
                width = projectManager->getActiveProject().project->canvas->getSize().x * scale;
                height = projectManager->getActiveProject().project->canvas->getSize().y * scale;
            }
        }

        if(ImGui::IsItemActive() || ImGui::IsItemHovered()) {
            locked = true;
        } else {
            locked = false;
        }

        ImGui::PopItemWidth();

        ImGui::Dummy({0, 220});

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 13.0f);

        bool valid = width > 0 && height > 0;

        if (valid) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.45f, 0.90f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.99f, 1.0f));

        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.17f, 0.35f, 1.0f));
        }

        ImGui::BeginDisabled(!valid);

        if(ImGui::Button("Export", ImVec2(w, 26.0f))) {
            if (valid) {
                ImGui::CloseCurrentPopup();

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
                bitmap = FreeImage_Rescale(bitmap, width, height, FILTER_BICUBIC);

                LPCSTR filter;
                LPCSTR extension;

                switch (projectManager->exportType) {
                    case ProjectManager::ExportType::PNG:
                        filter = "PNG (*.png)\0*.png\0";
                        extension = "png";
                        break;
                    case ProjectManager::ExportType::JPG:
                        filter = "JPEG (*.jpeg)\0*.jpeg\0";
                        extension = "jpeg";
                        break;
                    case ProjectManager::ExportType::BMP:
                        filter = "BMP (*.bmp)\0*.bmp\0";
                        extension = "bmp";
                        break;
                    case ProjectManager::ExportType::TGA:
                        filter = "TARGA (*.tga)\0*.tga\0";
                        extension = "tga";
                        break;
                }

                OPENFILENAMEA ofn;
                CHAR* szFile;
                HWND hwnd = nullptr;
                HANDLE hf;

                szFile = new CHAR[32767];
                ZeroMemory(szFile, sizeof(CHAR) * 32767);

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = szFile;
                ofn.lpstrFile[0] = '\0';
                ofn.nMaxFile = 32767;
                ofn.lpstrFilter = filter;
                ofn.nFilterIndex = 1;
                ofn.lpstrFileTitle = nullptr;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = nullptr;
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
                ofn.lpstrDefExt = extension;

                if (GetSaveFileNameA(&ofn) == true)
                {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
                    std::string narrowStr = (ofn.lpstrFile);
                    const char* path = narrowStr.c_str();

                    switch (projectManager->exportType) {
                        case ProjectManager::ExportType::PNG:
                            FreeImage_Save(FIF_PNG, bitmap, path, 0);
                            break;
                        case ProjectManager::ExportType::JPG:
                            bitmap = FreeImage_ConvertTo24Bits(bitmap);

                            FreeImage_Save(FIF_JPEG, bitmap, path, JPEG_QUALITYSUPERB);
                            break;
                        case ProjectManager::ExportType::BMP:
                            FreeImage_Save(FIF_BMP, bitmap, path, 0);
                            break;
                        case ProjectManager::ExportType::TGA:
                            FreeImage_Save(FIF_TARGA, bitmap, path, 0);
                            break;
                    }

                    CloseHandle(hf);
                }

                delete[] szFile;

                g::pd3dDeviceContext->Unmap(pStagingTexture, 0);

                pStagingTexture->Release();

                FreeImage_Unload(bitmap);
            }
        }

        ImGui::EndDisabled();

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::Dummy({0, 5});

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.25f, 0.26f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

        if (ImGui::Button("Cancel", ImVec2(w, 26.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleColor();

        ImGui::PopStyleColor();

        ImGui::PopStyleVar();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();

        ImGui::EndGroup();

        ImGui::PopStyleColor(6);
        ImGui::PopStyleVar();

        ImGui::EndGroup();
        ImGui::EndPopup();
    }

    ImGui::PopStyleVar(2);

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Always);
    if(ImGui::BeginPopupModal("Convolution matrix", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        vec::vec2 size = {0, 0};

        if(projectManager->getActiveProject().project) { size = projectManager->getActiveProject().project->canvas->getActiveLayer()->getSize(); }

        struct Matrix {
            vec::vec3 vec_1;
            vec::vec3 vec_2;
            vec::vec3 vec_3;
        };

        struct Kernel {
            Matrix matrix;
            float denominator;
            float offset;
            vec::vec2 size;
            float amplifier;
            float padding[2];
        };

        static Kernel kernel = {
                {
                        {0, 0, 0},
                        {0, 1, 0},
                        {0, 0, 0}
                },
                1,
                0,
                size,
                1
        };

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9, 7));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Matrix");

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        ImGui::InputFloat3("##vec1", &kernel.matrix.vec_1.x);
        ImGui::InputFloat3("##vec2", &kernel.matrix.vec_2.x);
        ImGui::InputFloat3("##vec3", &kernel.matrix.vec_3.x);

        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / 2 - 4.5f);

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Divisor"); ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2 + 9.0f);
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Offset");

        ImGui::InputFloat("##denominator", &kernel.denominator); ImGui::SameLine();
        ImGui::InputFloat("##offset", &kernel.offset);
        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Amplifier");
        ImGui::InputFloat("##amp", &kernel.amplifier);

        ImGui::PopItemWidth();

        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 13.0f);

        float space = ImGui::GetContentRegionAvail().x / 2 - 4.5f;

        bool valid = kernel.denominator != 0 && projectManager->getActiveProject().project && projectManager->getActiveProject().project->canvas->getActiveLayer()->getType() == Layer::Type::Pixel;

        if (valid) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.45f, 0.90f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.99f, 1.0f));

        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.17f, 0.35f, 1.0f));
        }

        ImGui::BeginDisabled(!valid);

        if(ImGui::Button("Apply", ImVec2(space, 26.0f))) {
            if (valid) {
                Kernel* k = new Kernel();

                k->matrix = kernel.matrix;
                k->denominator = kernel.denominator;
                k->offset = kernel.offset;
                k->size = size;
                k->amplifier = kernel.amplifier;

                effectManager->pushEffect(EffectManager::FX::KERNEL, { k, 4 } );
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndDisabled();

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.25f, 0.26f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

        if (ImGui::Button("Cancel", ImVec2(space, 26.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar(3);

        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Always);
    if(ImGui::BeginPopupModal("Gaussian blur", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        vec::vec2 size = {0, 0};

        if(projectManager->getActiveProject().project) { size = projectManager->getActiveProject().project->canvas->getActiveLayer()->getSize(); }

        struct Blur {
            float strength;
            vec::vec2 direction;
            vec::vec2 size;
            float padding[3];
        };

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9, 7));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Strength");

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

        static float strength = 1.0f;

        ImGui::DragFloat("##strength", &strength, 0.01f, 1.0f, 50.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

        ImGui::PopItemWidth();

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 13.0f);

        float space = ImGui::GetContentRegionAvail().x / 2 - 4.5f;

        bool valid = strength != 0 && projectManager->getActiveProject().project && projectManager->getActiveProject().project->canvas->getActiveLayer()->getType() == Layer::Type::Pixel;

        if (valid) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.45f, 0.90f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.99f, 1.0f));

        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.17f, 0.35f, 1.0f));
        }

        ImGui::BeginDisabled(!valid);

        if(ImGui::Button("Apply", ImVec2(space, 26.0f))) {
            if (valid) {
                Blur* blurH = new Blur({ strength, { 1.0f, 0.0f }, { size.x, size.y } });
                Blur* blurV = new Blur({ strength, { 0.0f, 1.0f }, { size.x, size.y } });

                effectManager->pushEffect(EffectManager::FX::GAUSSIAN_BLUR, { blurH , 2 } );
                effectManager->pushEffect(EffectManager::FX::GAUSSIAN_BLUR, { blurV , 2 } );

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndDisabled();

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.25f, 0.26f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

        if (ImGui::Button("Cancel", ImVec2(space, 26.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar(3);

        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(250, 0), ImGuiCond_Always);
    if(ImGui::BeginPopupModal("Noise", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        struct Noise {
            float strength;
            bool monochrome;
            char padding[11];
        };

        static Noise noise = { 1.0f, false };

        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9, 7));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);

        ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Strength"); ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetFontSize() * 5); ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Monochrome");

        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 30.0f - 4.5f);

        ImGui::DragFloat("##strength", &noise.strength, 0.01f, 0.0f, 5.0f, "%.2f", ImGuiSliderFlags_Logarithmic);

        ImGui::PopItemWidth();

        ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(1.0f, 1.0f,1.0f, 1.0f));

        ImGui::SameLine();

        ImGui::Checkbox("##monochrome", &noise.monochrome);

        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);

        ImGui::Dummy(ImVec2(0, 5));

        ImGui::BeginGroup();

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 13.0f);

        float space = ImGui::GetContentRegionAvail().x / 2 - 4.5f;

        bool valid = projectManager->getActiveProject().project && projectManager->getActiveProject().project->canvas->getActiveLayer()->getType() == Layer::Type::Pixel;

        if (valid) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.45f, 0.90f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.55f, 0.99f, 1.0f));

        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.93f, 0.07f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.17f, 0.35f, 1.0f));
        }

        ImGui::BeginDisabled(!valid);

        if(ImGui::Button("Apply", ImVec2(space, 26.0f))) {
            if (valid) {
                Noise* n = new Noise({ noise.strength, noise.monochrome });

                effectManager->pushEffect(EffectManager::FX::NOISE, { n , 1 } );

                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndDisabled();

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.16f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.24f, 0.25f, 0.26f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);

        if (ImGui::Button("Cancel", ImVec2(space, 26.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::PopStyleColor(3);

        ImGui::PopStyleVar(3);

        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    for (auto& id : g::popupStack) {
        ImGui::OpenPopup(id);

        g::popupStack.erase(g::popupStack.begin());
    }

    ImGui::Render();
    const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    g::pd3dDeviceContext->OMSetRenderTargets(1, &g::mainRenderTargetView, nullptr);
    g::pd3dDeviceContext->ClearRenderTargetView(g::mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    for(auto SRV : g::endFrameReleaseStackSRV) {
        if (!SRV) continue;
        SRV->Release();
    }

    g::endFrameReleaseStackSRV.clear();

    g::pSwapChain->Present(1, 0);
}

void gui::shutdown() {
    if (g::bInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        DestroyDevice();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

        g::bInitialized = false;
    }
}

bool gui::CreateDevice(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g::pSwapChain, &g::pd3dDevice, &featureLevel, &g::pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g::pSwapChain, &g::pd3dDevice, &featureLevel, &g::pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRTV();

    return true;
}

void gui::DestroyDevice() {
    DestroyRTV();
    if (g::pSwapChain) { g::pSwapChain->Release(); g::pSwapChain = nullptr; }
    if (g::pd3dDeviceContext) { g::pd3dDeviceContext->Release(); g::pd3dDeviceContext = nullptr; }
    if (g::pd3dDevice) { g::pd3dDevice->Release(); g::pd3dDevice = nullptr; }

    delete canvas;
}

void gui::CreateRTV() {
    ID3D11Texture2D* pBackBuffer;
    g::pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g::pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g::mainRenderTargetView);
    pBackBuffer->Release();
}

void gui::DestroyRTV() {
    if (g::mainRenderTargetView) { g::mainRenderTargetView->Release(); g::mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT gui::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g::pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
            {
                g::resizeWidth = LOWORD(lParam);
                g::resizeHeight = HIWORD(lParam);
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)
                return 0;
            break;
        case WM_ACTIVATE:
            if (wParam == WA_INACTIVE ) {
                memset(io->KeysDown, 0, sizeof(io->KeysDown));
            } else {
                for ( int i = 0; i < 512; i++ ) {
                    keybd_event( (BYTE)i, MapVirtualKeyA( (BYTE)i, 0 ), 0x0002, 0 );
                }
            }
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}