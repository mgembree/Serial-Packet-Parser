#pragma once

#include <cstdint>
#include <iosfwd>
#include <vector>

namespace spp {

bool parseHexStream(std::istream& in, std::vector<std::uint8_t>& bytes);

}  // namespace spp