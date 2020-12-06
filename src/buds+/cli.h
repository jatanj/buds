#pragma once

#include <cstddef>
#include <optional>
#include <CLI/CLI.hpp>
#include "util.h"

namespace buds {

using ArgsMap = std::map<std::string, std::string>;

constexpr inline auto PROGRAM_NAME = "buds+";
constexpr inline auto PROGRAM_DESCRIPTION = "Unofficial tool to configure and monitor Galaxy Buds+";

constexpr inline auto OPTION_CONFIG_FILE = "--config";
constexpr inline auto OPTION_ADDRESS = "--address";

struct CliArgs {
    std::optional<std::string> configFile;
    std::optional<std::string> address;
};

CLI::Option* addOption(CLI::App* cmd, ArgsMap* args, const std::string& name);

CLI::Option *addFlag(CLI::App *cmd, ArgsMap *args, const std::string &name);

struct ArgsConverter {
    explicit ArgsConverter(const ArgsMap& args) : args_(args) {}

    std::optional<std::string> string(const std::string& name)
    {
        auto value = args_.find(name);
        if (value == args_.end()) {
            return std::nullopt;
        }
        return util::shellExpand(value->second);
    }

private:
    const ArgsMap& args_;
};

CliArgs parseArgs(std::vector<std::string> argv);

} // namespace buds
