# Serial Packet Parser

<!-- TODO(next): add GitHub Actions CI badge below once workflow name/status URL is finalized. -->

C++17 serial packet parser and CSV logger for UART-style telemetry streams.

## Project Goals
- Read a byte stream from file or stdin.
- Support binary input and optional hex-text input for demos and fixtures.
- Allow the max payload limit to be configured from the CLI.
- Detect packet boundaries using start byte, length, payload, checksum.
- Reject malformed packets and checksum failures.
- Decode temperature, voltage, and status packets.
- Log decoded rows to CSV with UTC timestamps.
- Print parser health counters after each run.

## Packet Format
- Byte 0: start byte `0xAA`
- Byte 1: packet type
- Byte 2: payload length `len`
- Bytes 3..(3+len-1): payload bytes
- Final byte: checksum = sum of all previous frame bytes mod 256

## Packet Types
- `0x01` temperature: 2-byte little-endian unsigned value, scale `raw / 100.0` in C
- `0x02` voltage: 2-byte little-endian unsigned value, scale `raw / 1000.0` in V
- `0x03` status: 1-byte bit flags

## Repository Structure
```
.
|-- CMakeLists.txt
|-- README.md
|-- .gitignore
|-- data/
|   |-- sample_stream.bin
|   `-- sample_stream.hex
|-- include/
|   `-- spp/
|       |-- checksum.hpp
|       |-- decoder.hpp
|       |-- hex_input.hpp
|       |-- logger.hpp
|       |-- packet.hpp
|       `-- parser.hpp
|-- src/
|   |-- checksum.cpp
|   |-- decoder.cpp
|   |-- hex_input.cpp
|   |-- logger.cpp
|   |-- main.cpp
|   `-- parser.cpp
|-- tests/
|   `-- test_parser.cpp
|-- scripts/
|   `-- generate_sample.ps1
`-- docs/
		|-- tasks.md
		`-- week_plan.md
```

## Build
```powershell
cmake -S . -B build
cmake --build build
```

## Run
Generate sample binary and hex sample data:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\generate_sample.ps1
```

Parse from file:
```powershell
.\build\serial_packet_parser.exe --input .\data\sample_stream.bin --output .\output.csv
```

Parse from file with a custom payload cap:
```powershell
.\build\serial_packet_parser.exe --input .\data\sample_stream.bin --output .\output.csv --max-payload 128
```

Parse from stdin (example):
```powershell
Get-Content .\data\sample_stream.bin -AsByteStream | .\build\serial_packet_parser.exe --input - --output .\output.csv
```

Parse hex text from file:
```powershell
.\build\serial_packet_parser.exe --input .\data\sample_stream.hex --input-format hex --output .\output.csv
```

Parse hex text from stdin:
```powershell
Get-Content .\data\sample_stream.hex | .\build\serial_packet_parser.exe --input - --input-format hex --output .\output.csv
```

## Test
<!-- TODO(next): add a short integration-test note describing spp_cli_integration_run + spp_cli_integration_verify and expected CSV assertions. -->
```powershell
ctest --test-dir build --output-on-failure
```

## Parser Stats Example
```
Parser stats
	bytes processed: 27
	total packets:   4
	valid packets:   3
	dropped packets: 0
	checksum fails:  1
```

## 7-Day Plan
- Day 1: CMake + CLI skeleton
- Day 2: Packet format + checksum
- Day 3: State-machine parser
- Day 4: Decoder + CSV logger
- Day 5: Stats + robustness
- Day 6: Tests and boundaries
- Day 7: README polish + demo snippet

See `docs/week_plan.md` and `docs/tasks.md` for detailed weekly execution.

## Resume Bullets
- Built a C++17 packet parser using a finite-state machine to decode UART-style telemetry frames with checksum validation.
- Implemented configurable payload limits and robust handling for malformed and truncated packets, with runtime counters for drop and failure diagnostics.
- Developed a CSV logging pipeline for decoded telemetry and produced reproducible binary and hex CLI test inputs for protocol verification.
- Added unit tests for checksum and parser edge cases, improving reliability and regression confidence.
