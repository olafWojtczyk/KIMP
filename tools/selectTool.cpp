#include "selectTool.h"
#include "../managers/projectManager.h"

extern ProjectManager* projectManager;

void SelectTool::mouseHoldEvent(ImGuiMouseButton_ Button) {
    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;
    if(Button == ImGuiMouseButton_Left) {
        auto dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);

        if(g::canvasHovered) {
            end = start + vec::vec2({dt.x / scaleFactor, dt.y / scaleFactor});
        }
    }
}

void SelectTool::currentModeStyle(mode currentMode, mode mode) {
    if(currentMode == mode) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.08f, 0.09f, 0.10f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.11f, 0.12f, 0.13f, 1.0f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void SelectTool::render() {
    ImGui::Dummy({0, ImGui::GetContentRegionAvail().y / 6});

    ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().y / 1.5f, ImGui::GetContentRegionAvail().y / 1.5f);

    ImGui::Dummy({ImGui::GetContentRegionAvail().y / 6, 0}); ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);

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

    currentModeStyle(currentMode, mode::INTERSECT);

    if(ImGui::Button("I##intersectSelection", size)) {
        currentMode = mode::INTERSECT;
    }

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(10);
}

void SelectTool::mouseLeftEvent() {
    ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;
    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

    if(g::canvasHovered) {
        pressTime = std::chrono::steady_clock::now();

        if (currentMode == mode::NEW) {
            projectManager->getActiveProject().project->canvas->selectionManager->clearSelection();
        }

        start = vec::vec2({(ImGui::GetMousePos().x - canvasPos.x) / scaleFactor,(ImGui::GetMousePos().y - canvasPos.y) / scaleFactor});
    }
}

void SelectTool::mouseUpEvent() {
    if(g::canvasHovered) {
        releaseTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(releaseTime - pressTime);

        if(start.x > end.x || start.y > end.y) {
            std::swap(start.x, end.x);
            std::swap(start.y, end.y);
        }

        if(start.x != end.x || start.y != end.y) {
            //if short click, clear, if long, select
            if (duration > std::chrono::milliseconds(100)) {
                switch (currentMode) {
                    case mode::NEW:
                        projectManager->getActiveProject().project->canvas->selectionManager->selectPixels(start, end);
                        break;
                    case mode::ADD:
                        projectManager->getActiveProject().project->canvas->selectionManager->selectPixels(start, end);
                        break;
                    case mode::SUBTRACT:
                        projectManager->getActiveProject().project->canvas->selectionManager->deselectPixels(start, end);
                        break;
                    case mode::INTERSECT:
                        std::unordered_set<vec::vec2> intersectedPixels;
                        auto& selectionManager = projectManager->getActiveProject().project->canvas->selectionManager;
                        auto& selectedPixels = selectionManager->getSelectedPixels();

                        for (const auto& pixel : selectedPixels) {
                            if (pixel.x >= start.x && pixel.x <= end.x && pixel.y >= start.y && pixel.y <= end.y) {
                                intersectedPixels.insert(pixel);
                            }
                        }

                        selectionManager->clearSelection();
                        for (const auto& pixel : intersectedPixels) {
                            selectionManager->selectPixel(pixel);
                        }
                        break;
                }
            } else {
                if (currentMode == mode::NEW) {
                    projectManager->getActiveProject().project->canvas->selectionManager->clearSelection();
                }
            }
        }
    }
}
