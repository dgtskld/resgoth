# Resgoth Development Plan

## Goal

Build a portable Windows launcher that starts a game executable at a selected primary-display mode and restores the original mode when the game ends or a handled error occurs.

## Technical Approach

Resgoth uses C++20 and Qt 6 Widgets for the UI, configuration, file selection, and process lifecycle. Win32 APIs (`EnumDisplaySettingsW` and `ChangeDisplaySettingsExW`) enumerate and change display modes.

The MVP changes the Windows primary display only. It does not choose a GPU, move a game window to another display, launch through Steam, or change the mode when focus changes. A game is always launched as a directly selected executable, allowing Resgoth to track it as a child process.

## Release Principles

- Support Windows 10 and Windows 11.
- Restore the display mode after normal completion, launch failure, and a managed launcher close.
- Store portable settings and diagnostics next to the executable.
- Ship a portable ZIP with the executable, required Qt DLLs, and the Windows platform plugin; no installer is required.

## Stages

1. [Technical decisions](01_technical_decisions/plan.md)
2. [Application shell](02_application_shell/plan.md)
3. [Display mode handling](03_display_mode/plan.md)
4. [Game lifecycle](04_game_lifecycle/plan.md)
5. [Steam discovery and UX](05_steam_and_ux/plan.md)
6. [Packaging and validation](06_packaging_and_validation/plan.md)

## MVP Completion Criteria

The user selects a game executable and an available primary-display launch mode, starts the game, and sees the original display mode restored for all handled outcomes.
