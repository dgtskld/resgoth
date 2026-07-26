# Stage 2: Application Shell

## Outcome

A small working window lets the user select a game executable and launch display mode; settings persist between launches.

## Plan

1. Separate UI, configuration, display-mode, Steam-discovery, and process-lifecycle code.
2. Build the form with Steam game discovery, executable selection, primary-display information, current mode, launch mode, status, and action buttons.
3. Read and write the portable INI configuration.
4. Load the primary display and its available modes.
5. Validate input and disable launch until the configuration is valid.

## Validation

The form works without launching a game, and its saved settings survive an application restart.
