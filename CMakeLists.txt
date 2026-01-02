cmake_minimum_required(VERSION 3.31)
project(GameEngine VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# ============================================
# C++ STANDARD
# ============================================
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================
# BUILD CONFIGURATION
# ============================================
option(BE_BUILD_SHARED "Build BeEngine as shared library (DLL)" OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Build type not specified, defaulting to Debug")
endif()

# Define build type macros
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BE_DEBUG)
    message(STATUS "Building in DEBUG mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(BE_RELEASE)
    message(STATUS "Building in RELEASE mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Distribution" OR CMAKE_BUILD_TYPE STREQUAL "Dist")
    add_compile_definitions(BE_DIST)
    add_compile_definitions(NDEBUG)
    message(STATUS "Building in DISTRIBUTION mode")
endif()

# Library type message
if(BE_BUILD_SHARED)
    message(STATUS "Library Type: SHARED (DLL)")
else()
    message(STATUS "Library Type: STATIC")
endif()


# ============================================
# OUTPUT DIRECTORIES
# ============================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Per-config output directories (Debug/Release have separate folders)
foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin/${CONFIG})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib/${CONFIG})
endforeach()

# ============================================
# DEPENDENCIES
# ============================================
add_subdirectory(BeEngine/vendor/spdlog)
add_subdirectory(BeEngine/vendor/GLFW)
add_subdirectory(BeEngine/vendor/Glad)
add_subdirectory(BeEngine/vendor/GLM)
add_subdirectory(BeEngine/vendor/imgui)


# ============================================
# BUILD OPTIONS
# ============================================
option(BE_BUILD_SHARED "Build BeEngine as shared library" OFF)

# ============================================
# BeEngine Library
# ============================================
set(BEENGINE_SOURCES
    BeEngine/src/Application.cpp
    BeEngine/src/Log.cpp
    BeEngine/src/Events/Event.cpp
    BeEngine/src/Events/EventQueue.cpp
    BeEngine/src/Window.cpp
    BeEngine/src/Layers/Layer.cpp
    BeEngine/src/Layers/LayerStack.cpp
    BeEngine/src/Time/Time.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLRendererAPI.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLContext.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLIndexBuffer.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLShader.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLVertexArray.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLVertexBuffer.cpp
    BeEngine/src/Renderer/OpenGL/OpenGLFrameBuffer.cpp
    BeEngine/src/Renderer/Renderer.cpp
    BeEngine/src/Renderer/GraphicsContext.cpp
    BeEngine/src/Renderer/IndexBuffer.cpp
    BeEngine/src/Renderer/Shader.cpp
    BeEngine/src/Renderer/VertexArray.cpp
    BeEngine/src/Renderer/VertexBuffer.cpp
    BeEngine/src/Renderer/FrameBuffer.cpp
    BeEngine/src/ImGui/ImGuiLayer.cpp
    BeEngine/src/Input/Input.cpp
)

if(BE_BUILD_SHARED)
    add_library(BeEngine SHARED ${BEENGINE_SOURCES})
    target_compile_definitions(BeEngine PRIVATE BE_BUILD_DLL BE_DYNAMIC_LINK)

    # Windows: Copy DLL to executable directory
    if(WIN32)
        add_custom_command(TARGET Sandbox POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:BeEngine>
                $<TARGET_FILE_DIR:Sandbox>
            COMMENT "Copying BeEngine.dll to Sandbox directory"
        )
    endif()
else()
    add_library(BeEngine STATIC ${BEENGINE_SOURCES})
    # No DLL copying needed for static library
endif()

target_include_directories(BeEngine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/include
        ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/GLFW/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/src
)

target_link_libraries(BeEngine
    PUBLIC
        spdlog::spdlog
        glfw
        glad
        glm
        imgui
)


# ============================================
# COMPILER WARNINGS
# ============================================
if(MSVC)
    target_compile_options(BeEngine PRIVATE
        /W4                    # Warning level 4
        # /WX                    # Treat warnings as errors (optional, comment out if too strict)
        /permissive-           # Standards conformance
        /MP                    # Multi-processor compilation
    )

    # Disable specific warnings if needed
    # target_compile_options(BeEngine PRIVATE /wd4100)  # Unreferenced formal parameter

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(BeEngine PRIVATE
        -Wall                  # Enable most warnings
        -Wextra                # Extra warnings
        -Wpedantic             # Pedantic warnings
        # -Werror              # Treat warnings as errors (optional)
        -Wno-unused-parameter  # Disable unused parameter warning
    )
