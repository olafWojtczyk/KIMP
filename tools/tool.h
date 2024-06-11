#pragma once
#include <imgui.h>
#include <functional>


class tool {
public:
    virtual void select() = 0;

    virtual void render() {
        ImGui::Text("This is the base ToolClass!");
    };
    //fired as you press left
    virtual void mouseLeftEvent() = 0;
    //fired when you release left
    virtual void mouseUpEvent() = 0;
    //fired for the duration of you press left
    virtual void mouseHoldEvent(ImGuiMouseButton_ Button) = 0;
    //fired when you press right
    virtual void mouseRightEvent() = 0;
};