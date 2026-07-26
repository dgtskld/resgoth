# Stage 1: Technical Decisions

## Outcome

The MVP scope, module boundaries, settings format, and release approach are defined before implementation.

## Plan

1. Target Windows 10 and Windows 11, using the primary display only.
2. Use Qt 6 Widgets for the application and Win32 APIs for display modes.
3. Define `DisplayModeService` to enumerate, validate, apply, and restore modes.
4. Define `GameLauncher` to start and observe a direct executable.
5. Store the game path and selected mode in a portable INI file beside the executable.
6. Use a dynamically linked, portable Qt release directory instead of static linking.

## Validation

The decisions and limitations are documented, and a development build is reproducible with Qt and CMake.
