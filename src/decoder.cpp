#include "spp/decoder.hpp"

#include <iomanip>
#include <sstream>

namespace spp {

namespace {

std::string payloadToHex(const std::vector<std::uint8_t>& payload) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < payload.size(); ++i) {
        if (i > 0) {
            oss << ' ';
        }
        oss << std::setw(2) << static_cast<int>(payload[i]);
    }

    return oss.str();
}

}  // namespace

DecodedPacket decodePacket(const Packet& packet) {
    DecodedPacket decoded;
    decoded.type = packet.type;
    decoded.payload_hex = payloadToHex(packet.payload);

    switch (packet.type) {
    case PacketType::Temperature:
        if (packet.payload.size() >= 2) {
            const std::uint16_t raw =
                static_cast<std::uint16_t>(packet.payload[0]) |
                (static_cast<std::uint16_t>(packet.payload[1]) << 8);
            decoded.temperature_c = static_cast<double>(raw) / 100.0;
        }
        break;

    case PacketType::Voltage:
        if (packet.payload.size() >= 2) {
            const std::uint16_t raw =
                static_cast<std::uint16_t>(packet.payload[0]) |
                (static_cast<std::uint16_t>(packet.payload[1]) << 8);
            decoded.voltage_v = static_cast<double>(raw) / 1000.0;
        }
        break;

    case PacketType::Status:
        if (!packet.payload.empty()) {
            decoded.status_flags = packet.payload[0];
        }
        break;

    default:
        break;
    }

    return decoded;
}

}  // namespace spp
