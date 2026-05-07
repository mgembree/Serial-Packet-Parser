#pragma once

#include <fstream>
#include <string>

#include "spp/packet.hpp"

namespace spp {

class CsvLogger {
public:
    explicit CsvLogger(const std::string& outputPath);
    bool isOpen() const;
    void writeRow(const std::string& isoTimestamp, const DecodedPacket& packet);

private:
    std::ofstream out_;
};

}  // namespace spp
