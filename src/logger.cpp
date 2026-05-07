#include "spp/logger.hpp"

#include <iomanip>

namespace spp {

CsvLogger::CsvLogger(const std::string& outputPath)
    : out_(outputPath, std::ios::out | std::ios::trunc) {
    if (out_.is_open()) {
        out_ << "timestamp,type,temperature_c,voltage_v,status_flags,payload_hex\n";
    }
}

bool CsvLogger::isOpen() const {
    return out_.is_open();
}

void CsvLogger::writeRow(const std::string& isoTimestamp, const DecodedPacket& packet) {
    if (!out_.is_open()) {
        return;
    }

    out_ << isoTimestamp << ',';
    out_ << toString(packet.type) << ',';

    if (packet.temperature_c.has_value()) {
        out_ << std::fixed << std::setprecision(2) << *packet.temperature_c;
    }
    out_ << ',';

    if (packet.voltage_v.has_value()) {
        out_ << std::fixed << std::setprecision(3) << *packet.voltage_v;
    }
    out_ << ',';

    if (packet.status_flags.has_value()) {
        out_ << static_cast<unsigned int>(*packet.status_flags);
    }
    out_ << ',';

    out_ << '"' << packet.payload_hex << '"' << '\n';
}

}  // namespace spp
