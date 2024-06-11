#include "tool.h"
#include "../globals.h"

class PipetteTool : public tool {
    void select() override {
        g::activeTool = 14;
    }
    void mouseLeftEvent()  override;
    void mouseRightEvent() override;

    void mouseUpEvent() override {

    }

    void mouseHoldEvent(ImGuiMouseButton_ Button) override;

    void render() override;
};