#pragma once

#include <cstdint>
#include <vector>

namespace spp {

std::uint8_t computeChecksum(const std::vector<std::uint8_t>& bytes);

}  // namespace spp
