cmake_minimum_required (VERSION 3.8)
project(OpenHook CXX)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DUSE_GDI32)
add_definitions(-D_EXPORT_SHARED)

add_subdirectory(funchook)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(CMDInjector
    src/injector/cmdinjector.cpp
    src/injector/injector.cpp
    src/injector/injector.h
)

add_library(injector
    src/injector/injector.cpp
    src/injector/injector.h
)

add_library(openhook SHARED
    src/dll/main.cpp
    src/dll/oglhook.cpp
    src/dll/oglhook.h
    src/dll/uirender.h
)

# target_link_libraries(openhook PRIVATE funchook-shared opengl32.lib)
target_link_libraries(openhook PRIVATE funchook-static opengl32.lib)

