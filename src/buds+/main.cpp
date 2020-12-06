#include <chrono>
#include <filesystem>
#include <memory>
#include <unordered_set>
#include <cstdlib>
#include <csignal>

#include "buds_client.h"
#include "argos_output.h"
#include "cli.h"
#include "config.h"
#include "output.h"
#include "util.h"

constexpr inline auto DEFAULT_SHELL = "/bin/sh";

constexpr inline auto DEFAULT_CONFIG_DIR = "$HOME/.config/buds+";
constexpr inline auto DEFAULT_CONFIG_FILE = "default.yaml";

constexpr auto RECONNECT_WAIT_SECONDS = 1;
constexpr auto RECONNECT_WAIT_MULTIPLIER = 2;
constexpr auto RECONNECT_WAIT_MAX = 60;
const std::unordered_set<int> RECONNECT_ERRORS = { // NOLINT
    ECONNABORTED
};

int doConnectCommand(const buds::Config& config)
{
    if (config.command.connect.empty()) {
        return 0;
    }
    // FIXME: Avoid using system()
    return system(config.command.connect.c_str()); // NOLINT
}

struct RestartData {
    std::string command;
} restartData; // NOLINT

void handleRestart(int signum)
{
    boost::algorithm::trim(restartData.command);
    if (restartData.command.empty()) {
        return;
    }
    switch (signum) {
        case SIGHUP:
            // FIXME: Avoid using system()
            system(restartData.command.c_str()); // NOLINT
            break;
        default:
            LOG_WARN("Sigal {} not handled", signum);
            break;
    }
}

std::filesystem::path defaultConfig()
{
    auto p = std::filesystem::path{buds::util::shellExpand(DEFAULT_CONFIG_DIR)};
    p = p / DEFAULT_CONFIG_FILE;
    return p;
}

buds::Config loadConfig(const buds::CliArgs& args)
{
    auto configFile = args.configFile ?
        std::filesystem::path{*args.configFile} :
        defaultConfig();
    LOG_INFO("Config File: {}", configFile.string());

    auto config = buds::parseConfig(configFile.string());
    LOG_INFO("{}", config);

    return config;
}

std::shared_ptr<buds::Output> initOutput(const buds::Config& config)
{
    std::shared_ptr<buds::Output> output;

    if (config.output.type) {
        if (!config.output.file.empty()) {
            LOG_INFO("Outputting to file '{}'", config.output.file);
        }
        switch (*config.output.type) {
            case buds::ARGOS:
                if (!config.output.file.empty()) {
                    output = std::make_shared<buds::ArgosOutput>(config.output.file);
                } else {
                    LOG_ERROR("File config is not set for Argos output");
                }
                break;
            default:
                LOG_WARN("Output {} is not handled", *config.output.type);
                break;
        }
    }

    return output;
}

int main(int argc, char** argv)
{
    using namespace buds;

    auto args = parseArgs(std::vector<std::string>{argv + 1, argv + argc});

    auto config = loadConfig(args);
    auto output = initOutput(config);

    restartData.command = config.command.restart;
    signal(SIGHUP, handleRestart);

    buds::BudsClient buds(config, std::move(output));

    int wait = RECONNECT_WAIT_SECONDS;
    while (true) {
        auto rc = doConnectCommand(config);
        if (rc > 0) {
            LOG_ERROR("Connect command failed with code {}", rc);
        }

        auto error = buds.connect(std::chrono::hours::max());
        if (!RECONNECT_ERRORS.count(error)) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(wait));
        wait = std::min(wait * RECONNECT_WAIT_MULTIPLIER, RECONNECT_WAIT_MAX);
    }

    return 0;
}

