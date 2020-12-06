#include "bluetooth_client.h"

#include <cstdio>
#include <ostream>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#include <fmt/core.h>

#include "util.h"

namespace buds {

BluetoothClient::~BluetoothClient()
{
    close();
}

int BluetoothClient::connect(const std::string& address)
{
    close();

    struct sockaddr_rc addr{};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = static_cast<uint8_t>(1);
    str2ba(address.c_str(), &addr.rc_bdaddr);

    socket_ = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (socket_ < 0) {
        LOG_INFO("Failed to create socket ({})", errnoToString(errno).c_str());
        return errno;
    }

    if (::connect(socket_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        LOG_INFO("Failed to connect to {} ({})", address.c_str(), errnoToString(errno).c_str());
        return errno;
    }

    LOG_INFO("Connected to {}", address.c_str());
    return 0;
}

void BluetoothClient::close()
{
    if (socket_ >= 0) {
        auto n = ::close(socket_);
        if (n < 0) {
            LOG_INFO("Failed to close socket ({})", errnoToString(errno).c_str());
        } else {
            LOG_INFO("Closed socket");
            socket_ = -1;
        }
    }
}

ssize_t BluetoothClient::write(const std::vector<uint8_t>& msg) const
{
    if (socket_ < 0) {
        return false;
    }

    LOG_INFO("request[{} bytes]={}", msg.size(), util::toHex(msg, true).c_str());

    auto n = ::write(socket_, msg.data(), msg.size());
    if (n < 0) {
        LOG_INFO("Failed to write {} bytes : {}", msg.size(), errnoToString(errno).c_str());
    }
    return n;
}

} // namespace buds

