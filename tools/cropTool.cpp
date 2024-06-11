#include "cropTool.h"
#include "../managers/cropManager.h"

extern CropManager* cropManager;

void CropTool::mouseLeftEvent() {
    mouseHoldEvent(ImGuiMouseButton_Left);
}

void CropTool::mouseUpEvent() {

}

void CropTool::mouseRightEvent() {

}

void CropTool::render() {
    cropManager->setVisible(true);
    if (ImGui::Button("yes"))
        cropManager->crop();
}

void CropTool::mouseHoldEvent(ImGuiMouseButton_ Button) {

}
