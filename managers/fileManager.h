#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "imgui.h"

#include "commandManager.h"
#include "projectManager.h"
#include "../globals.h"

extern CommandManager* commandManager;
extern ProjectManager* projectManager;

class FileManager {
private:

public:
    FileManager();
    ~FileManager() = default;

    static void newFile();
    static void openFile();
    static void importFile();
    static void saveFile();
    static void saveFileAs();
    static void closeFile();
    static void exportFile();
};
