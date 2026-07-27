# Resgoth

Resgoth is a Windows launcher that temporarily changes the primary display mode while a selected game executable is running, then restores the original mode when the process exits.

## Features

- Windows 10 and Windows 11 support.
- Direct launch of a selected game EXE with its directory as the working directory.
- Automatic selection of the primary Windows display.
- Safe display-mode validation before applying a mode and automatic restoration after managed exits.
- Detection of installed Steam games to open the correct installation folder in the EXE picker. Steam games still launch through the explicitly selected EXE.

## Use

1. Optionally select a Steam game; `Browse…` opens its installation folder.
2. Select the exact game EXE.
3. Choose a launch mode and select `Launch`.
4. Resgoth restores the original display mode when the game exits. `Restore now` is available after a mode has been applied.

Settings are stored in `resgoth.ini` and diagnostics in `resgoth.log`, both next to the executable.

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
