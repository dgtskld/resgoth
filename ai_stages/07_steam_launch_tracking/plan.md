# Stage 7: Steam Launch and Process Tracking

## Outcome

When a discovered Steam game is selected, Resgoth launches it through Steam so that Steam can provide its normal game context. Resgoth still detects the actual game process and restores the display mode when that process exits.

## Plan

1. Add a launch-method selector with `Steam` as the default and `Manual EXE` as the direct-launch alternative.
2. For a selected Steam game, open `steam://run/<appId>` instead of starting the executable directly.
3. Require a user-selected executable for Steam games; it identifies the process to track and is not treated as a Steam launch command.
4. Snapshot matching processes before launch, then poll Windows processes for a newly started executable whose normalized full path matches the selected path.
5. Attach to the first matching new process and wait for it to exit before restoring the display mode.
6. Add a clear launch timeout and restore the display mode if the expected process does not appear.
7. Handle an already-running matching process explicitly rather than accidentally tracking it.
8. Show distinct status messages for starting Steam, waiting for the game process, tracking the game, timeout, and restoration.

## Validation

Test a Steam game with overlay and achievements, a direct manual executable, Steam not running, a wrong executable selection, an already-running game, a launcher that starts another executable, and a process that exits immediately.
