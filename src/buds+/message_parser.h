#pragma once

#include <cstdint>
#include <vector>
#include <memory>

#include "message.h"

namespace buds {

class MessageParser {
public:
    explicit MessageParser(const std::vector<uint8_t>& msg) : msg_(msg) {}

    std::unique_ptr<buds::Message> parse();

private:
    const std::vector<uint8_t>& msg_;
    size_t index_ = 0;

    const uint8_t* next(size_t n);

    template<typename M, typename D>
    std::unique_ptr<M> parseMessage(const uint8_t* p)
    {
        D data{};
        auto size = sizeof(data);
        if (!next(size)) {
            LOG_ERROR("Failed to parse message");
            return {};
        }
        memcpy(&data, p, size);
        return std::make_unique<M>(data);
    }
};

} // namespace buds
