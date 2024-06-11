#include "../../managers/projectManager.h"

extern ProjectManager* projectManager;

namespace menus {
    void addLayer() {
        auto activeProject = projectManager->getActiveProject();

        activeProject.project->canvas->addLayer(Layer::Type::Pixel, nullptr, activeProject.project->getSize());
    }
}