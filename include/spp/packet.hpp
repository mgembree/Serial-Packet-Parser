#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace spp {

constexpr std::uint8_t kStartByte = 0xAA;
constexpr std::size_t kMaxPayloadLength = 64;

enum class PacketType : std::uint8_t {
    Temperature = 0x01,
    Voltage = 0x02,
    Status = 0x03,
    Unknown = 0xFF,
};

inline PacketType packetTypeFromByte(std::uint8_t value) {
    switch (value) {
    case static_cast<std::uint8_t>(PacketType::Temperature):
        return PacketType::Temperature;
    case static_cast<std::uint8_t>(PacketType::Voltage):
        return PacketType::Voltage;
    case static_cast<std::uint8_t>(PacketType::Status):
        return PacketType::Status;
    default:
        return PacketType::Unknown;
    }
}

inline const char* toString(PacketType type) {
    switch (type) {
    case PacketType::Temperature:
        return "temperature";
    case PacketType::Voltage:
        return "voltage";
    case PacketType::Status:
        return "status";
    default:
        return "unknown";
    }
}

struct Packet {
    PacketType type = PacketType::Unknown;
    std::vector<std::uint8_t> payload;
};

struct ParserStats {
    std::uint64_t total_packets = 0;
    std::uint64_t valid_packets = 0;
    std::uint64_t dropped_packets = 0;
    std::uint64_t checksum_failures = 0;
    std::uint64_t bytes_processed = 0;
};

struct DecodedPacket {
    PacketType type = PacketType::Unknown;
    std::optional<double> temperature_c;
    std::optional<double> voltage_v;
    std::optional<std::uint8_t> status_flags;
    std::string payload_hex;
};

}  // namespace spp
