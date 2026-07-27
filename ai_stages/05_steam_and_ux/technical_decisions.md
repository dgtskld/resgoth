# Stage 5: Technical Decisions

## Steam as a Discovery Source

- Resgoth launches games only through a user-selected executable. There is no Steam App ID launch mode.
- Steam installation locations are read from the Windows registry and standard installation locations.
- Library locations are read from `steamapps/libraryfolders.vdf`.
- Each library's `steamapps/appmanifest_*.acf` files provide the game name, App ID, and install directory.

## User Experience

The `Steam game` list contains discovered installed games and precedes executable selection. Choosing a game does not launch it or guess an executable: game folders can contain multiple executables, launchers, and anti-cheat components. It only selects the initial folder for `Browse...`, where the user chooses the desired executable.

The selected Steam App ID is stored with the executable path and restores the matching game selection on the next launch when it is still discovered. `Reset` returns the Steam selection to `Manual selection` and clears the selected executable. `Apply now` changes the selected display mode without launching a game; `Restore now` returns to the mode captured before the first apply or launch. If Steam is absent, a library is unavailable, or metadata is invalid, manual executable selection remains available. Tooltips explain the relationship between Steam discovery and executable selection.
