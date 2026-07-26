# Stage 3: Display Mode Handling

## Outcome

An isolated Win32 service safely changes and restores the Windows primary display mode.

## Plan

1. Read the primary display's current full `DEVMODE` with `EnumDisplaySettingsW`.
2. Enumerate candidate modes, remove duplicates, and retain only modes accepted by `CDS_TEST`.
3. Show common resolutions with refresh rate and aspect ratio.
4. Apply the selected mode with `ChangeDisplaySettingsExW` and report errors.
5. Keep the original full mode in memory for the current run.
6. Restore the mode independently of UI state and log Win32 operations.

## Validation

On a test display, a selected mode applies and restores exactly; unsupported modes are not offered.
