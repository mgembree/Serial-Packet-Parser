#include <cassert>
#include <cstdint>
#include <sstream>
#include <vector>

#include "spp/checksum.hpp"
#include "spp/decoder.hpp"   // TODO: needed for validatePacket and decodePacket tests
#include "spp/hex_input.hpp"
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

void testCustomMaxPayloadDrop() {
    spp::PacketParser parser(1);
    const auto bytes = makePacket(0x01, {0x34, 0x12});

    bool sawDrop = false;
    for (std::uint8_t b : bytes) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::Dropped) {
            sawDrop = true;
        }
    }

    assert(sawDrop);
    assert(parser.stats().dropped_packets == 1);
    assert(parser.stats().valid_packets == 0);
}

void testParseHexStream() {
    std::istringstream input("AA 01 02 34 12 F3\n");
    std::vector<std::uint8_t> bytes;

    const bool ok = spp::parseHexStream(input, bytes);

    assert(ok);
    assert((bytes == std::vector<std::uint8_t>{0xAA, 0x01, 0x02, 0x34, 0x12, 0xF3}));
}

void testParseHexStreamRejectsOddDigits() {
    std::istringstream input("AA 1");
    std::vector<std::uint8_t> bytes;

    const bool ok = spp::parseHexStream(input, bytes);

    assert(!ok);
}

void testParseHexStreamRejectsInvalidChar() {
    std::istringstream input("AA GG");
    std::vector<std::uint8_t> bytes;

    const bool ok = spp::parseHexStream(input, bytes);

    assert(!ok);
}

void testValidatePacketRejectsWrongLength() {
    spp::Packet packet;
    packet.type = spp::PacketType::Temperature;
    packet.payload = {0x34};
    assert(!spp::validatePacket(packet));
    auto decoded = spp::decodePacket(packet);
    assert(!decoded.temperature_c.has_value());
}

void testValidatePacketAcceptsCorrectLength() {
    spp::Packet packet;
    packet.type = spp::PacketType::Temperature;
    packet.payload = {0x34, 0x12};
    assert(spp::validatePacket(packet));
    auto decoded = spp::decodePacket(packet);
    assert(decoded.temperature_c.has_value());
}

void testValidatePacketRejectsUnknownType() {
    spp::Packet packet;
    packet.type = spp::PacketType::Unknown;
    packet.payload = {0x34, 0x12};
    assert(!spp::validatePacket(packet));
    auto decoded = spp::decodePacket(packet);
    assert(!decoded.temperature_c.has_value());
}

void testParserResyncsAfterNoise() {
    spp::PacketParser parser;
    const auto valid = makePacket(0x01, {0x34, 0x12});

    std::vector<std::uint8_t> stream{0x00, 0xFF, 0x10, 0xAB, 0x55};
    stream.insert(stream.end(), valid.begin(), valid.end());

    bool sawValid = false;
    for (std::uint8_t b : stream) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::ValidPacket) {
            sawValid = true;
        }
    }

    assert(sawValid);
    assert(parser.stats().valid_packets == 1);
    assert(parser.stats().dropped_packets == 0);
    assert(parser.stats().checksum_failures == 0);
}

void testTruncatedFrameDoesNotPoisonNextPacket() {
    spp::PacketParser parser;

    // Starts a frame with length 2 but only one payload byte arrives.
    const std::vector<std::uint8_t> truncated{spp::kStartByte, 0x01, 0x02, 0x34};
    const auto firstCandidate = makePacket(0x03, {0x7F});
    const auto recoveryPacket = makePacket(0x03, {0x42});

    std::vector<std::uint8_t> stream = truncated;
    stream.insert(stream.end(), firstCandidate.begin(), firstCandidate.end());
    stream.insert(stream.end(), recoveryPacket.begin(), recoveryPacket.end());

    bool sawValid = false;
    for (std::uint8_t b : stream) {
        const auto outcome = parser.consume(b);
        if (outcome.event == spp::ParseEvent::ValidPacket) {
            sawValid = true;
            assert(outcome.packet.has_value());
            assert(outcome.packet->type == spp::PacketType::Status);
            assert(outcome.packet->payload.size() == 1);
            assert(outcome.packet->payload[0] == 0x42);
        }
    }

    assert(sawValid);
    assert(parser.stats().checksum_failures == 1);
    assert(parser.stats().valid_packets == 1);
}

void testMixedStreamStats() {
    spp::PacketParser parser;

    const auto validTemp = makePacket(0x01, {0x34, 0x12});
    auto badVoltage = makePacket(0x02, {0x88, 0x13});
    badVoltage.back() ^= 0xFF;
    const auto oversize = std::vector<std::uint8_t>{spp::kStartByte, 0x03,
                                                    static_cast<std::uint8_t>(spp::kMaxPayloadLength + 1)};

    std::vector<std::uint8_t> stream;
    stream.insert(stream.end(), validTemp.begin(), validTemp.end());
    stream.insert(stream.end(), badVoltage.begin(), badVoltage.end());
    stream.insert(stream.end(), oversize.begin(), oversize.end());

    for (std::uint8_t b : stream) {
        parser.consume(b);
    }

    const auto stats = parser.stats();
    assert(stats.total_packets == 2);
    assert(stats.valid_packets == 1);
    assert(stats.checksum_failures == 1);
    assert(stats.dropped_packets == 1);
}

}  // namespace

int main() {
    testChecksum();
    testValidPacket();
    testChecksumFailure();
    testOversizedDrop();
    testCustomMaxPayloadDrop();
    testParseHexStream();
    testParseHexStreamRejectsOddDigits();
    testParseHexStreamRejectsInvalidChar();
    testValidatePacketAcceptsCorrectLength();
    testValidatePacketRejectsUnknownType();
    testValidatePacketRejectsWrongLength();
    testParserResyncsAfterNoise();
    testTruncatedFrameDoesNotPoisonNextPacket();
    testMixedStreamStats();
    return 0;
}