endif()

# ============================================
# OPTIMIZATION FLAGS
# ============================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_compile_options(BeEngine PRIVATE /Od /Zi)  # No optimization, debug info
    else()
        target_compile_options(BeEngine PRIVATE -O0 -g)   # No optimization, debug info
    endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(BeEngine PRIVATE /O2 /Zi)  # Optimize for speed, keep debug info
    else()
        target_compile_options(BeEngine PRIVATE -O2 -g)   # Optimize for speed, keep debug info
    endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Distribution" OR CMAKE_BUILD_TYPE STREQUAL "Dist")
    if(MSVC)
        target_compile_options(BeEngine PRIVATE /O2 /GL)  # Optimize + Link-Time Optimization
        target_link_options(BeEngine PRIVATE /LTCG)       # Link-Time Code Generation
    else()
        target_compile_options(BeEngine PRIVATE -O3 -flto) # Maximum optimization + LTO
        target_link_options(BeEngine PRIVATE -flto)        # Link-Time Optimization
    endif()
endif()

# ============================================
# PLATFORM-SPECIFIC SETTINGS
# ============================================
if(WIN32)
    # Windows 11 SDK (Build 10.0.26100)
    set(CMAKE_SYSTEM_VERSION "10.0.26100.0")
    set_target_properties(BeEngine PROPERTIES
        PREFIX ""
        OUTPUT_NAME "BeEngine"
        WINDOWS_EXPORT_ALL_SYMBOLS ON
    )
     # Runtime library
    set_property(TARGET BeEngine PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
    )

elseif(APPLE)
    # macOS 15 Sequoia support
    set(CMAKE_OSX_DEPLOYMENT_TARGET "26.1")
    set_target_properties(BeEngine PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_RPATH TRUE
        MACOSX_RPATH TRUE
    )

     # Framework linking for macOS
    if(NOT IOS)
        target_link_libraries(BeEngine PRIVATE
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
        )
    endif()

elseif(UNIX AND NOT APPLE)
    # Linux - GCC 14+ / Clang 18+
    set_target_properties(BeEngine PROPERTIES
        POSITION_INDEPENDENT_CODE ON
    )

    # Threads for GLFW
    find_package(Threads REQUIRED)

    # X11 for windowing
    find_package(X11 REQUIRED)

    # Add -ldl for dynamic linking
    target_link_libraries(BeEngine PRIVATE Threads::Threads X11::X11 ${CMAKE_DL_LIBS})
endif()

# Sandbox - Executable
add_executable(Sandbox
    Sandbox/SandBoxApp.cpp
    Sandbox/SandboxLayer.cpp
)

target_link_libraries(Sandbox PRIVATE BeEngine)

# Sandbox also needs build configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Sandbox PRIVATE BE_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(Sandbox PRIVATE BE_RELEASE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Distribution" OR CMAKE_BUILD_TYPE STREQUAL "Dist")
    target_compile_definitions(Sandbox PRIVATE BE_DIST)
endif()

# ============================================
# CREATE LOGS DIRECTORY
# ============================================
add_custom_command(TARGET BeEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/logs
    COMMENT "Creating logs directory"
)

# ============================================
# INSTALLATION (OPTIONAL)
# ============================================
install(TARGETS BeEngine
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY BeEngine/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# ============================================
# PRINT CONFIGURATION SUMMARY
# ============================================
message(STATUS "")
message(STATUS "========================================")
message(STATUS "BeEngine Configuration Summary")
message(STATUS "========================================")
message(STATUS "Build Type:          ${CMAKE_BUILD_TYPE}")
if(BE_BUILD_SHARED)
    message(STATUS "Library Type:        SHARED (DLL)")
else()
    message(STATUS "Library Type:        STATIC")
endif()
message(STATUS "C++ Standard:        C++${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler:            ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform:            ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture:        ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Install Prefix:      ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
message(STATUS "")

# ============================================
# CUSTOM TARGETS
# ============================================

# Clean logs directory
add_custom_target(clean-logs
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin/logs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/logs
    COMMENT "Cleaning logs directory"
)

# Run Sandbox
add_custom_target(run
    COMMAND $<TARGET_FILE:Sandbox>
    DEPENDS Sandbox
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>
    COMMENT "Running Sandbox application"
)

# Full rebuild
add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Full rebuild"
)
