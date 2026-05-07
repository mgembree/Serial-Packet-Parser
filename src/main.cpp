#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "spp/decoder.hpp"
#include "spp/hex_input.hpp"
#include "spp/logger.hpp"
#include "spp/parser.hpp"

// Example command line usage: 
//   # Parse binary input from a file and write to parsed.csv with payload max of 128 bytes
//   ./spp_parser --input input.bin --output parsed.csv --input-format binary --max-payload 128
//   (parser file) --input (file) --output (file) --input-format (binary or hex) --max-payload (number)

namespace {
enum class InputFormat {
    Binary,
    Hex,
};

struct CliOptions {
    std::string inputPath = "-";
    std::string outputPath = "parsed.csv";
    InputFormat inputFormat = InputFormat::Binary;
    std::size_t maxPayload = 64;
};

void printUsage(const char* exeName) {
    std::cout << "Usage: " << exeName
              << " [--input <path|->] [--output <csv_path>] [--input-format <binary|hex>] [--max-payload <number>]\n";
}
// Parse command lines
bool parseArgs(int argc, char* argv[], CliOptions& options) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--input") {
            if (i + 1 >= argc) {
                return false;
            }
            options.inputPath = argv[++i];
        } else if (arg == "--output") {
            if (i + 1 >= argc) {
                return false;
            }
            options.outputPath = argv[++i];
        } else if (arg == "--input-format") {
            if (i + 1 >= argc) {
                return false;
            }

            const std::string value = argv[++i];
            if (value == "binary") {
                options.inputFormat = InputFormat::Binary;
            } else if (value == "hex") {
                options.inputFormat = InputFormat::Hex;
            } else {
                return false;
            }
        } else if (arg == "--max-payload") {
            if (i + 1 >= argc) {
                return false;
            }
            try {
                options.maxPayload = std::stoul(argv[++i]);
            } catch (const std::exception&) {
                return false;
            }
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return false;
        } else {
            return false;
        }
    }
    return true;
}
// Get current Time
std::string nowIso8601() {
    const auto now = std::chrono::system_clock::now();
    const auto tt = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &tt);
#else
    gmtime_r(&tt, &tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

bool processStream(std::istream& in, spp::PacketParser& parser, spp::CsvLogger& logger) {
    char ch = 0;
    while (in.get(ch)) {
        const auto outcome = parser.consume(static_cast<std::uint8_t>(ch));
        if (outcome.event == spp::ParseEvent::ValidPacket && outcome.packet.has_value()) {
            const auto decoded = spp::decodePacket(*outcome.packet);
            logger.writeRow(nowIso8601(), decoded);
        }
    }

    return in.eof() || !in.bad();
}

bool processHexStream(std::istream& in, spp::PacketParser& parser, spp::CsvLogger& logger) {
    std::vector<std::uint8_t> bytes;
    if (!spp::parseHexStream(in, bytes)) {
        return false;
    }

    for (std::uint8_t byte : bytes) {
        const auto outcome = parser.consume(byte);
        if (outcome.event == spp::ParseEvent::ValidPacket && outcome.packet.has_value()) {
            const auto decoded = spp::decodePacket(*outcome.packet);
            logger.writeRow(nowIso8601(), decoded);
        }
    }

    return true;
}

void printStats(const spp::ParserStats& stats) {
    std::cout << "Parser stats\n";
    std::cout << "  bytes processed: " << stats.bytes_processed << '\n';
    std::cout << "  total packets:   " << stats.total_packets << '\n';
    std::cout << "  valid packets:   " << stats.valid_packets << '\n';
    std::cout << "  dropped packets: " << stats.dropped_packets << '\n';
    std::cout << "  checksum fails:  " << stats.checksum_failures << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    CliOptions options;
    if (!parseArgs(argc, argv, options)) {
        printUsage(argv[0]);
        return 1;
    }

    spp::CsvLogger logger(options.outputPath);
    if (!logger.isOpen()) {
        std::cerr << "Failed to open output CSV: " << options.outputPath << '\n';
        return 1;
    }

    spp::PacketParser parser(options.maxPayload);
    bool ok = false;

    if (options.inputPath == "-") {
        std::cin >> std::noskipws;
        ok = options.inputFormat == InputFormat::Hex
            ? processHexStream(std::cin, parser, logger)
            : processStream(std::cin, parser, logger);
    } else {
        std::ifstream input(
            options.inputPath,
            options.inputFormat == InputFormat::Hex ? std::ios::in : (std::ios::in | std::ios::binary));
        if (!input.is_open()) {
            std::cerr << "Failed to open input file: " << options.inputPath << '\n';
            return 1;
        }

        ok = options.inputFormat == InputFormat::Hex
            ? processHexStream(input, parser, logger)
            : processStream(input, parser, logger);
    }

    if (!ok && options.inputFormat == InputFormat::Hex) {
        std::cerr << "Failed to parse hex input stream. Use hexadecimal byte pairs separated by whitespace or newlines.\n";
    }

    printStats(parser.stats());

    return ok ? 0 : 1;
}
