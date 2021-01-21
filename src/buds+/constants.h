#pragma once

#include <cstdint>

namespace buds::constants {

constexpr inline uint8_t MSG_PREAMBLE = 0xfd;
constexpr inline uint8_t MSG_POSTAMBLE = 0xdd;

constexpr inline uint8_t MSG_ID_STATUS_UPDATED = 96;
constexpr inline uint8_t MSG_ID_EXTENDED_STATUS_UPDATED = 97;
constexpr inline uint8_t MSG_ID_VERSION_INFO = 99;
constexpr inline uint8_t MSG_ID_MAIN_CHANGE = 112;
constexpr inline uint8_t MSG_ID_EQUALIZER = 134;
constexpr inline uint8_t MSG_ID_MANAGER_INFO = 136;
constexpr inline uint8_t MSG_ID_LOCK_TOUCHPAD = 144;
constexpr inline uint8_t MSG_ID_SET_TOUCHPAD_OPTION = 146;
constexpr inline uint8_t MSG_ID_SET_TOUCHPAD_OTHER_OPTION = 147;

constexpr inline uint8_t SIZE_PREAMBLE = 1;
constexpr inline uint8_t SIZE_POSTAMBLE = 1;
constexpr inline uint8_t SIZE_HEADER = 2;
constexpr inline uint8_t SIZE_MSG_ID = 1;
constexpr inline uint8_t SIZE_CRC = 2;

} // namespace buds::constants

