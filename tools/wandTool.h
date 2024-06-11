#pragma once

#include <unordered_set>

#include "tool.h"
#include "../globals.h"

class WandTool : public tool {
public:
    void select() override {
        g::activeTool = 2;
    }

    void mouseLeftEvent() override;

    void mouseUpEvent() override {}
    void mouseHoldEvent(ImGuiMouseButton_ Button) override {};
    void mouseRightEvent() override {};

    void render() override;

private:
    float threshold = 0.3f;

    enum class mode {
        NEW,
        ADD,
        SUBTRACT,
        INTERSECT
    };

    mode currentMode = mode::NEW;

    vec::vec4 originalColor = vec::vec4({0, 0, 0, 0});

    static void currentModeStyle(mode currentMode, mode mode);

    void dfs(int x, int y, std::vector<std::vector<bool>> visited, vec::vec2 size, vec::vec2 offset, BYTE* data, int rowSize);
};