#pragma once

#include <string>
#include "log.h"
#include "message.h"
#include <boost/algorithm/string.hpp>

namespace buds
{

enum OutputConfig {
    ARGOS
};

struct Config {
    struct Command {
        std::string connect;
        std::string restart;
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
    "command={{connect={}, restart={}}}, "
    "address={}, "
    "output={{type={}, file={}}}, "
    "lockTouchpad={}, "
    "mainEarbud={}"
    "}}",
    value.command.connect, value.command.restart,
    value.address,
    value.output.type, value.output.file,
    value.lockTouchpad,
    value.mainEarbud);

} // namespace fmt
