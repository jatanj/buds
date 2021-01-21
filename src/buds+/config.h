#pragma once

#include <string>
#include <variant>
#include <boost/algorithm/string.hpp>

#include "log.h"
#include "message.h"

namespace buds
{

struct Config {
    struct Command {
        std::string connect;
        std::string reconnect;
    } command;

    std::string address;

    enum OutputConfig {
        ARGOS
    };
    struct Output {
        std::optional<OutputConfig> type;
        std::string file;
    } output;

    bool lockTouchpad = false;
    std::optional<buds::MainEarbud> mainEarbud;
    std::optional<buds::EqualizerMode> equalizer;

    struct BashAction {
        std::string command;
    };
    using TouchpadAction = std::variant<TouchpadPredefinedAction, BashAction>;
    struct TouchpadActions {
        std::optional<TouchpadAction> left;
        std::optional<TouchpadAction> right;
    } touchpadAction;
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
