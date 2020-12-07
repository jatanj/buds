#pragma once

#include <vector>
#include <array>
#include <unistd.h>
#include <cerrno>

#include <fmt/core.h>

#include "log.h"
#include "util.h"

namespace buds {

class BluetoothClient {
public:
    ~BluetoothClient();

    int connect(const std::string& address);

    int close();

    int write(const std::vector<uint8_t>& msg) const;

    template<typename F>
    int read(const F& callback)
    {
        ssize_t n = 0;
        do {
            auto s = socket_;
            if (s == SOCKET_CLOSED) {
                break;
            }
            n = ::read(s, readBuffer_, READ_BUFFER_SIZE);
            if (n > 0) {
                auto data = std::vector<uint8_t>(readBuffer_, readBuffer_ + n);
                LOG_INFO("response[{} bytes]={}", n, util::toHex(data, true).c_str());
                callback(std::move(data));
            } else if (n < 0) {
                LOG_INFO("Failed to read : {}", errnoToString(errno).c_str());
                return errno;
            }
        } while (n > 0);
        return 0;
    }

private:
    static constexpr auto ERRNO_TO_STRING_LENGTH = 64;
    static constexpr auto READ_BUFFER_SIZE = 1024;
    static constexpr auto SOCKET_CLOSED = -1;

    int socket_ = SOCKET_CLOSED;
    uint8_t readBuffer_[READ_BUFFER_SIZE] = {};

    static std::string errnoToString(int err)
    {
        std::array<char, ERRNO_TO_STRING_LENGTH> buf{0};
        auto* str = strerror_r(err, buf.data(), buf.size());
        return str ? fmt::format("{} {}", errno, str) : "";
    }
};

} // namespace buds
