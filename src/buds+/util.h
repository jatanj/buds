#pragma once

#include <vector>
#include <string>

namespace buds::util {

std::string toHex(const std::vector<uint8_t> &bytes, bool spaced = false);

std::string shellExpand(const std::string &s);

} // namespace buds::util

