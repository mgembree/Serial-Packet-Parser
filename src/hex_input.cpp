#include "spp/hex_input.hpp"

#include <cctype>
#include <istream>

namespace spp {
namespace {
// Returns -1 for invalid hex characters.
int hexValue(unsigned char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }

    ch = static_cast<unsigned char>(std::tolower(ch));
    if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }

    return -1;
}

}  // namespace

bool parseHexStream(std::istream& in, std::vector<std::uint8_t>& bytes) {
    bytes.clear();

    int highNibble = -1;
    char ch = 0;
    while (in.get(ch)) {
        const auto uch = static_cast<unsigned char>(ch);
        if (std::isspace(uch) != 0) {
            continue;
        }

        const int nibble = hexValue(uch);
        if (nibble < 0) {
            return false;
        }

        if (highNibble < 0) {
            highNibble = nibble;
            continue;
        }

        bytes.push_back(static_cast<std::uint8_t>((highNibble << 4) | nibble));
        highNibble = -1;
    }

    // Fail if stream ended mid-byte.
    return (in.eof() || !in.bad()) && highNibble < 0;
}

}  // namespace spp