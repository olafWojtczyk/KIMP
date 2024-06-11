#include "tool.h"
#include "../globals.h"

class BrushTool : public tool {
public:
    BrushTool(int index) {
        type = index;
    }
    void select() override {
        g::activeTool = 4 + type;
    }

    void mouseLeftEvent() override;
    void mouseUpEvent() override;
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override;
    void render() override;
    int size = 20;
    float opacity = 0.f;

    int type = 0;
};