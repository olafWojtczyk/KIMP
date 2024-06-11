#include "cropManager.h"
#include "imgui.h"
#include "projectManager.h"

extern ProjectManager* projectManager;

CropManager::CropManager() {
    start = vec::vec2({0, 0});
    end = vec::vec2({0, 0});
}

void CropManager::renderCropBox() {
    if (visible) {
        ImVec2 pos = projectManager->getActiveProject().project->viewportScreenPos;
        float scale = projectManager->getActiveProject().project->canvas->scaleFactor;

        vec::vec2 size = projectManager->getActiveProject().project->canvas->getSize() / vec::vec2({scale, scale});

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImU32 color = IM_COL32(255, 255, 255, 255);

        if(start.x < end.x && start.y < end.y) {
            draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + start.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + start.y * scale), ImVec2(pos.x + start.x * scale, pos.y + end.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + end.x * scale, pos.y + start.y * scale), ImVec2(pos.x + size.x, pos.y + end.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + end.y * scale), ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 150));
        } else if (start.x > end.x && start.y > end.y) {
            draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + end.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + end.y * scale), ImVec2(pos.x + end.x * scale, pos.y + start.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + start.x * scale, pos.y + end.y * scale), ImVec2(pos.x + size.x, pos.y + start.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x, pos.y + start.y * scale), ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 150));
        } else if (start.x > end.x && start.y < end.y) {
            draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + start.y * scale), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + end.x * scale, pos.y + start.y * scale), ImVec2(pos.x, pos.y + size.y), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + end.x * scale, pos.y + end.y * scale), ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + start.x * scale, pos.y + start.y * scale), ImVec2(pos.x + size.x, pos.y + end.y * scale), IM_COL32(0, 0, 0, 150));
        } else {
            draw_list->AddRectFilled(pos, ImVec2(pos.x + start.x * scale, pos.y + size.y), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + start.x * scale, pos.y + end.y * scale), ImVec2(pos.x + end.x * scale, pos.y), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + start.x * scale, pos.y + start.y * scale), ImVec2(pos.x + end.x * scale, pos.y + size.y), IM_COL32(0, 0, 0, 150));
            draw_list->AddRectFilled(ImVec2(pos.x + end.x * scale, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 150));
        }

        draw_list->AddRect(ImVec2(pos.x + start.x * scale, pos.y + start.y * scale), ImVec2(pos.x + end.x * scale, pos.y + end.y * scale), color, 0.0f, 0, 1.5f);

        ImVec2 left1_3rd = ImVec2(pos.x + start.x * scale, pos.y + start.y * scale + (end.y - start.y) / 3 * scale);
        ImVec2 left2_3rd = ImVec2(pos.x + start.x * scale, pos.y + start.y * scale + (end.y - start.y) / 3 * 2 * scale);
        ImVec2 right1_3rd = ImVec2(pos.x + end.x * scale, pos.y + start.y * scale + (end.y - start.y) / 3 * scale);
        ImVec2 right2_3rd = ImVec2(pos.x + end.x * scale, pos.y + start.y * scale + (end.y - start.y) / 3 * 2 * scale);

        ImVec2 top1_3rd = ImVec2(pos.x + start.x * scale + (end.x - start.x) / 3 * scale, pos.y + start.y * scale);
        ImVec2 top2_3rd = ImVec2(pos.x + start.x * scale + (end.x - start.x) / 3 * 2 * scale, pos.y + start.y * scale);
        ImVec2 bottom1_3rd = ImVec2(pos.x + start.x * scale + (end.x - start.x) / 3 * scale, pos.y + end.y * scale);
        ImVec2 bottom2_3rd = ImVec2(pos.x + start.x * scale + (end.x - start.x) / 3 * 2 * scale, pos.y + end.y * scale);

        draw_list->AddLine(top1_3rd, bottom1_3rd, color, 1.5f);
        draw_list->AddLine(top2_3rd, bottom2_3rd, color, 1.5f);
        draw_list->AddLine(left1_3rd, right1_3rd, color, 1.5f);
        draw_list->AddLine(left2_3rd, right2_3rd, color, 1.5f);

        ImVec2 resetPos = ImGui::GetCursorScreenPos();

        //corners

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0, 0, 0, 0));

        ImGui::SetCursorScreenPos(ImVec2(pos.x + start.x * scale - 20, pos.y + start.y * scale - 20));
        ImGui::Selectable("##cropDraggerTopLeft", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            start = start + vec::vec2({dt.x * scale, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + end.x * scale - 20, pos.y + start.y * scale - 20));
        ImGui::Selectable("##cropDraggerTopRight", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            end = end + vec::vec2({dt.x * scale, 0});
            start = start + vec::vec2({0, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + start.x * scale - 20, pos.y + end.y * scale - 20));
        ImGui::Selectable("##cropDraggerBottomLeft", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            start = start + vec::vec2({dt.x * scale, 0});
            end = end + vec::vec2({0, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNESW);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + end.x * scale - 20, pos.y + end.y * scale - 20));
        ImGui::Selectable("##cropDraggerBottomRight", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            end = end + vec::vec2({dt.x * scale, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        }

        //sides

        ImGui::SetCursorScreenPos(ImVec2(pos.x + start.x * scale - 20, pos.y + start.y * scale + (end.y - start.y) / 2 * scale - 20));
        ImGui::Selectable("##cropDraggerLeft", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            start = start + vec::vec2({dt.x * scale, 0});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + end.x * scale - 20, pos.y + start.y * scale + (end.y - start.y) / 2 * scale - 20));
        ImGui::Selectable("##cropDraggerRight", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            end = end + vec::vec2({dt.x * scale, 0});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + start.x * scale + (end.x - start.x) / 2 * scale - 20, pos.y + start.y * scale - 20));
        ImGui::Selectable("##cropDraggerTop", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            start = start + vec::vec2({0, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }

        ImGui::SetCursorScreenPos(ImVec2(pos.x + start.x * scale + (end.x - start.x) / 2 * scale - 20, pos.y + end.y * scale - 20));
        ImGui::Selectable("##cropDraggerBottom", false, ImGuiSelectableFlags_None, ImVec2(40, 40));
        if(ImGui::IsItemActive()) {
            ImVec2 dt = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left, 1.f);
            end = end + vec::vec2({0, dt.y * scale});

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
        }

        ImGui::PopStyleColor(3);

        ImGui::SetCursorScreenPos(resetPos);
        visible = false;
    }
}

void CropManager::setVisible(bool visible) {
    this->visible = visible;
}

void CropManager::setPos(vec::vec2 start, vec::vec2 end) {
    this->start = start;
    this->end = end;
}

void CropManager::crop() {
    if (start - end != vec::vec2({0, 0})) {

        vec::vec2 pos;

        if (start.x < end.x && start.y < end.y) {
            pos = start;
        } else if (start.x > end.x && start.y > end.y) {
            pos = end;
        } else if (start.x > end.x && start.y < end.y) {
            pos = vec::vec2({end.x, start.y});
        } else {
            pos = vec::vec2({start.x, end.y});
        }

        //projectManager->getActiveProject().project->canvas->resize(end - start, pos);
    }
}
