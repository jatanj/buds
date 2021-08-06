#include <chrono>
#include <thread>
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

const std::unordered_set<int> SUCCESS_RETURN_CODES{0};              // NOLINT

struct SignalData {
    buds::BudsClient* buds = nullptr;
    buds::Config* config = nullptr;
    bool paused = false;
} signalData; // NOLINT

int shellCommand(const std::string& cmd)
{
    if (cmd.empty()) {
        LOG_WARN("Cannot run empty command");
        return 1;
    }
    // FIXME: Avoid using system()
    return system(cmd.c_str()); // NOLINT
}

int connect(const buds::Config& config)
{
    return shellCommand(config.command.connect);
}

void handleStop(int /*unused*/ = SIGTSTP)
{
    if (auto error = signalData.buds->close()) {
        LOG_ERROR("Cannot reconnect (failed to close connection : {})", error);
        return;
    }
    signalData.paused = shellCommand(signalData.config->command.disconnect) == 0;
}

void handleContinue(int /*unused*/ = SIGCONT)
{
    signalData.paused = shellCommand(signalData.config->command.connect) != 0;
}

void handleRestart(int /*unused*/ = SIGHUP)
{
    handleStop();

    // Bluetoothctl seems to return before the buds are 'fully' disconnected
    // so we just wait one second as a minor hack.
    std::this_thread::sleep_for(std::chrono::seconds(1));

    handleContinue();
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

    if (config.command.connect.empty()) {
        LOG_ERROR("Connect command is required");
        LOG_ERROR("Ensure your config is readable and the buds+.command.connect property is set.");
        return 1;
    }

    buds::BudsClient buds{config, std::move(output)};

    signalData.buds = &buds;
    signalData.config = &config;

    signal(SIGHUP, handleRestart);
    signal(SIGTSTP, handleStop);
    signal(SIGCONT, handleContinue);

    int wait = RECONNECT_WAIT_SECONDS;
    while (true) {
        if (signalData.paused) {
            std::this_thread::sleep_for(std::chrono::seconds(RECONNECT_WAIT_SECONDS));
            continue;
        }

        if (auto rc = connect(config)) {
            LOG_ERROR("Connect command failed with code {}", rc);
        }

        auto rc = buds.blockingConnect();
        if (config.onFailure.reconnect.count(rc)) {
            // Wait for a few seconds before attempting to reconnect.
            std::this_thread::sleep_for(std::chrono::seconds(wait));
            wait = std::min(wait * RECONNECT_WAIT_MULTIPLIER, RECONNECT_WAIT_MAX);
        } else if (config.onFailure.pause.count(rc)) {
            // Sleep indefinitely until the user manually reconnects.
            buds.close();
            signalData.paused = true;
        } else if (SUCCESS_RETURN_CODES.count(rc)) {
            // Reset wait and attempt to reconnect immediately.
            wait = RECONNECT_WAIT_SECONDS;
        } else {
            return rc;
        }
    }
}

