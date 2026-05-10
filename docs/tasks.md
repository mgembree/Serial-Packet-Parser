# Tasks

## Backlog

## In Progress
- [ ] README polish: CI badge and integration-test notes.

## Done
- [x] Add end-to-end CLI integration test that verifies CSV content with CTest fixtures.
- [x] Fix CTest integration verify path so `spp_cli_integration_verify` finds generated CSV.
- [x] Add `.gitignore` guard to prevent re-adding local CMake distribution folder.
- [x] Define base packet format with checksum rule.
- [x] Add sample binary stream generator script.
- [x] Add stream reader that accepts hex text input as optional mode.
- [x] Add configurable max payload from CLI flag.
- [x] Add initial documentation plan.
- [x] Add packet-type-specific decode validation (strict payload lengths).
- [x] Add CSV flush interval and buffered writing option.
- [x] Add GitHub Actions workflow for build + tests.
