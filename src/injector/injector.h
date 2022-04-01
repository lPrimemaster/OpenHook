#include <Windows.h>
#include <iostream>

#ifdef _EXPORT_SHARED
#define OPENHOOK_API __declspec(dllexport)
#elif _EXPORT_STATIC
#define OPENHOOK_API
#else
#define OPENHOOK_API __declspec(dllimport)
#endif

namespace OpenHook
{
    class OPENHOOK_API Injector
    {
    public:
        Injector(const std::string& pname, const std::string& dllname);
        ~Injector() = default;

        const bool checkProcess() noexcept;
        const bool inject() noexcept;

    private:
        const DWORD getPID() noexcept;

        // TODO: Fix exporting this in shared libs
        std::string pname;
        std::string dllname;
        DWORD pid;
    };
}