#pragma once

#include <windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "managers/shortcutManager.h"
#include "managers/commandManager.h"
#include "globals.h"

#include <d3d11.h>
#include <dwmapi.h>
#include "tinyxml2.h"
#include "managers/menuManager.h"
#include "managers/layoutManager.h"
#include "managers/canvasManager.h"

namespace gui {
    inline MenuManager* menuManager = new MenuManager();
    inline LayoutManager* layout = new LayoutManager();

    inline Canvas* canvas = nullptr;

    inline tinyxml2::XMLDocument mainMenuBar;
    inline tinyxml2::XMLDocument editorLayout;

    inline ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    inline ImGuiIO* io = nullptr;

    inline WNDCLASSEXW wc;
    inline HWND hwnd;

    inline HICON hIcon;

    void init();
    void render();
    void shutdown();

    bool CreateDevice(HWND hWnd);
    void DestroyDevice();
    void CreateRTV();
    void DestroyRTV();
}