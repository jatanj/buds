#pragma once

#include <chrono>
#include <string>
#include <future>

#include "bluetooth_client.h"
#include "config.h"
#include "message.h"
#include "message_parser.h"
#include "message_builder.h"
#include "output.h"
#include "constants.h"

namespace buds {

using namespace constants;

class BudsPlusCapabilities {
public:
    virtual ~BudsPlusCapabilities() = default;

    virtual void lockTouchpad(bool enabled) = 0;

    virtual void changeMainEarbud(MainEarbud earbud) = 0;

    virtual void changeEqualizerMode(EqualizerMode mode) = 0;

    virtual void setTouchpadOption(TouchpadActions actions) = 0;
};

class BudsClient : BudsPlusCapabilities {
public:
    BudsClient(Config config, std::shared_ptr<Output> output) :
        config_(std::move(config)), output_(std::move(output)) {}

    ~BudsClient() override = default;

    int connect();

    int blockingConnect();

    int close();

    void lockTouchpad(bool enabled) override;

    void changeMainEarbud(MainEarbud earbud) override;

    void changeEqualizerMode(EqualizerMode mode) override;

    void setTouchpadOption(TouchpadActions actions) override;

    Output& output() const { return *output_; }

  private:
    const Config config_;
    const std::shared_ptr<Output> output_;
    const MessageBuilder builder_{};

    BluetoothClient btClient_;
    std::future<int> readTask_;
    std::unordered_map<uint8_t, std::function<void(void)>> touchpadActions_;

    void read(const std::vector<uint8_t>& msg);

    void write(const Message& msg);

    void handle(Message* msg);

    template <typename T>
    void handleStatusUpdate(T* msg)
    {
        if (msg->data) {
            LOG_INFO("{}", *msg->data);
            if (output_) {
                output_->update(*msg->data);
            }
        }

        // Respond with one zero byte with the same message id.
        write(T{});

        // Also respond with MANAGER_INFO for EXTENDED_STATUS_UPDATED messages.
        // Not sure if we need to do this for STATUS_UPDATED messages.
        if constexpr (std::is_same_v<T, ExtendedStatusUpdatedMessage>) {
            write(ManagerInfoMessage{ManagerInfoData{}});
        }

        if (config_.mainEarbud && *config_.mainEarbud != msg->data->mainConnection) {
            changeMainEarbud(*config_.mainEarbud);
        }
    }

    void configureTouchpadActions(const Config::TouchpadActions& actions);
};

} // namespace buds
