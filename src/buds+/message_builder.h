#pragma once

#include <vector>
#include <optional>
#include <stdint.h>
#include <stdio.h>

#include "galaxy-buds-rfcomm-crc16-ccitt.h"
#include "constants.h"
#include "message.h"
#include "util.h"

namespace buds {

using namespace constants;

struct MessageBuilder {
public:
    static std::vector<uint8_t> build(const Message& msg)
    {
        return buildMessage(msg.id(), msg.payload(), msg.isResponse());
    }

private:
    static std::optional<uint16_t> buildHeader(const std::vector<uint8_t>& payload, bool isResponse)
    {
        constexpr uint16_t isFragment = 0; // TODO?

        auto size = payload.size() + SIZE_MSG_ID + SIZE_CRC;
        if (size > 0x7ff) {
            return std::nullopt;
        }

        return (isResponse << 12) | (isFragment << 11) | (static_cast<uint16_t>(size) & 0x7ff);
    }

    static std::vector<uint8_t> buildMessage(uint8_t id, const std::vector<uint8_t>& payload, bool isResponse = false)
    {
        auto header = buildHeader(payload, isResponse);
        if (!header) {
            return {};
        }

        std::vector<uint8_t> msg;
        msg.reserve(SIZE_PREAMBLE + sizeof(header) + SIZE_MSG_ID + payload.size() + SIZE_CRC + SIZE_POSTAMBLE);

        msg.push_back(MSG_PREAMBLE);

        append(msg, *header);
        msg.push_back(id);
        msg.insert(msg.end(), payload.begin(), payload.end());

        std::vector<uint8_t> crc;
        crc.push_back(id);
        crc.insert(crc.end(), payload.begin(), payload.end());
        append(msg, crc16(crc));

        msg.push_back(MSG_POSTAMBLE);
        return msg;
    }

    template <typename T>
    static void append(std::vector<uint8_t>& msg, T data)
    {
        for (size_t i = 0; i < sizeof(data); i++) {
            msg.push_back(data & 0xff);
            data = data >> 8;
        }
    }
};

} // namespace buds
