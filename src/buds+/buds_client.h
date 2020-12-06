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

    int connect(const std::chrono::duration<int64_t>& duration);

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
};

} // namespace buds
