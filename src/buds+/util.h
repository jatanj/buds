#pragma once

#include <vector>
#include <string>
#include <sstream>

#include "log.h"

namespace buds::util {

std::string toHex(const std::vector<uint8_t> &bytes, bool spaced = false);

std::string shellExpand(const std::string &s);

inline void print(const std::vector<uint8_t>& msg)
{
    LOG_INFO("{} ({} bytes)", toHex(msg, true).c_str(), msg.size());
}

} // namespace buds::util

