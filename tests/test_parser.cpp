#include <cassert>
#include <cstdint>
#include <vector>

#include "spp/checksum.hpp"
#include "spp/parser.hpp"

namespace {

std::vector<std::uint8_t> makePacket(std::uint8_t type, const std::vector<std::uint8_t>& payload) {
    std::vector<std::uint8_t> bytes;
    bytes.push_back(spp::kStartByte);
    bytes.push_back(type);
    bytes.push_back(static_cast<std::uint8_t>(payload.size()));
    bytes.insert(bytes.end(), payload.begin(), payload.end());
    bytes.push_back(spp::computeChecksum(bytes));
    return bytes;
}

void testChecksum() {
    const std::vector<std::uint8_t> bytes{0xAA, 0x01, 0x02, 0x34, 0x12};
    const std::uint8_t sum = spp::computeChecksum(bytes);
    assert(sum == static_cast<std::uint8_t>((0xAA + 0x01 + 0x02 + 0x34 + 0x12) & 0xFF));
}

void testValidPacket() {
    spp::PacketParser parser;
    const auto bytes = makePacket(0x01, {0x34, 0x12});

    bool sawValid = false;
    for (std::uint8_t b : bytes) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::ValidPacket) {
            sawValid = true;
            assert(outcome.packet.has_value());
            assert(outcome.packet->payload.size() == 2);
        }
    }

    assert(sawValid);
    assert(parser.stats().valid_packets == 1);
    assert(parser.stats().checksum_failures == 0);
}

void testChecksumFailure() {
    spp::PacketParser parser;
    auto bytes = makePacket(0x02, {0x88, 0x13});
    bytes.back() ^= 0xFF;

    bool sawChecksumFailure = false;
    for (std::uint8_t b : bytes) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::ChecksumFailure) {
            sawChecksumFailure = true;
        }
    }

    assert(sawChecksumFailure);
    assert(parser.stats().checksum_failures == 1);
    assert(parser.stats().valid_packets == 0);
}

void testOversizedDrop() {
    spp::PacketParser parser;
    std::vector<std::uint8_t> bytes{spp::kStartByte, 0x03, static_cast<std::uint8_t>(spp::kMaxPayloadLength + 1)};

    bool sawDrop = false;
    for (std::uint8_t b : bytes) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::Dropped) {
            sawDrop = true;
        }
    }

    assert(sawDrop);
    assert(parser.stats().dropped_packets == 1);
}

}  // namespace

int main() {
    testChecksum();
    testValidPacket();
    testChecksumFailure();
    testOversizedDrop();
    return 0;
}
