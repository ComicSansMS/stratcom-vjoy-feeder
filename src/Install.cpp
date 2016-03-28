#include "Install.hpp"
#include "HidRepDesc.hpp"
#include "Log.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include <cstring>
#include <regex>

extern "C" {
    typedef void (WINAPI *refresh_vjoy_T)(void);
    typedef BOOL (WINAPI *is_vjoy_installed_T)(void);
}

namespace {
    char const* dll_locations[] ={
        "",
        "C:\\Program Files\\vJoy\\",
        nullptr
    };

    char const* VJOY_DEVICE_REGISTRY_BASE = "SYSTEM\\CurrentControlSet\\services\\vjoy\\Parameters";
    char const* HID_REP_DESC_VALUE = "HidReportDesctiptor";
    char const* HID_REP_DESC_SIZE_VALUE = "HidReportDesctiptorSize";
};

bool isRunningAsAdmin()
{
    // Allocate and initialize a SID of the administrators group.
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID sidAdministratorsGroup;
    if(!AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &sidAdministratorsGroup))
    {
        throw GetLastError();
    }
    std::unique_ptr<PSID, void(*)(PSID*)> guard_sidAdministratorsGroup(&sidAdministratorsGroup,
                                                                      [](PSID* p) { if(p) FreeSid(*p); });

    // Determine whether the SID of administrators group is enabled in 
    // the primary access token of the process.
    BOOL isRunningAsAdmin;
    if(!CheckTokenMembership(NULL, sidAdministratorsGroup, &isRunningAsAdmin))
    {
        throw GetLastError();
    }

    return (isRunningAsAdmin == TRUE);
}

void elevateProcess()
{
    char module_path[MAX_PATH];
    if(GetModuleFileName(NULL, module_path, sizeof(module_path) / sizeof(module_path[0])))
    {
        // Launch itself as admin
        SHELLEXECUTEINFO sei;
        std::memset(&sei, 0, sizeof(sei));
        sei.cbSize = sizeof(sei);
        sei.lpVerb = "runas";
        sei.lpFile = module_path;
        sei.lpParameters = "--install-vjoy";
        sei.hwnd = nullptr;
        sei.nShow = SW_NORMAL;
        if(!ShellExecuteEx(&sei))
        {
            DWORD dwError = GetLastError();
            if(dwError == ERROR_CANCELLED)
            {
                // The user refused to allow privileges elevation.
                LOG("User did not allow elevation");
            }
        } else
        {
            std::exit(1);  // Quit itself
        }
    }
}

void addVJoyDeviceToRegistry(int device_id, char const* descriptor, int descriptor_size)
{
    std::string const device_name = std::string("AADevice") +
                                    ((device_id < 10) ? "0" : "") + std::to_string(device_id);
    std::string device_key_path = std::string(VJOY_DEVICE_REGISTRY_BASE) + "\\" + device_name;
    HKEY device_key;
    DWORD disp;
    auto res = RegCreateKeyEx(HKEY_LOCAL_MACHINE, device_key_path.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS,
                                nullptr, &device_key, &disp);
    if(res == ERROR_SUCCESS) {
        std::unique_ptr<HKEY, void(*)(HKEY*)> guard_device_key(&device_key, [](HKEY* p) { RegCloseKey(*p); });
        if(disp == REG_CREATED_NEW_KEY) {
            DWORD const desc_size = descriptor_size;
            res = RegSetValueEx(device_key, HID_REP_DESC_VALUE, 0, REG_BINARY,
                                reinterpret_cast<BYTE const*>(descriptor),
                                desc_size);
            if(res != ERROR_SUCCESS) {
                LOG("Error writing device descriptor to registry");
            }
            res = RegSetValueEx(device_key, HID_REP_DESC_SIZE_VALUE, 0, REG_DWORD,
                                reinterpret_cast<BYTE const*>(&desc_size),
                                sizeof(DWORD));
            if(res != ERROR_SUCCESS) {
                LOG("Error writing device descriptor size to registry");
            }
        } else {
            LOG("Warning! Registry key overwrite detected - aborting");
        }
    } else {
        LOG("Error creating new registry key for device");
    }
    LOG("Created new VJoy device #" << device_id << ".");
}

void updateRegistry()
{
    HKEY vjoy_base;
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, VJOY_DEVICE_REGISTRY_BASE, 0,
                    KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &vjoy_base) != ERROR_SUCCESS)
    {
        LOG("Unable to open registry");
        return;
    }
    std::unique_ptr<HKEY, void(*)(HKEY*)> guard_vjoy_base(&vjoy_base, [](HKEY* p) { RegCloseKey(*p); });
    char key_name_buffer[255];
    std::regex rx_device_key("Device(\\d{2})");
    int largest_installed_device = 0;
    for(DWORD key_index = 0;; ++key_index) {
        DWORD key_name_buffer_size = sizeof(key_name_buffer) / sizeof(key_name_buffer[0]);
        auto res = RegEnumKeyEx(vjoy_base, key_index, key_name_buffer, &key_name_buffer_size,
                                nullptr, nullptr, nullptr, nullptr);
        if(res == ERROR_SUCCESS) {
            std::match_results<char const*> matches;
            if(std::regex_match(key_name_buffer, matches, rx_device_key)) {
                largest_installed_device = std::max(largest_installed_device, std::stoi(matches[1]));
            }
        } else if(res == ERROR_NO_MORE_ITEMS) {
            break;
        } else {
            LOG("Error while enumerating registry.");
            largest_installed_device = -1;
            break;
        }
    }
    if((largest_installed_device >= 0) && (largest_installed_device < 16))
    {
        addVJoyDeviceToRegistry(largest_installed_device,
                                HidRepDesc::hid_3Axis_32Buttons,
                                sizeof(HidRepDesc::hid_3Axis_32Buttons));
        if (largest_installed_device + 3 < 16)
        {
            addVJoyDeviceToRegistry(largest_installed_device + 1,
                                    HidRepDesc::hid_32Buttons,
                                    sizeof(HidRepDesc::hid_32Buttons));
            addVJoyDeviceToRegistry(largest_installed_device + 2,
                                    HidRepDesc::hid_32Buttons,
                                    sizeof(HidRepDesc::hid_32Buttons));
        }
    }
}

void locateVJoyInstallDllAndInstall()
{
    for(int i=0; dll_locations[i]; ++i)
    {
        std::string dll_path = std::string(dll_locations[i]) + "vJoyInstall.dll";
        HMODULE vjoy_inst = LoadLibrary(dll_path.c_str());
        if(vjoy_inst)
        {
            std::unique_ptr<HMODULE, void(*)(HMODULE*)> guard_vjoy_inst(&vjoy_inst, [](HMODULE* p) { FreeLibrary(*p); });
            auto refresh_vjoy = reinterpret_cast<refresh_vjoy_T>(GetProcAddress(vjoy_inst, "refresh_vjoy"));
            auto is_vjoy_installed =
                reinterpret_cast<is_vjoy_installed_T>(GetProcAddress(vjoy_inst, "is_vjoy_installed"));
            if(refresh_vjoy && is_vjoy_installed)
            {
                if(is_vjoy_installed())
                {
                    LOG("Successfully loaded vJoyInstall from " << dll_path.c_str());
                    updateRegistry();
                    LOG("Reloading VJoy driver.");
                    refresh_vjoy();
                    LOG("Installation complete.");
                    return;
                }
            }
        }
    }
    LOG("Unable to locate vJoyInstall");
}

void installVJoyDevices()
{
    if(!isRunningAsAdmin()) {
        elevateProcess();
    }

    locateVJoyInstallDllAndInstall();
}

