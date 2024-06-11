#include "tool.h"
#include "../globals.h"

class CropTool : public tool {
public:
    void select() override {
        g::activeTool = 3;
    }

    void mouseLeftEvent() override;
    void mouseUpEvent() override;
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override;
    void render() override;

    int type = 0;
};