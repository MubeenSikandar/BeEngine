cmake_minimum_required(VERSION 3.31)
project(GameEngine VERSION 0.1.0 LANGUAGES CXX)

# Use latest C++ standard (C++23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# BeEngine - Shared Library (DLL/dylib/so)
add_library(BeEngine SHARED
    BeEngine/test.cpp
)

target_include_directories(BeEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine
)

# Define BE_BUILD_DLL when building the library
target_compile_definitions(BeEngine PRIVATE BE_BUILD_DLL)

# Platform-specific settings
if(WIN32)
    # Windows 11 SDK (Build 10.0.26100)
    set(CMAKE_SYSTEM_VERSION "10.0.26100.0")
    set_target_properties(BeEngine PROPERTIES
        PREFIX ""
        OUTPUT_NAME "BeEngine"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
elseif(APPLE)
    # macOS 15 Sequoia support
    set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0")
    set_target_properties(BeEngine PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_RPATH TRUE
        MACOSX_RPATH TRUE
    )
elseif(UNIX)
    # Linux - GCC 14+ / Clang 18+
    set_target_properties(BeEngine PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )
endif()

# Sandbox - Executable
add_executable(Sandbox
    Sandbox/main.cpp
)

target_link_libraries(Sandbox PRIVATE BeEngine)

# Copy DLL to Sandbox directory on Windows
if(WIN32)
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BeEngine>
        $<TARGET_FILE_DIR:Sandbox>
    )
endif()
