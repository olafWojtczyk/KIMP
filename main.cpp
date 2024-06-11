#include <iostream>
#include "gui.h"
#include "managers/fileManager.h"
#include "globals.h"
#include "managers/projectManager.h"
#include "gui/menus/newLayer.h"

#include <FreeImage.h>
#include "managers/toolController.h"
#include "managers/assetManager.h"
#include "gui/menus/export.h"
#include "managers/shaderManager.h"
#include "managers/cropManager.h"
#include "managers/effectManager.h"

#include "gui/menus/effect.h"

CommandManager* commandManager = nullptr;
ShortcutManager* shortcutManager = nullptr;
FileManager* fileManager = nullptr;
ShaderManager* shaderManager = nullptr;
ProjectManager* projectManager = nullptr;
ToolController* toolController = nullptr;
AssetManager* assetManager = nullptr;
CropManager* cropManager = nullptr;
EffectManager* effectManager = nullptr;

int main() {
#if (defined(_DEBUG) && false)
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif

    commandManager = new CommandManager();
    fileManager = new FileManager();
    shortcutManager = new ShortcutManager();

    FreeImage_Initialise();
    gui::init();

    shaderManager = new ShaderManager();

    projectManager = new ProjectManager();
    toolController = new ToolController();

    assetManager = new AssetManager();
    AssetManager::loadImages();

    cropManager = new CropManager();
    effectManager = new EffectManager();

    commandManager->registerCommand("action.workspace.layer.raster.new", menus::addLayer);
    commandManager->registerCommand("action.workspace.exportPNG", menus::quickExport);

    commandManager->registerCommand("action.workspace.layer.filter.blur.gaussian", menus::gaussianBlurFX);
    commandManager->registerCommand("action.workspace.layer.filter.basic.convolution", menus::kernelFX);
    commandManager->registerCommand("action.workspace.layer.filter.noise.add", menus::noseFX);

    bool exit = false;
    while (!exit) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            switch (msg.message) {
                case WM_QUIT:
                    exit = true;
                    break;
            }
        }
        if (exit) break;

        gui::render();
        shortcutManager->handle();
    }

    gui::shutdown();

    return 0;
}
