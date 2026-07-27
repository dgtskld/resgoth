# Stage 7: Technical Decisions

## Launch Strategy

Resgoth will open `steam://run/<appId>` for a selected discovered Steam game. This lets the Steam client establish the game context, including overlay, playtime, and Steamworks features when the game supports them. `Manual selection` continues to use the existing direct `QProcess` launch path.

The selected executable remains required for Steam games. Steam manifests provide an install directory but do not reliably identify the executable that represents the real game process. The user selects that executable explicitly.

## Process Matching

Before opening the Steam URI, Resgoth records all running process IDs whose normalized full image path matches the selected executable. It then polls a Windows process snapshot at a short interval. A matching executable with a process ID absent from the baseline becomes the tracked game process.

The monitor obtains full image paths with Windows process-query APIs, compares paths case-insensitively, and never attaches to a pre-existing matching process. It uses a bounded startup timeout. If no new matching process appears, Resgoth restores the saved display mode and reports the failure.

## Limitations

This approach tracks the executable selected by the user, not every process a game may spawn. If a launcher starts a different final game executable, the user must select the final executable. A future enhancement may offer parent-child process tracking, but it is not required for the first Steam launch implementation.

Opening a Steam URI only confirms that Windows accepted the URI handler; it does not prove that Steam started the game. The process monitor is the source of truth for lifecycle handling.

Resgoth and the game should run at the same Windows privilege level. An elevated game process can deny path-query access to a non-elevated launcher, preventing reliable matching.
