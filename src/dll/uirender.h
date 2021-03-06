#include <Windows.h>

// TODO: Put exported stuff onto a include dir instead for easy user access
#ifdef _EXPORT_SHARED
#define OPENHOOK_API __declspec(dllexport)
#else
#ifdef OPENHOOK_STATIC
#define OPENHOOK_API
#else
#define OPENHOOK_API __declspec(dllimport)
#endif
#endif

namespace OpenHook
{
    void OPENHOOK_API AttachPatcherThread(HMODULE hModule) noexcept;
}

namespace OpenHook
{
    class OPENHOOK_API UIRenderer
    {
    public:
        virtual ~UIRenderer() {  };
        
        virtual void setup() = 0;
        virtual void render() = 0;
        virtual void destroy() = 0;

        inline const bool isSetup() const
        {
            return setupDone;
        }

        inline void setContext(HDC hdc)
        {
            context = hdc;
        }

        inline void setSetupDone()
        {
            setupDone = true;
        }

        inline static void SetDefaultRenderer(UIRenderer* r)
        {
            defRenderer = r;
        }

        inline static UIRenderer* GetDefaultRenderer()
        {
            return defRenderer;
        }

    protected:
        bool setupDone = false;
        HDC context = nullptr;

    private:
        inline static UIRenderer* defRenderer = nullptr;
    };
}
