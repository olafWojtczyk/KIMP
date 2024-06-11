#include "handTool.h"
#include "../managers/projectManager.h"

extern ProjectManager* projectManager;

void HandTool::mouseHoldEvent(ImGuiMouseButton_ Button) {
    auto dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
    auto canvas = projectManager->getActiveProject().project->canvas;

    if(g::canvasHovered && Button == ImGuiMouseButton_Left) {
        canvas->offset = canvas->offset + vec::vec2({dt.x, dt.y});
    }
    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
}

void HandTool::render() {

}
