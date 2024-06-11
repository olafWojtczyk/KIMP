#include "fileManager.h"
#include "FreeImage.h"

FileManager::FileManager() {
    std::function<void()> fN = FileManager::newFile;
    commandManager->registerCommand("action.file.new", fN);

    std::function<void()> fO = FileManager::openFile;
    commandManager->registerCommand("action.file.open", fO);

    std::function<void()> fI = FileManager::importFile;
    commandManager->registerCommand("action.file.import", fI);

    std::function<void()> fE = FileManager::exportFile;
    commandManager->registerCommand("action.workspace.export", fE);

    std::function<void()> fS = FileManager::saveFile;
    commandManager->registerCommand("action.file.save", fS);

    std::function<void()> fSa = FileManager::saveFileAs;
    commandManager->registerCommand("action.file.saveAs", fSa);

    std::function<void()> fC = FileManager::closeFile;
    commandManager->registerCommand("action.file.close", fC);
}

void FileManager::newFile() {
    g::popupStack.push_back("New File");
}

void FileManager::openFile() {
    OPENFILENAMEW ofn;
    WCHAR* szFile;
    HWND hwnd = nullptr;

    szFile = new WCHAR[32767];
    ZeroMemory(szFile, sizeof(WCHAR) * 32767);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 32767;
    ofn.lpstrFilter = L"Krzak Project Document (*.kpd)\0*.kpd\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == true)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string narrowStr = converter.to_bytes(ofn.lpstrFile);

        std::ifstream is(narrowStr, std::ios::binary);

        if (is) {
            projectManager->loadProject(ofn.lpstrFile, is);
        }
    }

    delete[] szFile;
}

void FileManager::importFile() {
    OPENFILENAMEW ofn;
    WCHAR* szFile;
    HWND hwnd = nullptr;

    szFile = new WCHAR[32767];
    ZeroMemory(szFile, sizeof(WCHAR) * 32767);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 32767;
    ofn.lpstrFilter = L"freeimage compatible picture (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
     
    if (GetOpenFileNameW(&ofn) == true)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string narrowStr = converter.to_bytes(ofn.lpstrFile);

       projectManager->getActiveProject().project->canvas->addImageLayer(narrowStr.data());
    }

    delete[] szFile;
}

void FileManager::exportFile() {
    g::popupStack.push_back("Export");
}

void FileManager::saveFile() {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string narrowStr = converter.to_bytes(projectManager->getActiveProject().project->path);

    bool saved = narrowStr.c_str() != nullptr && strlen(narrowStr.c_str()) > 1;

    if(saved) {
        std::ofstream os(narrowStr.c_str(), std::ios::binary);
        if(os.is_open()) {
            projectManager->saveProject(projectManager->getActiveProject(), os);
            os.close();
        }

    } else {
        FileManager::saveFileAs();
    }
}

void FileManager::saveFileAs() {
    std::stringstream ss;
    projectManager->saveProject(projectManager->getActiveProject(), ss);

    OPENFILENAMEW ofn;
    WCHAR* szFile;
    HWND hwnd = nullptr;
    HANDLE hf;

    szFile = new WCHAR[32767];
    ZeroMemory(szFile, sizeof(WCHAR) * 32767);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 32767;
    ofn.lpstrFilter = L"Krzak Project Document (*.kpd)\0*.kpd\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"kpd";

    if (GetSaveFileNameW(&ofn) == true)
    {
        hf = CreateFileW(ofn.lpstrFile,
                         GENERIC_WRITE,
                         0,
                         (LPSECURITY_ATTRIBUTES) nullptr,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         (HANDLE) nullptr);

        DWORD dwBytesWritten;

        std::string str = ss.str();
        const char* data = str.c_str();

        bool success = WriteFile(hf, data, str.size(), &dwBytesWritten, nullptr);

        std::filesystem::path fsPath(ofn.lpstrFile);
        std::wstring wsFilename = fsPath.filename().wstring();

        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string narrowStr = converter.to_bytes(wsFilename);
        char* cFilename = new char[narrowStr.length() + 1];
        std::strcpy(cFilename, narrowStr.c_str());

        projectManager->getActiveProject().project->path = ofn.lpstrFile;
        projectManager->getActiveProject().project->setName(cFilename);

        if (!success) {
            MessageBoxA(nullptr, "Failed to save file", "Error", MB_OK);
        }

        CloseHandle(hf);
    }

    delete[] szFile;
}

void FileManager::closeFile() {

}
