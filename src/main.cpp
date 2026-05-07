#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "spp/decoder.hpp"
#include "spp/logger.hpp"
#include "spp/parser.hpp"

namespace {

struct CliOptions {
    std::string inputPath = "-";
    std::string outputPath = "parsed.csv";
};

void printUsage(const char* exeName) {
    std::cout << "Usage: " << exeName << " [--input <path|-] [--output <csv_path>]\n";
}

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
        } else if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return false;
        } else {
            return false;
        }
    }
    return true;
}

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

    spp::PacketParser parser;
    bool ok = false;

    if (options.inputPath == "-") {
        std::cin >> std::noskipws;
        ok = processStream(std::cin, parser, logger);
    } else {
        std::ifstream input(options.inputPath, std::ios::binary);
        if (!input.is_open()) {
            std::cerr << "Failed to open input file: " << options.inputPath << '\n';
            return 1;
        }

        ok = processStream(input, parser, logger);
    }

    printStats(parser.stats());

    return ok ? 0 : 1;
}
