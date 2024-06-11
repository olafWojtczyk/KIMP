#pragma once

#include "../vec.h"

class CropManager {
public:
    CropManager();

    void renderCropBox();
    void setVisible(bool visible);

    void setPos(vec::vec2 start, vec::vec2 end);

    void crop();

private:
    vec::vec2 start;
    vec::vec2 end;

    bool visible = false;

};