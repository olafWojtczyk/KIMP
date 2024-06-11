#include <imgui.h>

namespace menus {
    void gaussianBlurFX() {
        g::popupStack.push_back("Gaussian blur");
    }

    void kernelFX() {
        g::popupStack.push_back("Convolution matrix");
    }

    void noseFX() {
        g::popupStack.push_back("Noise");
    }
}