#include "tool.h"
#include "../globals.h"

class ShapeTool : public tool {
public:
    ShapeTool(int index) {
        type = index;
    }
    void select() override {
        g::activeTool = 8 + type;
    }

    void mouseLeftEvent() override;
    void mouseUpEvent() override;
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override;
    void render() override;

    int type = 0;
};