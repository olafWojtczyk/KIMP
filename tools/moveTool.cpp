#include "moveTool.h"
#include "../managers/commandManager.h"
#include "../managers/projectManager.h"

extern CommandManager* commandManager;
extern ProjectManager* projectManager;

void MoveTool::render() {
    float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

    if(isDragging) {
        off.x += dt.x / scaleFactor;
        off.y += dt.y / scaleFactor;

        if(ImGui::BeginTooltip()) {
            ImGui::Text("x: %d px", (int)off.x);
            ImGui::Text("y: %d px", (int)off.y);

            ImGui::EndTooltip();
        }
    }
}

void MoveTool::mouseHoldEvent(ImGuiMouseButton_ Button) {
    if(Button == ImGuiMouseButton_Left) {
        dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
        auto layer = projectManager->getActiveProject().project->canvas->getActiveLayer();

        ImVec2 canvasPos = projectManager->getActiveProject().project->viewportScreenPos;
        float scaleFactor = projectManager->getActiveProject().project->canvas->scaleFactor;

        vec::vec2 mousePos = vec::vec2({ (ImGui::GetMousePos().x - canvasPos.x) / scaleFactor, (ImGui::GetMousePos().y - canvasPos.y) / scaleFactor });
        if (!projectManager->getActiveProject().project->canvas->selectionManager->getSelectedPixels().empty()
            && projectManager->getActiveProject().project->canvas->selectionManager->isOpAllowed(mousePos)) {
            auto s = projectManager->getActiveProject().project->canvas->selectionManager;
            for (auto& i : s->getSelectedPixels()) {
            }
        }
        else {
            vec::vec4 col = projectManager->getActiveProject().project->canvas->getColor(vec::vec2({ mousePos.x, mousePos.y }), layer);

            if (col.w != 0) {
                layer->setPosition(layer->getPosition() + vec::vec2({ dt.x / scaleFactor, dt.y / scaleFactor }));
            }
        }

        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

        isDragging = true;
    }
}

void MoveTool::mouseUpEvent() {
    isDragging = false;

    off = ImVec2(0, 0);
}
