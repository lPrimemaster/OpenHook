# OpenHook
OpenHook is a simple OpenGL/GDI hooking library for C++. It allows the user to render anything on the screen of another application with no patching required.

OpenHook taps into the execution of the running module instead of copying frame data to the RAM and using GDI's `BitBlt` function to render an overlay each frame, making it very fast.

## Rendering Modes
| Mode | Available |
|-|:-:|
| Overlay (UI) | :heavy_check_mark: |
| Before/After `glDraw*()` | :x: |

## Compilation
To use the library, first clone the repository to your project directory with the recurse flag to download the git submodules as well.
```shell
git clone --recurse-submodules git@github.com:lPrimemaster/OpenHook.git
```

If you are using CMake to build your project, you can simply now add the subdirectory to your `CMakeLists.txt` file and link the lib (static only for now, even though the option for dynamic is in CMakeLists.txt).
```cmake
# Add this before add_subdirectory if you wish to be able to use printf from your dll
# This opens a new console 
set(OPENHOOK_DEBUG ON CACHE INTERNAL "" FORCE)

add_subdirectory(OpenHook)
add_library(your-target ...)
target_link_libraries(your-target PRIVATE openhook ...)
```

If not, compile the project and use the libraries generated using CMake and MSVC (there is still no CMake install step so you will have to access the headers and link the lib files manually).

## Example Usage
```cpp
// Define this before any OpenHook headers
#define OPENHOOK_STATIC
#include "OpenHook/src/dll/uirender.h"

class MyUiRenderer: public OpenHook::UIRenderer
{
public:
    virtual void setup() override
    {
        // Init your UI stuff

        // You can get the win32 HDC here if needed
        // using the protected member variable `context`
        //
        // This is the same HDC GDI is using

        // Example for getting the win32 window handle
        HWND window = WindowFromDC(context);

        // Here you might want to change windows `CallWindowProc` 
        // via `SetWindowLongPtr` to handle events in your UI
    }
    virtual void render() override
    {
        // Do your rendering here
        glDraw*();
        OrUseAnotherLibToRender();
        ...

        // Rendering here is the same as natively doing something like
        // glDraw*(); // injected app opengl render
        // glDraw*(); // your render
        // SwapBuffers();
    }
    virtual void destroy() override
    {
        // Do your cleanup here

        // Here you might want to change windows `CallWindowProc` 
        // via `SetWindowLongPtr` back to its original state
    }
};

// And finnaly the dll entrypoint
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			DisableThreadLibraryCalls(hModule);
			
			// Set your class as the default used by OpenHook
			// The passed resourse is automatically released on detach/close
			OpenHook::UIRenderer::SetDefaultRenderer(new MyUiRenderer());

			// Run your thread on dll attach
			OpenHook::AttachPatcherThread(hModule);
			break;
		}
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
```

## Injecting your  dll
The provided package also builds an executable target to inject the dll from the command line in a simple fashion. Inside `bin/Release/` use
```shell
./CMDInjector.exe ProcessNameToInject.exe YourDll.dll
```

If you wish to build your own injector app or cli you can use the `OpenHook::Injector` exported class.

```cpp
class OPENHOOK_API Injector
{
public:
	Injector(const  std::string&  pname, const  std::string&  dllname);
	~Injector() = default;

	const bool checkProcess() noexcept;
	const bool inject() noexcept;

private:
	const DWORD getPID() noexcept;

	std::string pname;
	std::string dllname;
	DWORD pid;
};

// Example as in CMDInjector
int  main(int argc, char* argv[])
{
	// TODO: Error checking with argv!
	OpenHook::Injector injector(argv[1], argv[2]);
	injector.inject();
}
```

