# Stage 4: Game Lifecycle

## Outcome

A directly launched game runs with the selected mode, and the original mode is restored after the game or any handled error.

## Plan

1. Start the selected executable through `QProcess` with the game directory as its working directory.
2. Validate configuration and apply the display mode before starting the process.
3. Restore the mode after launch failure, process completion, or a managed main-window close.
4. Prevent duplicate launches and make repeated restoration safe.
5. Show clear status and error messages.
6. Document that forced termination, power loss, and operating-system failure cannot trigger restoration.

## Validation

Check successful launch, a missing executable, immediate exit, launch failure, and closing the launcher while a game is running.
