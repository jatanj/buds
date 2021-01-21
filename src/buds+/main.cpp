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

constexpr auto DEFAULT_CONFIG_DIR = "$HOME/.config/buds+";
constexpr auto DEFAULT_CONFIG_FILE = "default.yaml";

constexpr auto RECONNECT_WAIT_SECONDS = 5;
constexpr auto RECONNECT_WAIT_MULTIPLIER = 2;
constexpr auto RECONNECT_WAIT_MAX = 60;
const std::unordered_set<int> RECONNECT_RETURN_CODES{ // NOLINT
    ECONNABORTED
};
const std::unordered_set<int> SUCCESS_RETURN_CODES{ // NOLINT
    0
};

int doConnectCommand(const buds::Config& config)
{
    if (config.command.connect.empty()) {
        return 0;
    }
    // FIXME: Avoid using system()
    return system(config.command.connect.c_str()); // NOLINT
}

struct ReconnectData {
    buds::BudsClient* buds = nullptr;
    std::string command;
} reconnectData; // NOLINT

void handleReconnect(int signum)
{
    boost::algorithm::trim(reconnectData.command);
    if (!reconnectData.buds || reconnectData.command.empty()) {
        return;
    }

    if (auto error = reconnectData.buds->close()) {
        LOG_ERROR("Cannot reconnect (failed to close connection : {})", error);
        return;
    }

    switch (signum) {
        case SIGHUP:
            // FIXME: Avoid using system()
            system(reconnectData.command.c_str()); // NOLINT
            break;
        default:
            LOG_WARN("Sigal {} not handled", signum);
            break;
    }
}

std::filesystem::path defaultConfig()
{
    std::filesystem::path p{buds::util::shellExpand(DEFAULT_CONFIG_DIR)};
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
            case buds::Config::ARGOS:
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
    auto args = buds::parseArgs(std::vector<std::string>{argv + 1, argv + argc});

    auto config = loadConfig(args);
    auto output = initOutput(config);

    buds::BudsClient buds{config, std::move(output)};

    reconnectData.buds = &buds;
    reconnectData.command = config.command.reconnect;
    signal(SIGHUP, handleReconnect);

    int wait = RECONNECT_WAIT_SECONDS;
    while (true) {
        if (auto rc = doConnectCommand(config)) {
            LOG_ERROR("Connect command failed with code {}", rc);
        }

        auto rc = buds.connect();
        if (RECONNECT_RETURN_CODES.count(rc)) {
            std::this_thread::sleep_for(std::chrono::seconds(wait));
            wait = std::min(wait * RECONNECT_WAIT_MULTIPLIER, RECONNECT_WAIT_MAX);
        } else if (SUCCESS_RETURN_CODES.count(rc)) {
            wait = RECONNECT_WAIT_SECONDS;
        } else {
            return rc;
        }
    }

    return 0;
}

