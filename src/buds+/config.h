#pragma once

#include <string>
#include <variant>
#include <boost/algorithm/string.hpp>
#include <unordered_set>

#include "log.h"
#include "message.h"

namespace buds {

struct Config {
    struct Command {
        std::string connect;
        std::string disconnect;
    } command;

    std::string address;

    struct OnFailure {
        std::unordered_set<int> reconnect;
        std::unordered_set<int> pause;
    } onFailure;

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

constexpr inline auto NONE = "<none>";
constexpr inline auto UNKNOWN = "<unknown>";

inline std::string toString(const std::optional<buds::Config::TouchpadAction>& action)
{
    if (!action) {
        return NONE;
    }
    if (const auto* custom = std::get_if<buds::Config::BashAction>(&*action)) {
        return fmt::format("custom:{}", custom->command);
    }
    if (const auto* custom = std::get_if<buds::TouchpadPredefinedAction>(&*action)) {
        return fmt::format("predefined:{}", *custom);
    }
    return UNKNOWN;

}

inline std::string toString(const std::optional<buds::Config::OnFailure>& onFailure)
{
    std::string s = NONE;
    if (onFailure) {
        s = fmt::format("{{reconnect={}, pause={}}}", fmt::join(onFailure->reconnect, ","), fmt::join(onFailure->pause, ","));
    }
    return s;
}

FMT_FORMATTER(
    buds::Config,
    "Config{{"
    "command={{connect={}, disconnect={}}}, "
    "address={}, "
    "onFailure={}, "
    "output={{type={}, file={}}}, "
    "lockTouchpad={}, "
    "mainEarbud={}, "
    "touchpadAction={{left={}, right={}}}"
    "}}",
    value.command.connect, value.command.disconnect,
    value.address,
    toString(value.onFailure),
    value.output.type, value.output.file,
    value.lockTouchpad,
    value.mainEarbud,
    toString(value.touchpadAction.left), toString(value.touchpadAction.right));

} // namespace fmt
