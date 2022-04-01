#include "injector.h"

int main(int argc, char* argv[])
{
    // TODO: Error checking with argv!
    OpenHook::Injector injector(argv[1], argv[2]);
    injector.inject();
    return 0;
}