#include "pipetteTool.h"
#include "../managers/projectManager.h"

extern ProjectManager* projectManager;

void PipetteTool::mouseRightEvent() {
    if(!g::canvasHovered) return;

    auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();
    ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;

    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

    vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });
    vec::vec4 col = projectManager->getActiveProject().project->canvas->getColor(vec::vec2({mousePos.x, mousePos.y}), layer);

    projectManager->getActiveProject().project->canvas->bgColor = { col.x, col.y, col.z, col.w };

    ImGui::BeginTooltip();
    ImGui::ColorButton("Pixel", { col.x, col.y, col.z, col.w }, 0, { 48,48 });
    ImGui::Text("XY %.2f %.2f", mousePos.x, mousePos.y);
    ImGui::Text("RGBA %.2f %.2f %.2f %.2f", col.x, col.y, col.z, col.w );
    ImGui::EndTooltip();
}

void PipetteTool::mouseLeftEvent() {
    if(!g::canvasHovered) return;

    auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();
    ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;

    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

    vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });
    vec::vec4 col = projectManager->getActiveProject().project->canvas->getColor(vec::vec2({mousePos.x, mousePos.y}), layer);

    projectManager->getActiveProject().project->canvas->fgColor = { col.x, col.y, col.z, col.w };

    ImGui::BeginTooltip();
    ImGui::ColorButton("Pixel", { col.x, col.y, col.z, col.w }, 0, { 48,48 });
    ImGui::Text("XY %.2f %.2f", mousePos.x, mousePos.y);
    ImGui::Text("RGBA %.2f %.2f %.2f %.2f", col.x, col.y, col.z, col.w );
    ImGui::EndTooltip();
}

void PipetteTool::mouseHoldEvent(ImGuiMouseButton_ Button) {
    if(g::canvasHovered) {
        auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();
        ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;

        float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

        vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });

        if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x > projectManager->getActiveProject().project->canvas->getSize().x || mousePos.y > projectManager->getActiveProject().project->canvas->getSize().y) return;

        vec::vec4 col = projectManager->getActiveProject().project->canvas->getColor(vec::vec2({mousePos.x, mousePos.y}), layer);

        if (Button == ImGuiMouseButton_Left)
            projectManager->getActiveProject().project->canvas->fgColor = { col.x, col.y, col.z, col.w };
        else if (Button == ImGuiMouseButton_Right)
            projectManager->getActiveProject().project->canvas->bgColor = { col.x, col.y, col.z, col.w };

        ImGui::BeginTooltip();
        ImGui::ColorButton("Pixel", { col.x, col.y, col.z, col.w }, 0, { 48,48 });
        ImGui::Text("XY %.2f %.2f", mousePos.x, mousePos.y);
        ImGui::Text("RGBA %.2f %.2f %.2f %.2f", col.x, col.y, col.z, col.w );
        ImGui::EndTooltip();
    }
}

void PipetteTool::render() {

}
