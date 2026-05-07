# Tasks

## Backlog
- [ ] Add packet-type-specific decode validation (strict payload lengths).
- [ ] Add CSV flush interval and buffered writing option.
- [ ] Add GitHub Actions workflow for build + tests.

## In Progress
- [x] Create CMake and C++17 scaffold.
- [x] Implement checksum and parser FSM.
- [x] Add decoder and CSV logger.
- [x] Add parser stats counters.
- [x] Add unit test skeleton with CTest.

## Done (Week 1 Start)
- [x] Define base packet format with checksum rule.
- [x] Add sample binary stream generator script.
- [x] Add stream reader that accepts hex text input as optional mode.
- [x] Add configurable max payload from CLI flag.
- [x] Add initial documentation plan.
