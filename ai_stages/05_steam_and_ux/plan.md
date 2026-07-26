# Stage 5: Steam Discovery and UX

## Outcome

The launcher can discover installed Steam games to help the user find a game folder, while preserving reliable direct-executable launch tracking.

## Plan

1. Discover Steam installation locations and configured libraries.
2. Read installed-game metadata from Steam app manifests.
3. Present discovered games before executable selection.
4. Use the selected game's install directory as the starting directory for `Browse...`.
5. Keep executable selection explicit; do not infer a launch executable or launch through Steam.
6. Add concise tooltips, a `Restore now` action, current-mode details, and clear status messages.

## Validation

Selecting a Steam game changes the Browse starting folder only. Direct executable launch and display-mode restoration continue to work without Steam.
