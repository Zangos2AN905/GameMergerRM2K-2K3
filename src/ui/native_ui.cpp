#include "native_ui.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>
#include <codecvt>
#include <locale>

namespace UI {

    // Helper to convert std::string (UTF-8) to std::wstring (UTF-16)
    std::wstring to_wstring(const std::string& str) {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    // Helper to convert std::wstring (UTF-16) to std::string (UTF-8)
    std::string to_string(const std::wstring& wstr) {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    std::string select_folder(const std::string& title) {
        std::string path;
        IFileOpenDialog* pFileOpen;

        // Initialize COM library for this thread if not already
        // We use CoInitializeEx so we don't conflict if already init
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        // If RPC_E_CHANGED_MODE, it's already init with different mode, which is usually fine for just a dialog?
        // Actually FileOpenDialog usually requires STA.

        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

        if (SUCCEEDED(hr)) {
            DWORD dwOptions;
            if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions))) {
                pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            }
            
            if (!title.empty()) {
                pFileOpen->SetTitle(to_wstring(title).c_str());
            }

            // Show the Open dialog box.
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box.
            if (SUCCEEDED(hr)) {
                IShellItem* pItem;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr)) {
                    PWSTR pszFilePath;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    if (SUCCEEDED(hr)) {
                        path = to_string(pszFilePath);
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
            pFileOpen->Release();
        } else {
            std::cerr << "CoCreateInstance failed for FileDialog. HR: " << std::hex << hr << std::endl;
        }
        
        // We don't call CoUninitialize() here because we might want to use it again
        // usually it's better to init once in main.
        
        return path;
    }
}
#else
// Fallback for non-Windows (just in case)
namespace UI {
    std::string select_folder(const std::string& title) {
        return "";
    }
}
#endif
