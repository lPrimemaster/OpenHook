#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>

#include "../../funchook/include/funchook.h"
#include "oglhook.h"

static DWORD WINAPI patcherThread(HMODULE instance)
{
    // Load console for debug
    #ifdef OPNEHOOK_DEBUG
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    #endif

    bool initFailed = false;
    DWORD approxSleep = 0;
    
    #ifdef USE_GDI32
    printf("[*] OpenHook: Looking for gdi32.dll!\n");
    while(GetModuleHandle("gdi32.dll") == 0 && approxSleep < 5000)
    {
        Sleep(100);
        approxSleep += 100;
    }

    if(GetModuleHandle("gdi32.dll") == 0 && approxSleep >= 5000)
    {
        printf("[*] OpenHook: Could not find gdi32.dll!\n");
        initFailed = true;
    }
    #else
    printf("[*] OpenHook: Looking for opengl32.dll!\n");
    while(GetModuleHandle("opengl32.dll") == 0 && approxSleep < 5000)
    {
        Sleep(100);
        approxSleep += 100;
    }

    if(GetModuleHandle("opengl32.dll") == 0 && approxSleep >= 5000)
    {
        printf("[*] OpenHook: Could not find opengl32.dll!\n");
        initFailed = true;
    }
    #endif

    funchook_t* fhook = nullptr;
    #ifdef USE_GDI32
    HMODULE hMod = GetModuleHandle("gdi32.dll");
    #else
    HMODULE hMod = GetModuleHandle("opengl32.dll");
    #endif
    if (hMod)
	{
        #ifdef USE_GDI32
        printf("[*] OpenHook: Found gdi32.dll!\n");
        #else
        printf("[*] OpenHook: Found opengl32.dll!\n");
        #endif

        // Set the renderer
        if(OpenHook::UIRenderer::GetDefaultRenderer() == nullptr)
        {
            printf("[*] OpenHook: GetDefaultRenderer failed.\n");
            printf("[*] OpenHook: Maybe you forgot to set the default renderer via SetDefaultRenderer?\n");
            initFailed = true;
        }
        else
        {
            SetUIRender(OpenHook::UIRenderer::GetDefaultRenderer());

            // Install hook
            fhook = funchook_create();
            int rv;

            twglSwapBuffers* owglptr = GetOriginalwglSwapBuffersPtr();

            #ifdef USE_GDI32
            *owglptr = (twglSwapBuffers)GetProcAddress(hMod, "SwapBuffers");
            #else
            *owglptr = (twglSwapBuffers)GetProcAddress(hMod, "wglSwapBuffers");
            #endif

            rv = funchook_prepare(fhook, (void**)owglptr, hwglSwapBuffers);
            if(rv != 0)
            {
                printf("[*] OpenHook: funchook_prepare failed with error: %d\n", rv);
                initFailed = true;
            }

            rv = funchook_install(fhook, 0);
            if(rv != 0)
            {
                printf("[*] OpenHook: funchook_install failed with error: %d\n", rv);
                initFailed = true;
            }
        }
    }

    if(!initFailed)
    {
        printf("[*] OpenHook: Initialization successful!\n");
        while(!GetAsyncKeyState(VK_END))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    else
    {
        printf("[*] OpenHook: Initialization error!\n");
    }


    FinishUIRender();

    if(fhook)
    {
        funchook_uninstall(fhook, 0);
        funchook_destroy(fhook);
    }

    printf("[*] OpenHook: Shutting down!\n");

    #ifdef OPNEHOOK_DEBUG
    fclose(f);
    FreeConsole();
    #endif

    FreeLibraryAndExitThread(instance, 0);
    return 0;
}

void OpenHook::AttachPatcherThread(HMODULE hModule) noexcept
{
    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)patcherThread, hModule, 0, 0);
    if (hThread != NULL)
    {
        CloseHandle(hThread);
    }
}

// Example
// BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
// {
//     switch (ul_reason_for_call) 
//     {
//         case DLL_PROCESS_ATTACH:
//         {
//             DisableThreadLibraryCalls(hModule);
//             OpenHook::AttachPatcherThread(hModule);
//             break;
//         }
//         case DLL_THREAD_ATTACH:
//         case DLL_THREAD_DETACH:
//         case DLL_PROCESS_DETACH:
//             // TODO: Free resources on a detach
//             break;
//     }

//     return TRUE;
// }