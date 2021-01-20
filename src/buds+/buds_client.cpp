#include "buds_client.h"
#include "message.h"
#include <bits/stdint-uintn.h>

namespace buds {

int BudsClient::connect()
{
    if (auto error = btClient_.connect(config_.address)) {
        return error;
    }

    readTask_ = std::async(std::launch::async, [this]{
        return btClient_.read([this](auto&& p) {
            read(std::forward<decltype(p)>(p));
        });
    });

    lockTouchpad(config_.lockTouchpad);

    if (config_.mainEarbud) {
        changeMainEarbud(*config_.mainEarbud);
    }

    if (config_.equalizer) {
        changeEqualizerMode(*config_.equalizer);
    }

    readTask_.wait_for(std::chrono::hours::max());
    return readTask_.get();
}

int BudsClient::close()
{
    return btClient_.close();
}

void BudsClient::lockTouchpad(bool enabled)
{
    LOG_INFO("lockTouchpad({})", enabled);
    write(LockTouchpadMessage{enabled});
}

void BudsClient::changeMainEarbud(MainEarbud earbud)
{
    LOG_INFO("changeMainEarbud({})", static_cast<uint8_t>(earbud));
    write(MainChangeMessage{earbud});
}

void BudsClient::changeEqualizerMode(EqualizerMode mode)
{
    LOG_INFO("changeEqualizerMode({})", static_cast<uint8_t>(mode));
    write(EqualizerMessage{mode});
}

void BudsClient::read(const std::vector<uint8_t>& msg)
{
    MessageParser parser{msg};
    if (auto parsed = parser.parse()) {
        handle(parsed.get());
    }
}

void BudsClient::write(const Message& msg)
{
    btClient_.write(MessageBuilder::build(msg));
}

void BudsClient::handle(Message* msg)
{
    if (auto *status = dynamic_cast<ExtendedStatusUpdatedMessage*>(msg)) {
        handleStatusUpdate(status);
    } else if (auto* status = dynamic_cast<StatusUpdatedMessage*>(msg)) {
        handleStatusUpdate(status);
    } else if (auto* version = dynamic_cast<VersionInfoMessage*>(msg)) {
        if (version->data) {
            LOG_INFO("{}", *version->data);
        }
        write(VersionInfoMessage{});
    }
    if (output_) {
        output_->render();
    }
}

} // namespace buds
