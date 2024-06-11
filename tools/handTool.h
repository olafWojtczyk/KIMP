#include "tool.h"
#include "../globals.h"

class HandTool : public tool {
    void select() override {
        g::activeTool = 15;
    }


    void mouseLeftEvent() override {

    }
    void mouseUpEvent() override {

    }
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override {

    }

    void render() override;
};
