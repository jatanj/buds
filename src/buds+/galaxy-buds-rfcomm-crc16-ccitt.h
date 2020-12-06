#pragma once

#include <vector>
#include <stdint.h>

namespace buds {

uint16_t crc16(const std::vector<uint8_t>& data);

} // namespace buds

