#include <chrono>

#include "tool.h"
#include "../globals.h"

class SelectTool : public tool {
    void select() override {
        g::activeTool = 1;
    }

    void mouseLeftEvent() override;

    void mouseUpEvent() override;
    void mouseHoldEvent(ImGuiMouseButton_ Button) override;
    void mouseRightEvent() override {};

    void render() override;

private:
    vec::vec2 start;
    vec::vec2 end;

    std::chrono::steady_clock::time_point pressTime;
    std::chrono::steady_clock::time_point releaseTime;

    enum class mode {
        NEW,
        ADD,
        SUBTRACT,
        INTERSECT
    };

    mode currentMode = mode::NEW;

    static void currentModeStyle(mode currentMode, mode mode);
};
