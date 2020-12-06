#include "buds_client.h"

namespace buds {

int BudsClient::connect(const std::chrono::duration<int64_t>& duration)
{
    if (auto error = btClient_.connect(config.address)) {
        return error;
    }

    readTask_ = std::async(std::launch::async, [this]{
        return btClient_.read([this](auto&& p) {
            read(std::forward<decltype(p)>(p));
        });
    });

    lockTouchpad(config.lockTouchpad);

    if (config.mainEarbud) {
        changeMainEarbud(*config.mainEarbud);
    }

    readTask_.wait_for(duration);
    return readTask_.get();
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
        auto data = status->data;
        if (data) {
            LOG_INFO("{}", *data);
            if (output_) {
                output_->update(*data);
            }
        } else {
            LOG_ERROR("EXTENDED_STATUS_UPDATED has no data");
        }

        // We need to send an empty EXTENDED_STATUS_MESSAGE response back followed by MANAGER_INFO.
        write(ExtendedStatusUpdatedMessage{std::nullopt});
        write(ManagerInfoMessage{ManagerInfoData{}});

    } else if (auto* status = dynamic_cast<StatusUpdatedMessage*>(msg)) {
        LOG_INFO("{}", status->data());
        if (output_) {
            output_->update(status->data());
        }

        if (config.mainEarbud && *config.mainEarbud != status->data().mainConnection) {
            changeMainEarbud(*config.mainEarbud);
        }
    }

    if (output_) {
        output_->render();
    }
}

} // namespace buds
