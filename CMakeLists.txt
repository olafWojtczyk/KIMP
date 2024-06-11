cmake_minimum_required(VERSION 3.27)
project(KIMP)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

set(IMGUI_PATH "${CMAKE_SOURCE_DIR}/ext/imgui")
set(NLOHMANN_JSON_PATH "${CMAKE_SOURCE_DIR}/ext/nlohmann")
set(TINYXML2_PATH "${CMAKE_SOURCE_DIR}/ext/tinyxml-2")
set(FREEIMAGE_PATH "${CMAKE_SOURCE_DIR}/ext/FreeImage")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")

include_directories(${IMGUI_PATH})
include_directories(${NLOHMANN_JSON_PATH})
include_directories(${TINYXML2_PATH})
include_directories(${FREEIMAGE_PATH})

file(GLOB IMGUI_SOURCES "${IMGUI_PATH}/*.cpp")
file(GLOB TINYXML2_SOURCES "${TINYXML2_PATH}/*.cpp")

find_library(FREEIMAGE_LIB NAMES freeimage PATHS ${FREEIMAGE_PATH})

add_executable(KIMP main.cpp
        gui.cpp
        gui.h
        resources.rc
        resource.h
        managers/commandManager.cpp
        managers/commandManager.h
        managers/menuManager.h
        managers/shortcutManager.h
        managers/layoutManager.h
        managers/fileManager.cpp
        managers/fileManager.h
        managers/canvasManager.cpp
        managers/canvasManager.h
        vec.h
        layer.h
        managers/shaderManager.cpp
        managers/shaderManager.h
        globals.h
        managers/projectManager.cpp
        managers/projectManager.h
        gui/menus/newLayer.h
        managers/toolController.cpp
        tools/moveTool.cpp
        tools/moveTool.h
        managers/assetManager.cpp
        managers/assetManager.h
        tools/handTool.cpp
        tools/handTool.h
        managers/selectionManager.cpp
        managers/selectionManager.h
        tools/wandTool.cpp
        tools/wandTool.h
        tools/moveTool.cpp
        tools/moveTool.h
        tools/brushTool.cpp
        tools/brushTool.h
        tools/tool.cpp
        tools/tool.h
        tools/zoomTool.cpp
        tools/zoomTool.h
        tools/pipetteTool.cpp
        tools/pipetteTool.h
        tools/selectTool.cpp
        tools/selectTool.h
        tools/cropTool.cpp
        tools/cropTool.h
        managers/cropManager.cpp
        managers/cropManager.h
        gui/menus/export.h
        managers/effectManager.cpp
        managers/effectManager.h
        gui/menus/effect.h
        tools/shapeTool.cpp
)

add_library(ImGui STATIC ${IMGUI_SOURCES})
add_library(TinyXML2 STATIC ${TINYXML2_SOURCES})

target_link_libraries(KIMP ImGui d3d11 dxgi d3dcompiler dxguid dwmapi TinyXML2 ${FREEIMAGE_LIB})