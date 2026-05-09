#pragma once

#include <fstream>
#include <string>

#include "spp/packet.hpp"

namespace spp {

class CsvLogger {
public:
    explicit CsvLogger(const std::string& outputPath);
    CsvLogger(const std::string& outputPath, std::size_t flushInterval);
    bool isOpen() const;
    void writeRow(const std::string& isoTimestamp, const DecodedPacket& packet);
    void flush();
private:
    std::vector<std::string> buffer_;
    std::size_t flushInterval_;
    std::size_t rowsUntilFlush_;
    std::ofstream out_;
};

}  // namespace spp
