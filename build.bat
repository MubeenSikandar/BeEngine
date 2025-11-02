@echo off
echo Building GameEngine with latest standards...
echo C++23 - Windows 11 SDK (10.0.26100)

if not exist build mkdir build
cd build
cmake ..
cmake --build . --config Release

echo.
echo Build complete!
echo Run: .\build\bin\Release\Sandbox.exe
