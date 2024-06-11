#define NOMINMAX
#include "wandTool.h"
#include "../managers/projectManager.h"
#include "../managers/shaderManager.h"

#include <cmath>
#include <stack>
#include <thread>
#include <mutex>

////////////////////////////////////////
//                                    //
//       FIX FOR IMAGE OFFSET         //
//                                    //
////////////////////////////////////////

extern ProjectManager* projectManager;
extern ShaderManager* shaderManager;

void WandTool::currentModeStyle(mode currentMode, mode mode) {
    if(currentMode == mode) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void WandTool::render() {
    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y / 6});

    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().y / 1.5f, ImGui::GetContentRegionAvail().y / 1.5f);

    ImGui::Dummy({ImGui::GetContentRegionAvail().y / 6, 0}); ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.25);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);

    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.11f, 0.12f, 0.13f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

    currentModeStyle(currentMode, mode::NEW);

    if(ImGui::Button("N##newSelection", size)) {
        currentMode = mode::NEW;
    } ImGui::SameLine();

    currentModeStyle(currentMode, mode::ADD);

    if(ImGui::Button("A##addSelection", size)) {
        currentMode = mode::ADD;
    } ImGui::SameLine();

    currentModeStyle(currentMode, mode::SUBTRACT);

    if(ImGui::Button("S##subtractSelection", size)) {
        currentMode = mode::SUBTRACT;
    } ImGui::SameLine();

    ImGui::PopStyleColor(8);

    static int tolerance = threshold * 100.0f;

    ImGui::Dummy({15, 0}); ImGui::SameLine();

    ImGui::TextColored(ImVec4(0.65f, 0.65f, 0.65f, 1.0f), "Tolerance: "); ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.13f, 0.14f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));

    ImGui::Dummy({0, 1}); ImGui::SameLine();

    ImGui::BeginGroup();

    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().y * 1.75);
    ImGui::DragInt("##tolerance", &tolerance, 1, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);

    ImGui::EndGroup();

    ImGui::PopStyleVar(3);

    ImGui::PopStyleColor(4);

    ImGui::PopItemWidth();

    threshold = tolerance / 100.0f;
}

vec::vec4 getPixel(vec::vec2 coords, BYTE *data, int rowSize, vec::vec2 size) {

    vec::vec4 color = vec::vec4({0, 0, 0, 0});

    if(data != nullptr) {
        if((int)coords.x >= 0 && (int)(coords.x - 1.0f) < size.x && (int)coords.y >= 0 && (int)coords.y < size.y  - 1) {
            color = vec::vec4({
                  data[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 0)] / 255.0f,
                  data[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 1)] / 255.0f,
                  data[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 2)] / 255.0f,
                  data[(unsigned int)((int)coords.y * rowSize + (int)coords.x * 4 + 3)] / 255.0f
            });
        }
    }

    return color;
}

bool isSimilar(const vec::vec4& color1, const vec::vec4& color2, float threshold) {
    float diff = std::sqrt(std::pow(color1.x - color2.x, 2) + std::pow(color1.y - color2.y, 2) + std::pow(color1.z - color2.z, 2) + std::pow(color1.w - color2.w, 2));
    return diff <= threshold;
}

void WandTool::dfs(int x, int y, std::vector<std::vector<bool>> visited, vec::vec2 size, vec::vec2 offset, BYTE* data, int rowSize) {
    std::stack<std::pair<int, int>> stack;
    stack.push({x, y});

    while (!stack.empty()) {
        std::pair<int, int> pos = stack.top();
        stack.pop();

        int x = pos.first;
        int y = pos.second;

        if (x < 0 || y < 0 || x >= size.x || y >= size.y) {
            continue;
        }

        if (visited[x][y]) {
            continue;
        }

        vec::vec4 currentColor = getPixel(vec::vec2({(float)x, (float)y}), data, rowSize, size);

        if (!isSimilar(originalColor, currentColor, threshold)) {
            continue;
        }

        vec::vec2 pixel = vec::vec2({std::round((float)x + offset.x), std::round((float)y + offset.y)});

        switch (currentMode) {
            case mode::NEW:
                projectManager->getActiveProject().project->canvas->selectionManager->selectPixel(pixel);
                break;
            case mode::ADD:
                projectManager->getActiveProject().project->canvas->selectionManager->selectPixel(pixel);
                break;
            case mode::SUBTRACT:
                projectManager->getActiveProject().project->canvas->selectionManager->deselectPixel(pixel);
                break;
        }

        visited[x][y] = true;

        stack.push({x - 1, y});
        stack.push({x + 1, y});
        stack.push({x, y - 1});
        stack.push({x, y + 1});
    }
}

