#pragma once

#include <string>
#include <boost/algorithm/string.hpp>

#include "log.h"
#include "message.h"

namespace buds
{

enum OutputConfig {
    ARGOS
};

struct Config {
    struct Command {
        std::string connect;
        std::string reconnect;
    };

    struct Output {
        std::optional<OutputConfig> type;
        std::string file;
    };

    Command command;
    std::string address;
    Output output;
    bool lockTouchpad = false;
    std::optional<buds::MainEarbud> mainEarbud;
};

Config parseConfig(const std::string &path);

} // namespace buds

namespace fmt {

FMT_FORMATTER(
    buds::Config,
    "Config{{"
    "command={{connect={}, reconnect={}}}, "
    "address={}, "
    "output={{type={}, file={}}}, "
    "lockTouchpad={}, "
    "mainEarbud={}"
    "}}",
    value.command.connect, value.command.reconnect,
    value.address,
    value.output.type, value.output.file,
    value.lockTouchpad,
    value.mainEarbud);

} // namespace fmt
