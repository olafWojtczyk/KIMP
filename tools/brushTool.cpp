#include "brushTool.h"
#include "../managers/projectManager.h"

extern ProjectManager* projectManager;

void BrushTool::mouseLeftEvent() {
    mouseHoldEvent(ImGuiMouseButton_Left);
}

void BrushTool::mouseUpEvent() {

}

void BrushTool::mouseRightEvent() {

}

void BrushTool::render() {
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().y * 1.75);
    ImGui::DragInt("size", &size, 1, 1000); ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().y * 1.75);
    ImGui::DragFloat("opacity", &opacity, 0.f, 1.f);

}

void BrushTool::mouseHoldEvent(ImGuiMouseButton_ Button) {

    static ImVec2 lastMousePos = ImGui::GetMousePos(); // Store the last mouse position

    auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();
    ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;

    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

    auto c = projectManager->getActiveProject().project->canvas->fgColor;
    vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });

    float distance = std::sqrt(std::pow(mousePos.x - lastMousePos.x, 2) + std::pow(mousePos.y - lastMousePos.y, 2));

    int numDots = static_cast<int>(distance);
   
    switch (type) {
    case 0:
        for (int i = 0; i < size * size; i++) {
            int x = i % size;
            int y = i / size;
            auto v = vec::vec2({ mousePos.x + x - size / 2, mousePos.y + y - size / 2 });
            float distance = std::sqrt(std::pow(v.x - mousePos.x, 2) + std::pow(v.y - mousePos.y, 2));
            float radius = size / 2;
            if (distance <= radius) {
                projectManager->getActiveProject().project->canvas->setColor(v, { c.x, c.y, c.z, c.w }, layer);
            }
        }
        break;
    case 1:
        for (int i = 0; i < size * size; i++) {
            auto v = vec::vec2({ mousePos.x + i % size - size / 2, mousePos.y + i / size - size / 2 });
            projectManager->getActiveProject().project->canvas->setColor(v, { c.x,c.y,c.z,c.w }, layer);
        }
        break;
    case 2:
        for (int i = 0; i < size * size; i++) {
            auto v = vec::vec2({ mousePos.x + i % size - size / 2, mousePos.y + i / size - size / 2 });
            projectManager->getActiveProject().project->canvas->setColor(v, { 0,0,0,0 }, layer);
        }
        break;

    }

}
