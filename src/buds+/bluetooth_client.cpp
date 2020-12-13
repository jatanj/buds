#include "bluetooth_client.h"

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
    if (auto error = close()) {
        return error;
    }

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

int BluetoothClient::close()
{
    if (socket_ != SOCKET_CLOSED) {
        auto n = ::close(socket_);
        if (n < 0) {
            LOG_INFO("Failed to close connection ({})", errnoToString(errno).c_str());
            return errno;
        }
        LOG_INFO("Closed connection");
        socket_ = SOCKET_CLOSED;
    }
    return 0;
}

int BluetoothClient::write(const std::vector<uint8_t>& msg) const
{
    if (socket_ < 0) {
        LOG_ERROR("Socket is closed (cannot write {} bytes)", msg.size());
        return -1;
    }

    LOG_INFO("request[{} bytes]={}", msg.size(), util::toHex(msg, true).c_str());

    auto n = ::write(socket_, msg.data(), msg.size());
    if (n < 0) {
        LOG_INFO("Failed to write {} bytes : {}", msg.size(), errnoToString(errno).c_str());
        return errno;
    }
    if (static_cast<size_t>(n) < msg.size()) {
        LOG_INFO("Failed to write {} bytes ({} written)", msg.size(), n);
        return -1;
    }
    return 0;
}

} // namespace buds

