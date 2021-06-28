#include "argos_output.h"

#include <fstream>
#include <optional>
#include <ostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <fmt/format.h>
#include <boost/algorithm/string/replace.hpp>

#include "message.h"

namespace buds
{

constexpr inline auto HEADPHONES_EMOJI = ":headphones:";

constexpr inline auto IN_EAR_EMOJI = ":ear:";
constexpr inline auto OUTSIDE_EAR_EMOJI = ":hand:";
constexpr inline auto IN_CASE_EMOJI = ":electric_plug:";

constexpr inline auto OUTPUT_FILE_TEMP_PREFIX = "/tmp/buds_";
constexpr inline auto OUTPUT_FILE_SHEBANG = "#!/bin/bash";
constexpr inline auto OUTPUT_FILE_PERMISSIONS = 0755;

void ArgosOutput::update(const BudsState& state)
{
    state_.isConnected = state.isConnected;
}

void ArgosOutput::update(const StatusUpdatedData& data)
{
    const std::lock_guard<std::mutex> lock(updateMutex_);

    state_.leftBattery = data.deviceBatGageL;
    state_.rightBattery = data.deviceBatGageR;

    PlacementParser placement{data.placement};
    state_.leftPlacement = placement.left();
    state_.rightPlacement = placement.right();
}

void ArgosOutput::update(const ExtendedStatusUpdatedData& data)
{
    const std::lock_guard<std::mutex> lock(updateMutex_);

    state_.leftBattery = data.deviceBatGageL;
    state_.rightBattery = data.deviceBatGageR;

    PlacementParser placement{data.placement};
    state_.leftPlacement = placement.left();
    state_.rightPlacement = placement.right();
}

std::optional<uint8_t> ArgosOutput::batteryInfo(const BudsTrayState& state)
{
    if (state.leftBattery && state.rightBattery) {
        return std::min(*state.leftBattery, *state.rightBattery);
    }
    if (state.leftBattery) {
        return *state.leftBattery;
    }
    if (state.rightBattery) {
        return *state.rightBattery;
    }
    return std::nullopt;
}

std::string ArgosOutput::wearStatusInfo(const BudsTrayState& state)
{
    static auto indicator = [](PlacementParser::Placement status) {
        switch (status) {
            case PlacementParser::Placement::IN_EAR: return IN_EAR_EMOJI;
            case PlacementParser::Placement::OUTSIDE_EAR: return OUTSIDE_EAR_EMOJI;
            case PlacementParser::Placement::IN_CASE: return IN_CASE_EMOJI;
            default: return "";
        }
    };
    std::string s;
    if (state.leftPlacement) {
        s += indicator(*state.leftPlacement);
        s += "L";
    }
    if (state.rightPlacement) {
        s += "R";
        s += indicator(*state.rightPlacement);
    }
    return s;
}

void ArgosOutput::render()
{
    const std::lock_guard<std::mutex> lock(updateMutex_);

    if (tempFile_.empty()) {
        tempFile_ = OUTPUT_FILE_TEMP_PREFIX + boost::algorithm::replace_all_copy(outputFile_, "/", "!");

        std::ofstream ofs(outputFile_);
        ofs << OUTPUT_FILE_SHEBANG << std::endl
            << "cat " << tempFile_ << std::endl;
        chmod(outputFile_.c_str(), S_IRWXU|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH);
    }

    std::ofstream ofs(tempFile_);
    ofs << buildScript();
}

std::string batteryLine(const std::optional<uint8_t>& battery)
{
    auto s = fmt::format(
        "{} {}",
        HEADPHONES_EMOJI,
        battery ? fmt::format("{}% ", *battery) : "");
    s += '\n';
    return s;
}

std::string buildBashOptionLine(const std::string& name, const std::string& icon, const std::string& cmd)
{
    std::string s;
    s.reserve(128); // NOLINT
    s += fmt::format("{} | iconName={} ", name, icon);
    s += fmt::format("bash='{}' ", cmd);
    s += "terminal=false\n";
    return s;
}

std::string connectOptionLine(int pid)
{
    return buildBashOptionLine("Connect", "call-start", fmt::format("kill -SIGCONT {}", pid));
}

std::string disconnectOptionLine(int pid)
{
    return buildBashOptionLine("Disconnect", "call-stop", fmt::format("kill -SIGTSTP {}", pid));
}

std::string restartOptionLine(int pid)
{
    return buildBashOptionLine("Reconnect", "view-refresh", fmt::format("kill -SIGHUP {}", pid));
}

std::string ArgosOutput::buildScript() const
{
    auto pid = getpid();

    std::stringstream ss;
    ss << batteryLine(batteryInfo(state_));
    ss << std::endl;
    ss << "---" << std::endl;
    if (!state_.isConnected) {
        ss << connectOptionLine(pid);
    }
    if (state_.isConnected) {
        ss << disconnectOptionLine(pid);
        ss << restartOptionLine(pid);
    }
    return ss.str();
}

} // namespace buds
