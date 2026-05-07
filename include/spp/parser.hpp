#pragma once

#include <cstdint>
#include <optional>

#include "spp/packet.hpp"

namespace spp {

enum class ParseEvent {
    None,
    ValidPacket,
    ChecksumFailure,
    Dropped,
};

struct ParseOutcome {
    ParseEvent event = ParseEvent::None;
    std::optional<Packet> packet;
};

class PacketParser {
public:
    PacketParser(std::size_t maxPayload = kMaxPayloadLength);

    ParseOutcome consume(std::uint8_t byte);
    const ParserStats& stats() const;

private:
    enum class State {
        WaitStart,
        ReadType,
        ReadLength,
        ReadPayload,
        ReadChecksum,
    };

    void resetFrame();
    std::size_t maxPayload_;
    State state_;
    ParserStats stats_;
    std::uint8_t type_byte_;
    std::uint8_t length_;
    std::vector<std::uint8_t> payload_;
    std::vector<std::uint8_t> frame_bytes_;
};

}  // namespace spp
