#pragma once

#include "spp/packet.hpp"

namespace spp {

bool validatePacket(const Packet& packet);
DecodedPacket decodePacket(const Packet& packet);

}  // namespace spp
