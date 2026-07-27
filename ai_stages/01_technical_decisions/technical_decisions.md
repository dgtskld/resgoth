# Stage 1: Technical Decisions

## MVP Scope

- Target platform: Windows 10 and Windows 11.
- Resgoth operates on the Windows primary display only. Selecting another display or forcing a game window onto it is out of scope.
- The application uses C++20 and Qt 6 Widgets.
- Win32 APIs enumerate and temporarily change display modes.

## Module Contracts

`DisplayModeService` is the only display-mode integration point. It reads the primary display's current mode, enumerates testable modes, applies a selected mode, and restores the saved full `DEVMODE`.

`GameLauncher` starts only the executable explicitly selected by the user. It reports launch errors and observes the child process until it exits.

## Configuration

Settings are stored in `resgoth.ini` beside `resgoth.exe`. The file records the selected executable, Steam App ID when available, and launch mode. Diagnostics are written to `resgoth.log` in the same directory.

## Distribution and Licensing

Releases are portable ZIP archives containing `resgoth.exe`, the required Qt DLLs, and the Qt Windows platform plugin. No installer or static Qt build is used. Resgoth source is MIT-licensed; Qt remains subject to its own licenses. Releases include the applicable notices and must keep the Qt DLLs replaceable by the user.
