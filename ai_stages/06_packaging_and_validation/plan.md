# Stage 6: Packaging and Validation

## Outcome

A portable release package and a clear pre-release validation checklist.

## Plan

1. Build the Release configuration and inspect its Qt and compiler-runtime dependencies.
2. Create a portable ZIP containing `resgoth.exe`, required Qt DLLs, and the Windows platform plugin; do not create an installer.
3. Include third-party notices and the relevant license text.
4. Test the ZIP on a Windows machine without a Qt development installation.
5. Manually test display-mode changes and restoration on supported display configurations.
6. Keep the README current with usage, recovery behavior, limitations, and known issues.
7. Build tagged releases in GitHub Actions, publish a ZIP with `SHA256SUMS.txt`, and generate a GitHub provenance attestation.

## Validation

The extracted ZIP starts without a local Qt installation, and the key mode-change and restoration scenarios have been manually tested.
