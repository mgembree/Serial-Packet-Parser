#include "spp/logger.hpp"

#include <iomanip>
#include <sstream>

namespace spp {
CsvLogger::CsvLogger(const std::string& outputPath)
    : flushInterval_(0), rowsUntilFlush_(0), out_(outputPath, std::ios::out | std::ios::trunc) {
    if (out_.is_open()) {
        out_ << "timestamp,type,temperature_c,voltage_v,status_flags,payload_hex\n";
    }
}

CsvLogger::CsvLogger(const std::string& outputPath, std::size_t flushInterval)
    : flushInterval_(flushInterval), rowsUntilFlush_(flushInterval), out_(outputPath, std::ios::out | std::ios::trunc) {
    if (out_.is_open()) {
        out_ << "timestamp,type,temperature_c,voltage_v,status_flags,payload_hex\n";
    }
}

bool CsvLogger::isOpen() const {
    return out_.is_open();
}

void CsvLogger::flush() {
    if (!out_.is_open()) {
        return;
    }

    for (const std::string& row : buffer_) {
        out_ << row << '\n';
    }
    out_.flush();
    buffer_.clear();
    rowsUntilFlush_ = flushInterval_;
}

//Row format: timestamp,type,temperature_c,voltage_v,status_flags,payload_hex
void CsvLogger::writeRow(const std::string& isoTimestamp, const DecodedPacket& packet) {
    if (!out_.is_open()) {
        return;
    }

    std::ostringstream row;
    row << isoTimestamp << ',';
    row << toString(packet.type) << ',';

    if (packet.temperature_c.has_value()) {
        row << std::fixed << std::setprecision(2) << *packet.temperature_c;
    }
    row << ',';

    if (packet.voltage_v.has_value()) {
        row << std::fixed << std::setprecision(3) << *packet.voltage_v;
    }
    row << ',';

    if (packet.status_flags.has_value()) {
        row << static_cast<unsigned int>(*packet.status_flags);
    }
    row << ',';

    row << '"' << packet.payload_hex << '"';

    buffer_.push_back(row.str());
    if (flushInterval_ == 0 || --rowsUntilFlush_ == 0) {
        flush();
    }
}

}  // namespace spp
