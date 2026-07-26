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

## Limitations

If Resgoth or Windows closes unexpectedly while a game is running, the display mode may not be restored automatically. In that case, choose your usual resolution in Windows display settings.

## Building

Development builds use Qt 6 and CMake:

```powershell
cmake -S . -B build/debug -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build/debug
```

Release builds are dynamically linked and deploy Qt beside the executable. Build them with the installed Qt kit:

```powershell
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.11.1\mingw_64"
cmake --build build/release --parallel
```

Keep the executable, Qt DLLs, and `plugins` directory together when copying or packaging the application.
