#include "util.h"

#include <sstream>
#include <iomanip>
#include <wordexp.h>

namespace buds::util {

std::string toHex(const std::vector<uint8_t>& bytes, bool spaced)
{
    if (bytes.empty()) {
        return {};
    }
    std::ostringstream s;
    auto size = bytes.size();
    for (size_t i = 0; i < size; i++) {
        s << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]);
        if (spaced && i < size - 1) {
            s << " ";
        }
    }
    return s.str();
}

std::string shellExpand(const std::string &s) {
  wordexp_t p;
  wordexp(s.c_str(), &p, 0);
  std::string expanded = p.we_wordv[p.we_offs];
  wordfree(&p);
  return expanded;
}

} // namespace buds::util

