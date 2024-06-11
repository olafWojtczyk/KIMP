#include "tool.h"
#include "../globals.h"

class MoveTool : public tool {
public:
    void select() override {
        g::activeTool = 0;
    }


    void mouseLeftEvent() override {};
    void mouseUpEvent() override;
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override {

    }

    void render() override;
private:
    bool isDragging = false;
    ImVec2 dt;
    ImVec2 off = ImVec2(0, 0);
};