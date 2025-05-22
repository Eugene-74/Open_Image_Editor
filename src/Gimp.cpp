
#include "Gimp.hpp"

#include <windows.h>

#include <string>
#include <vector>

namespace Gimp {

/**
 * @brief Check if GIMP is installed on the system.
 * @return true if GIMP is found, false otherwise.
 */
bool exist() {
    std::wstring gimpPath = findExecutable();
    return !gimpPath.empty();
}

/**
 * @brief Find the GIMP executable path in the Windows registry.
 * @return The path to the GIMP executable if found, otherwise an empty string.
 */
std::wstring findExecutable() {
    const std::vector<HKEY> roots = {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER};
    const std::wstring uninstallPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

    for (HKEY root : roots) {
        HKEY hUninstallKey = nullptr;

        if (RegOpenKeyExW(root, uninstallPath.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hUninstallKey) != ERROR_SUCCESS) {
            continue;
        }

        wchar_t subKeyName[256];
        DWORD subKeyNameSize = 256;
        DWORD index = 0;

        while (RegEnumKeyExW(hUninstallKey, index++, subKeyName, &subKeyNameSize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            subKeyNameSize = 256;
            std::wstring fullSubKeyPath = uninstallPath + L"\\" + subKeyName;

            HKEY hAppKey = nullptr;
            if (RegOpenKeyExW(root, fullSubKeyPath.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &hAppKey) == ERROR_SUCCESS) {
                wchar_t displayName[256];
                DWORD bufferSize = sizeof(displayName);
                DWORD type = 0;

                if (RegQueryValueExW(hAppKey, L"DisplayName", nullptr, &type, reinterpret_cast<LPBYTE>(displayName), &bufferSize) == ERROR_SUCCESS) {
                    if (type == REG_SZ && std::wstring(displayName).find(L"GIMP") != std::wstring::npos) {
                        // Trouvé, maintenant on lit "DisplayIcon"
                        wchar_t installPath[MAX_PATH];
                        DWORD pathSize = sizeof(installPath);
                        if (RegQueryValueExW(hAppKey, L"DisplayIcon", nullptr, &type, reinterpret_cast<LPBYTE>(installPath), &pathSize) == ERROR_SUCCESS) {
                            std::wstring pathStr = installPath;
                            size_t commaPos = pathStr.find(L",");
                            if (commaPos != std::wstring::npos) {
                                pathStr = pathStr.substr(0, commaPos);  // Supprime la virgule et le numéro d'icône
                            }
                            RegCloseKey(hAppKey);
                            RegCloseKey(hUninstallKey);
                            return pathStr;
                        }
                    }
                }
                RegCloseKey(hAppKey);
            }
        }
        RegCloseKey(hUninstallKey);
    }
    return L"";  // Not found
}
/**
 * @brief Launch GIMP with the specified image.
 * @param imagePath The path to the image to open in GIMP.
 * @return true if GIMP was launched successfully, false otherwise.
 */
bool launchWithImage(const std::wstring& imagePath) {
    std::wstring gimpPath = findExecutable();
    if (gimpPath.empty()) return false;

    std::wstring commandLine = L"\"" + gimpPath + L"\" \"" + imagePath + L"\"";

    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi;

    BOOL result = CreateProcessW(
        nullptr,
        &commandLine[0],  // modifiable string
        nullptr,
        nullptr,
        FALSE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi);

    if (result) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }

    return false;
}
}  // namespace Gimp