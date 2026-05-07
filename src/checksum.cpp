#include "spp/checksum.hpp"

namespace spp {

std::uint8_t computeChecksum(const std::vector<std::uint8_t>& bytes) {
    std::uint32_t sum = 0;
    for (std::uint8_t value : bytes) {
        sum += value;
    }
    return static_cast<std::uint8_t>(sum & 0xFF);
}

}  // namespace spp
