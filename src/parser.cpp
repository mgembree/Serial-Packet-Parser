#include "spp/parser.hpp"

#include "spp/checksum.hpp"

namespace spp {

PacketParser::PacketParser()
    : state_(State::WaitStart),
      type_byte_(0),
      length_(0) {}

const ParserStats& PacketParser::stats() const {
    return stats_;
}

void PacketParser::resetFrame() {
    state_ = State::WaitStart;
    type_byte_ = 0;
    length_ = 0;
    payload_.clear();
    frame_bytes_.clear();
}

ParseOutcome PacketParser::consume(std::uint8_t byte) {
    stats_.bytes_processed++;

    switch (state_) {
    case State::WaitStart:
        if (byte == kStartByte) {
            frame_bytes_.clear();
            frame_bytes_.push_back(byte);
            state_ = State::ReadType;
        }
        return {};

    case State::ReadType:
        type_byte_ = byte;
        frame_bytes_.push_back(byte);
        state_ = State::ReadLength;
        return {};

    case State::ReadLength:
        length_ = byte;
        frame_bytes_.push_back(byte);
        payload_.clear();

        if (length_ > kMaxPayloadLength) {
            stats_.dropped_packets++;
            resetFrame();
            return {ParseEvent::Dropped, std::nullopt};
        }

        state_ = (length_ == 0) ? State::ReadChecksum : State::ReadPayload;
        return {};

    case State::ReadPayload:
        payload_.push_back(byte);
        frame_bytes_.push_back(byte);

        if (payload_.size() >= length_) {
            state_ = State::ReadChecksum;
        }
        return {};

    case State::ReadChecksum: {
        const std::uint8_t expected = computeChecksum(frame_bytes_);
        stats_.total_packets++;

        if (byte != expected) {
            stats_.checksum_failures++;
            resetFrame();
            return {ParseEvent::ChecksumFailure, std::nullopt};
        }

        Packet packet;
        packet.type = packetTypeFromByte(type_byte_);
        packet.payload = payload_;

        stats_.valid_packets++;
        resetFrame();
        return {ParseEvent::ValidPacket, packet};
    }
    }

    return {};
}

}  // namespace spp
