#include "injector.h"
#include <TlHelp32.h>

OpenHook::Injector::Injector(const std::string& pname, const std::string& dllname) : pname(pname)
{
    char fullpath[512];
    if(GetFullPathName(dllname.c_str(), 512, fullpath, NULL))
    {
        this->dllname = std::string(fullpath);
        std::cerr << "Full dll path: " << this->dllname << std::endl;
    }
    else
        std::cerr << "Could not find full path for dll: " << dllname << std::endl;
}

const bool OpenHook::Injector::checkProcess() noexcept
{
    DWORD targetProcID = getPID();
    if (!targetProcID)
    {
        return false;
    }

    pid = targetProcID;
    return true;
}

const bool OpenHook::Injector::inject() noexcept
{
    std::cerr << "Loading dll into target process..." << std::endl;

    if(!checkProcess())
    {
        std::cerr << "Failed to find process: " << pname << std::endl;
        return false;
    }

    // Get a static address of the LoadLibrary function as a thread-start-routine function.
    LPTHREAD_START_ROUTINE funcLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");
    if (!funcLoadLibrary) 
    {
        std::cerr << "Failed to retrieve a static function pointer to `LoadLibraryA`" << std::endl;
        return false;
    }

    // Open the target process.
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) 
    {
        std::cerr << "Failed to open target process." << std::endl;
        return false;
    }

    // Virtually allocate memory for the path of the dll in the target process.
    LPVOID pDllPathAddr = VirtualAllocEx(hProcess, 0, sizeof(dllname.c_str()) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pDllPathAddr) 
    {
        std::cerr << "Failed to allocate memory in the target process." << std::endl;
        return false;
    }

    // Write the dll path to the target process using WPM.
    WriteProcessMemory(hProcess, pDllPathAddr, (LPVOID)dllname.c_str(), strlen(dllname.c_str()) + 1, NULL);

    // Create a remote thread in the target process with LoadLibrary to load our dll into the target process.
    HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0, funcLoadLibrary, pDllPathAddr, NULL, NULL);
    if (!hRemoteThread || hRemoteThread == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Failed to load dll into target process." << std::endl;
        return false;
    }

    // Wait until the remote thread is done loading the dll.
    WaitForSingleObject(hRemoteThread, INFINITE);

    CloseHandle(hRemoteThread);
    CloseHandle(hProcess);

    std::cerr << "Done!" << std::endl;

    return true;
}

const DWORD OpenHook::Injector::getPID() noexcept
{
    DWORD localpid = 0;

    // Create snapshot
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // Check if the snapshot is valid, otherwise bail out
    if (hSnap == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 procEntry = {0};
    procEntry.dwSize = sizeof(PROCESSENTRY32);

    // Iterate over all processes in the snapshot
    if (Process32First(hSnap, &procEntry)) 
    {
        do {
            // Check if current process name is the same as the passed in process name
            if (strcmp(procEntry.szExeFile, pname.c_str()) == 0)
            {
                localpid = procEntry.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnap, &procEntry));
    }

    // Cleanup
    CloseHandle(hSnap);

    std::cerr << "Got process id: " << localpid << std::endl;

    return localpid;
}