#include "config.h"
#include "util.h"

#include <yaml-cpp/yaml.h>

namespace buds {

// TODO: Allow user to name each connection instead of using 'buds+'

constexpr inline auto KEY_BUDS = "buds+";
constexpr inline auto KEY_COMMAND = "command";
constexpr inline auto KEY_CONNECT = "connect";
constexpr inline auto KEY_RESTART = "restart";
constexpr inline auto KEY_ADDRESS = "address";
constexpr inline auto KEY_OUTPUT = "output";
constexpr inline auto KEY_OUTPUT_TYPE = "type";
constexpr inline auto KEY_OUTPUT_FILE = "file";
constexpr inline auto KEY_LOCK_TOUCHPAD = "lock_touchpad";
constexpr inline auto KEY_MAIN_EARBUD = "main_earbud";

constexpr inline auto EARBUD_LEFT = "left";
constexpr inline auto EARBUD_RIGHT = "right";

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

std::optional<OutputConfig> parseOutputType(const std::string& str)
{
    auto s = toLowerTrim(str);
    if (s == OUTPUT_ARGOS) {
        return OutputConfig::ARGOS;
    }
    LOG_ERROR("Invalid output config '{}'", str);
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
        result.command.connect = connect.as<std::string>();
    }
    if (auto restart = config[KEY_BUDS][KEY_COMMAND][KEY_RESTART]) {
        result.command.restart = restart.as<std::string>();
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

    return result;
}

} // namespace buds

