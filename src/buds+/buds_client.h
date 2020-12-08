#pragma once

#include <bits/stdint-intn.h>
#include <chrono>
#include <memory>
#include <string>
#include <future>
#include <fstream>

#include "bluetooth_client.h"
#include "config.h"
#include "message.h"
#include "message_parser.h"
#include "message_builder.h"
#include "output.h"
#include "constants.h"

namespace buds {

using namespace constants;

class BudsClient {
public:
    BudsClient(Config config, std::shared_ptr<Output> output) :
        config_(std::move(config)), output_(std::move(output)) {}

    int connect();

    int close();

    void lockTouchpad(bool enabled);

    void changeMainEarbud(MainEarbud earbud);

  private:
    const Config config_;
    const std::shared_ptr<Output> output_;
    const MessageBuilder builder_{};

    BluetoothClient btClient_;
    std::future<int> readTask_;

    void read(const std::vector<uint8_t>& msg);

    void write(const Message& msg);

    void handle(Message* msg);

    template <typename T>
    void handleStatusUpdate(T* status)
    {
        if (status->data) {
            LOG_INFO("{}", *status->data);
            if (output_) {
                output_->update(*status->data);
            }
        } else {
            LOG_ERROR("Message has no data");
        }

        // Respond with one zero byte with the same message id.
        write(T{});

        // Also respond with MANAGER_INFO for EXTENDED_STATUS_UPDATED messages.
        // Not sure if we need to do this for STATUS_UPDATED messages.
        if constexpr (std::is_same_v<T, ExtendedStatusUpdatedMessage>) {
            write(ManagerInfoMessage{ManagerInfoData{}});
        }

        if (config_.mainEarbud && *config_.mainEarbud != status->data->mainConnection) {
            changeMainEarbud(*config_.mainEarbud);
        }
    }

};

} // namespace buds
