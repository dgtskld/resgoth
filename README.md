# Resgoth

Resgoth is a Windows launcher that temporarily changes the primary display mode while a selected game executable is running, then restores the original mode when the process exits.

## Features

- Windows 10 and Windows 11 support.
- Steam launch for discovered games, with direct EXE launch available as a manual alternative.
- Automatic selection of the primary Windows display.
- Safe display-mode validation before applying a mode and automatic restoration after managed exits.
- Detection of installed Steam games and tracking of the selected game process after Steam starts it.

## Use

1. Choose `Steam` or `Manual EXE` as the launch method. Steam is selected by default.
2. For Steam, choose an installed game and then select its final game EXE. Resgoth does not start this EXE directly; it uses it to identify the process started by Steam.
3. For Manual EXE, choose the executable to start directly.
4. Choose a launch mode and select `Launch`.
5. Resgoth restores the original display mode when the game exits. `Restore now` is available after a mode has been applied.

Settings are stored in `resgoth.ini` and diagnostics in `resgoth.log`, both next to the executable.

## How It Works

Resgoth saves the current primary-display mode, switches Windows to the selected launch mode, starts the game, and waits for the game process to finish. When you exit the game, Resgoth restores the original display mode.

Expect a short pause while Windows applies the launch mode before the game appears, and another short pause after the game exits while the original mode is restored.

For Steam launches, Resgoth asks Steam to start the selected game and waits for the final game EXE chosen by you to appear. If a launcher starts another executable, choose that final executable rather than the launcher. This lets Steam provide its usual game context while Resgoth still knows when to restore the display mode.

Some games display a resolution from their own configuration or internal renderer rather than the active Windows display mode. Treat that value as informational: Resgoth applies the selected launch mode before starting the game. The selected mode remains active unless the game changes it itself.

## Limitations

If Resgoth or Windows closes unexpectedly while a game is running, the display mode may not be restored automatically. In that case, choose your usual resolution in Windows display settings.

## Building

Requirements:

- CMake 4.3 or later
- Ninja
- Qt 6.11.1 for MinGW 64-bit
- A matching MinGW-w64 compiler available in `PATH`

Set `QtPrefix` to the root directory of the Qt kit. The compiler and Qt kit must use the same MinGW architecture.

### Debug

```powershell
$QtPrefix = 'C:\Qt\6.11.1\mingw_64'
cmake -S . -B build\debug -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$QtPrefix"
cmake --build build\debug --parallel
```

### Release

```powershell
$QtPrefix = 'C:\Qt\6.11.1\mingw_64'
cmake -S . -B build\release -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QtPrefix"
cmake --build build\release --parallel
```

The Release output is dynamically linked. Keep `resgoth.exe`, the Qt DLLs, and the `plugins` directory together when copying or packaging the application.