void WandTool::mouseLeftEvent() {
    if(g::canvasHovered) {
        if (currentMode == mode::NEW) {
            projectManager->getActiveProject().project->canvas->selectionManager->clearSelection();
        }

        auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();
        ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;

        float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

        vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });
        originalColor = projectManager->getActiveProject().project->canvas->getColor(vec::vec2({mousePos.x, mousePos.y}), layer);

        auto canvas = projectManager->getActiveProject().project->canvas;

        int regions = 8;
        std::vector<std::thread> threads;

        std::mutex dxMutex;

        if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x >= layer->getSize().x / scaleFactor || mousePos.y >= layer->getSize().y / scaleFactor) return;

//        if (layer != nullptr) {
//            if (layer->getTexture() != nullptr) {
//                for (int i = 1; i <= regions; i++) {
//                    threads.emplace_back([&, i]() {
//                        vec::vec2 regStart = vec::vec2({0, (layer->getSize().y / regions) * (i - 1)});
//                        vec::vec2 regEnd = vec::vec2({layer->getSize().x, (layer->getSize().y / regions) * i});
//
//                        std::lock_guard<std::mutex> lock(dxMutex);
//                        ID3D11Texture2D *region = canvas->renderRegion(layer->getTexture(),
//                                                                       {std::ceil(regStart.x), std::ceil(regStart.y)},
//                                                                       {std::ceil(regEnd.x), std::ceil(regEnd.y)});
//
//                        D3D11_MAPPED_SUBRESOURCE mappedResource;
//                        g::pd3dDeviceContext->Map(region, 0, D3D11_MAP_READ, 0, &mappedResource);
//
//                        BYTE *pData = static_cast<BYTE *>(mappedResource.pData);
//                        int rowSize = mappedResource.RowPitch;
//
//                        std::vector<std::vector<bool>> visited((int) (std::ceil((regEnd - regStart).x)),
//                                                               std::vector<bool>(
//                                                                       (int) (std::ceil((regEnd - regStart).y) + 1),
//                                                                       false));
//
//                        vec::vec2 closestPixel;
//                        float minDistance = std::numeric_limits<float>::max();
//
//                        for (int x = 0; x < (int) (std::ceil((regEnd - regStart).x)); x++) {
//                            for (int y = 0; y < (int) (std::ceil((regEnd - regStart).y) + 1); y++) {
//                                vec::vec4 color = getPixel(vec::vec2({(float) x, (float) y}), pData, rowSize, vec::vec2(
//                                        {std::ceil((regEnd - regStart).x), std::ceil((regEnd - regStart).y)}));
//
//                                vec::vec2 currentPixel = vec::vec2({(float) x, (float) y});
//                                float distance = std::sqrt(std::pow(mousePos.x - currentPixel.x, 2) +
//                                                           std::pow(mousePos.y - currentPixel.y, 2));
//
//                                if (isSimilar(originalColor, color, threshold) && distance < minDistance) {
//                                    minDistance = distance;
//                                    closestPixel = currentPixel;
//                                }
//                            }
//                        }
//
//                        if (minDistance != std::numeric_limits<float>::max()) {
//                            dfs(closestPixel.x, closestPixel.y, visited,
//                                vec::vec2({std::ceil((regEnd - regStart).x), std::ceil((regEnd - regStart).y) + 1}),
//                                regStart + layer->getPosition(), pData, rowSize);
//                        }
//
//                        g::pd3dDeviceContext->Unmap(region, 0);
//                    });
//                }
//
//                for (auto& thread : threads) {
//                    thread.join();
//                }
//
//                auto a = projectManager->getActiveProject().project->canvas->selectionManager->getSelectedPixels();
//            }
//        }
    }
}
