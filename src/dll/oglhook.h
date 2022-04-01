#include "uirender.h"

typedef BOOL(__stdcall * twglSwapBuffers)(_In_ HDC hDc);

twglSwapBuffers* GetOriginalwglSwapBuffersPtr();
BOOL __stdcall hwglSwapBuffers(_In_ HDC hDc);
void SetUIRender(OpenHook::UIRenderer* render);
void FinishUIRender();


// DWORD WINAPI patcherThread(HMODULE instance);
