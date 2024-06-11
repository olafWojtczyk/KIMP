#include "zoomTool.h"
#include "../managers/projectManager.h"
#include <cmath>

extern ProjectManager* projectManager;

void ZoomTool::mouseHoldEvent(ImGuiMouseButton_ Button) {
    if(!g::canvasHovered) { return; }

    if(Button == ImGuiMouseButton_Left) {
        auto dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

        float distance = (dt.x + 1.f - dt.y) / 2.f;

        if(distance != 0) {
            auto canvas = projectManager->getActiveProject().project->canvas;
            canvas->scaleFactor += distance/100;
        }

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
    } else if(Button == ImGuiMouseButton_Right) {
        auto dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);

        float distance = (dt.x+1.f - dt.y) / 2.f;

        if(distance != 0) {
            auto canvas = projectManager->getActiveProject().project->canvas;
            canvas->scaleFactor -= distance/10000;
        }

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Right);
    }
}

void ZoomTool::mouseLeftEvent() {
    if(!g::canvasHovered) { return; }

    auto canvas = projectManager->getActiveProject().project->canvas;
    canvas->scaleFactor *= 1.25;
}

void ZoomTool::mouseRightEvent() {
    if(!g::canvasHovered) { return; }

    auto canvas = projectManager->getActiveProject().project->canvas;
    canvas->scaleFactor /= 1.25;
}

void ZoomTool::render() {

}
