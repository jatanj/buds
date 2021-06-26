#pragma once

#include <string>
#include <variant>
#include <boost/algorithm/string.hpp>

#include "log.h"
#include "message.h"

namespace buds {

struct Config {
    struct Command {
        std::string connect;
        std::string disconnect;
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

Config parseConfig(const std::string& path);

} // namespace buds

namespace fmt
{

inline std::string toString(const std::optional<buds::Config::TouchpadAction>& action)
{
    if (!action) {
        return "<none>";
    }
    if (const auto* custom = std::get_if<buds::Config::BashAction>(&*action)) {
        return fmt::format("custom:{}", custom->command);
    }
    if (const auto* custom = std::get_if<buds::TouchpadPredefinedAction>(&*action)) {
        return fmt::format("predefined:{}", *custom);
    }
    return "<unknown>";

}

FMT_FORMATTER(
    buds::Config,
    "Config{{"
    "command={{connect={}, disconnect={}}}, "
    "address={}, "
    "output={{type={}, file={}}}, "
    "lockTouchpad={}, "
    "mainEarbud={}, "
    "touchpadAction={{left={}, right={}}}"
    "}}",
    value.command.connect, value.command.disconnect,
    value.address,
    value.output.type, value.output.file,
    value.lockTouchpad,
    value.mainEarbud,
    toString(value.touchpadAction.left), toString(value.touchpadAction.right));

} // namespace fmt
