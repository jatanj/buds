#include "config.h"

#include <optional>
#include <yaml-cpp/yaml.h>

#include "message.h"
#include "util.h"

namespace buds {

// TODO: Allow user to name each connection instead of using 'buds+'

constexpr inline auto KEY_BUDS = "buds+";
constexpr inline auto KEY_COMMAND = "command";
constexpr inline auto KEY_CONNECT = "connect";
constexpr inline auto KEY_DISCONNECT = "disconnect";
constexpr inline auto KEY_ADDRESS = "address";
constexpr inline auto KEY_OUTPUT = "output";
constexpr inline auto KEY_OUTPUT_TYPE = "type";
constexpr inline auto KEY_OUTPUT_FILE = "file";
constexpr inline auto KEY_LOCK_TOUCHPAD = "lock_touchpad";
constexpr inline auto KEY_MAIN_EARBUD = "main_earbud";
constexpr inline auto KEY_EQUALIZER = "equalizer";
constexpr inline auto KEY_TOUCHPAD_ACTION = "touchpad_action";
constexpr inline auto KEY_TOUCHPAD_LEFT = "left";
constexpr inline auto KEY_TOUCHPAD_RIGHT = "right";
constexpr inline auto KEY_TOUCHPAD_ACTION_CUSTOM = "custom";
constexpr inline auto KEY_TOUCHPAD_CUSTOM_BASH_COMMAND = "command";

constexpr inline auto EARBUD_LEFT = "left";
constexpr inline auto EARBUD_RIGHT = "right";

constexpr inline auto EQUALIZER_NORMAL = "normal";
constexpr inline auto EQUALIZER_BASS_BOOST = "bass_boost";
constexpr inline auto EQUALIZER_SOFT = "soft";
constexpr inline auto EQUALIZER_DYNAMIC = "dynamic";
constexpr inline auto EQUALIZER_CLEAR = "clear";
constexpr inline auto EQUALIZER_TREBLE_BOOST = "treble_boost";

constexpr inline auto TOUCHPAD_ACTION_VOICE_ASSISTANT = "voice_assistant";
constexpr inline auto TOUCHPAD_ACTION_VOLUME = "volume";
constexpr inline auto TOUCHPAD_ACTION_AMBIENT_SOUND = "ambient_sound";
constexpr inline auto TOUCHPAD_ACTION_SPOTIFY = "spotify";
constexpr inline auto TOUCHPAD_CUSTOM_BASH = "bash";

constexpr inline auto OUTPUT_ARGOS = "argos";

std::string toLowerTrim(const std::string& str)
{
    auto s = boost::algorithm::to_lower_copy(str);
    boost::algorithm::trim(s);
    return s;
}

std::optional<MainEarbud> parseMainEarbud(const std::string& str)
{
    auto s = toLowerTrim(str);
    if (s == EARBUD_LEFT) {
        return MainEarbud::LEFT;
    }
    if (s == EARBUD_RIGHT) {
        return MainEarbud::RIGHT;
    }
    LOG_ERROR("Invalid main earbud config '{}'", str);
    return std::nullopt;
}

std::optional<EqualizerMode> parseEqualizer(const std::string& str)
{
    auto s = toLowerTrim(str);
    if (s == EQUALIZER_NORMAL) {
        return EqualizerMode::NORMAL;
    }
    if (s == EQUALIZER_BASS_BOOST) {
        return EqualizerMode::BASS_BOOST;
    }
    if (s == EQUALIZER_SOFT) {
        return EqualizerMode::SOFT;
    }
    if (s == EQUALIZER_DYNAMIC) {
        return EqualizerMode::DYNAMIC;
    }
    if (s == EQUALIZER_CLEAR) {
        return EqualizerMode::CLEAR;
    }
    if (s == EQUALIZER_TREBLE_BOOST) {
        return EqualizerMode::TREBLE_BOOST;
    }
    LOG_ERROR("Invalid equalizer mode '{}'", str);
    return std::nullopt;
}

std::optional<Config::OutputConfig> parseOutputType(const std::string& str)
{
    auto s = toLowerTrim(str);
    if (s == OUTPUT_ARGOS) {
        return Config::OutputConfig::ARGOS;
    }
    LOG_ERROR("Invalid output config '{}'", str);
    return std::nullopt;
}

std::optional<TouchpadPredefinedAction> parseTouchpadPredefinedAction(const std::string& str)
{
    auto s = toLowerTrim(str);
    if (s == TOUCHPAD_ACTION_VOICE_ASSISTANT) {
        return TouchpadPredefinedAction::VOICE_ASSISTANT;
    }
    if (s == TOUCHPAD_ACTION_VOLUME) {
        return TouchpadPredefinedAction::VOLUME;
    }
    if (s == TOUCHPAD_ACTION_AMBIENT_SOUND) {
        return TouchpadPredefinedAction::AMBIENT_SOUND;
    }
    if (s == TOUCHPAD_ACTION_SPOTIFY) {
        return TouchpadPredefinedAction::SPOTIFY;
    }
    LOG_ERROR("Invalid touchpad action '{}'", str);
    return std::nullopt;
}

std::optional<Config::TouchpadAction> parseTouchpadAction(const YAML::Node& node)
{
    if (node.IsScalar()) {
        return parseTouchpadPredefinedAction(node.as<std::string>());
    }
    if (auto custom = node[KEY_TOUCHPAD_ACTION_CUSTOM]) {
        auto customValue = custom.as<std::string>();
        if (customValue == TOUCHPAD_CUSTOM_BASH) {
            if (auto command = node[KEY_TOUCHPAD_CUSTOM_BASH_COMMAND]) {
                return Config::BashAction{command.as<std::string>()};
            }
        }
    }
    LOG_ERROR("Failed to parse touchpad action");
    return std::nullopt;
}

Config parseConfig(const std::string &path)
{
    Config result{};

    YAML::Node config = YAML::LoadFile(path);
    if (!config) {
        return result;
    }

    if (auto connect = config[KEY_BUDS][KEY_COMMAND][KEY_CONNECT]) {
        result.command.connect = boost::trim_copy(connect.as<std::string>());
    }
    if (auto disconnect = config[KEY_BUDS][KEY_COMMAND][KEY_DISCONNECT]) {
        result.command.disconnect = boost::trim_copy(disconnect.as<std::string>());
    }
    if (auto address = config[KEY_BUDS][KEY_ADDRESS]) {
        result.address = address.as<std::string>();
    }
    if (auto outputType = config[KEY_BUDS][KEY_OUTPUT][KEY_OUTPUT_TYPE]) {
        result.output.type = parseOutputType(outputType.as<std::string>());
    }
    if (auto outputFile = config[KEY_BUDS][KEY_OUTPUT][KEY_OUTPUT_FILE]) {
        result.output.file = util::shellExpand(outputFile.as<std::string>());
    }
    if (auto lockTouchpad = config[KEY_BUDS][KEY_LOCK_TOUCHPAD]) {
        result.lockTouchpad = lockTouchpad.as<bool>();
    }
    if (auto mainEarbud = config[KEY_BUDS][KEY_MAIN_EARBUD]) {
        result.mainEarbud = parseMainEarbud(mainEarbud.as<std::string>());
    }
    if (auto equalizer = config[KEY_BUDS][KEY_EQUALIZER]) {
        result.equalizer = parseEqualizer(equalizer.as<std::string>());
    }
    if (auto touchpadAction = config[KEY_BUDS][KEY_TOUCHPAD_ACTION]) {
        if (auto left = touchpadAction[KEY_TOUCHPAD_LEFT]) {
            result.touchpadAction.left = parseTouchpadAction(left);
        }
        if (auto right = touchpadAction[KEY_TOUCHPAD_RIGHT]) {
            result.touchpadAction.right = parseTouchpadAction(right);
        }
    }

    return result;
}

} // namespace buds

