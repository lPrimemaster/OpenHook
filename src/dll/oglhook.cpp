#include <Windows.h>
#include <iostream>
#include "oglhook.h"

static twglSwapBuffers owglSwapBuffers = nullptr;
static OpenHook::UIRenderer* localRender = nullptr;

twglSwapBuffers* GetOriginalwglSwapBuffersPtr()
{
    return &owglSwapBuffers;
}

void SetUIRender(OpenHook::UIRenderer* render)
{
    localRender = render;
}

void FinishUIRender()
{
    if(localRender)
    {
        localRender->destroy();
        delete localRender;
    }
}

// Execution will get detoured to this
BOOL __stdcall hwglSwapBuffers(_In_ HDC hDc)
{
    if(localRender)
    {
        if(!localRender->isSetup())
        {
            localRender->setContext(hDc);
            localRender->setup();
            localRender->setSetupDone();
        }
        
        localRender->render();
    }

    return owglSwapBuffers(hDc);
}