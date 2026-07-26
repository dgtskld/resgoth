# Stage 4: Technical Decisions

## Launch Lifecycle

- A game is launched only as a direct executable through `QProcess`; its directory is the working directory.
- Before launch, Resgoth captures the primary display mode and applies the selected launch mode.
- Resgoth attempts restoration after `FailedToStart`, process completion, and managed main-window close.
- Launch is disabled while the tracked process is active. After successful restoration, the saved mode is cleared, so another restoration attempt is safe.

## Limitation

Resgoth cannot restore the display mode if it is forcibly terminated, the computer loses power, or Windows fails. Display-mode operations are recorded in `resgoth.log` for diagnostics.
