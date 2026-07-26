# Repository Guidelines

## Project Structure & Module Organization

- `main.cpp` is the current Qt application entry point and UI prototype.
- `CMakeLists.txt` defines the CMake target, Qt 6 dependencies, and Windows DLL deployment steps.
- `ai_stages/` contains the product plan. Read the relevant numbered stage before implementing its feature; it is the source of truth for MVP scope and sequencing.
- `cmake-build-debug/` and other generated build directories are local artifacts. Do not commit them.

As the application grows, keep platform-specific display-mode code in a focused Windows module and keep Qt UI, configuration, and process-lifecycle concerns separate. Add source files to the target explicitly in `CMakeLists.txt`.

## Build, Test, and Development Commands

Qt 6 (Core, Gui, and Widgets) and a C++20-capable compiler are required. Configure a separate build directory; provide the Qt installation prefix when CMake cannot find Qt:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build --config Debug
.\build\Debug\resgoth.exe
```

The Windows post-build rules copy the Qt DLLs and platform plugin next to the executable. Reconfigure after changing `CMakeLists.txt`.

## Coding Style & Naming Conventions

Use C++20 and Qt 6 APIs. Follow the existing four-space indentation, braces on the same line as declarations, and clear, compact functions. Use `PascalCase` for classes and Qt widgets, `camelCase` for functions and local variables, and descriptive file names such as `display_mode_manager.cpp`. Prefer RAII and explicit error handling for process and display-mode changes; restoration must be reliable on every managed exit path.

No formatter or linter is configured. Keep changes compatible with the project’s existing style and avoid unrelated reformatting.

## Testing Guidelines

There is currently no automated test framework. For each change, build the Debug configuration and manually verify the affected flow. Display-mode work must include safe restoration checks after normal exit, launch failure, and closing the launcher. When tests are introduced, place them under `tests/`, name files after the component (for example, `test_display_mode_manager.cpp`), and register them with CTest.

## Commit & Pull Request Guidelines

The history currently uses short, imperative summaries (for example, `added gitignore and etc`). Continue with concise subjects such as `Add display mode restoration`. Keep commits focused. Pull requests should explain the user-visible behavior, identify the applicable `ai_stages` plan, list validation performed, link relevant issues, and include screenshots for UI changes.
