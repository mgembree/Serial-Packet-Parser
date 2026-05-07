# 7-Day Build Plan

## Day 1 - Scaffold and CLI
- Set up CMake and source/include/tests folders.
- Add executable skeleton with CLI flags: --input and --output.
- Validate binary file read and stdin read paths.

## Day 2 - Protocol Definition
- Lock packet format: start/type/len/payload/checksum.
- Implement checksum function and packet enums.
- Document packet type payload layout.

## Day 3 - State Machine Parser
- Implement finite-state parser with resync behavior.
- Add max payload guard and malformed packet drop logic.
- Add parser events for valid, dropped, checksum-failed.

## Day 4 - Decode and Logging
- Decode temperature, voltage, and status packets.
- Write CSV logger with timestamp and decoded fields.
- Confirm logger handles unknown packet type safely.

## Day 5 - Metrics and Hardening
- Print end-of-run stats and failure counters.
- Handle noisy data, truncated frames, and invalid lengths.
- Improve CLI error messages and return codes.

## Day 6 - Tests
- Add checksum tests.
- Add parser boundary tests: valid, bad checksum, oversize len.
- Add simple CI-ready CTest entry.

## Day 7 - Demo and Polish
- Finalize README with build/run/demo sections.
- Generate sample input stream and expected CSV output.
- Capture one terminal demo snippet for portfolio/README.
