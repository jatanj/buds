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

void ArgosOutput::update(const StatusUpdatedData& data)
{
    const std::lock_guard<std::mutex> lock(updateMutex_);

    info_.leftBattery = data.deviceBatGageL;
    info_.rightBattery = data.deviceBatGageR;

    PlacementParser placement{data.placement};
    info_.leftPlacement = placement.left();
    info_.rightPlacement = placement.right();
}

void ArgosOutput::update(const ExtendedStatusUpdatedData& data)
{
    const std::lock_guard<std::mutex> lock(updateMutex_);

    info_.leftBattery = data.deviceBatGageL;
    info_.rightBattery = data.deviceBatGageR;

    PlacementParser placement{data.placement};
    info_.leftPlacement = placement.left();
    info_.rightPlacement = placement.right();
}

std::optional<uint8_t> ArgosOutput::batteryInfo(const BudsTrayInfo& info)
{
    if (info.leftBattery && info.rightBattery) {
        return std::min(*info.leftBattery, *info.rightBattery);
    }
    if (info.leftBattery) {
        return *info.leftBattery;
    }
    if (info.rightBattery) {
        return *info.rightBattery;
    }
    return std::nullopt;
}

std::string ArgosOutput::wearStatusInfo(const BudsTrayInfo& info)
{
    auto indicator = [](PlacementParser::Placement status) {
        switch (status) {
            case PlacementParser::Placement::IN_EAR: return IN_EAR_EMOJI;
            case PlacementParser::Placement::OUTSIDE_EAR: return OUTSIDE_EAR_EMOJI;
            case PlacementParser::Placement::IN_CASE: return IN_CASE_EMOJI;
            default: return "";
        }
    };
    std::string s;
    if (info.leftPlacement) {
        s += indicator(*info.leftPlacement);
        s += "L";
    }
    if (info.rightPlacement) {
        s += "R";
        s += indicator(*info.rightPlacement);
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

std::string ArgosOutput::buildScript() const
{
    auto battery = batteryInfo(info_);
    auto pid = getpid();

    std::stringstream ss;
    ss << fmt::format(
        "{} {}",
        HEADPHONES_EMOJI,
        battery ? fmt::format("{}% ", *battery) : "")
        << std::endl
        << std::endl;
    ss << "---" << std::endl;
    ss << "Restart | "
       << "iconName=view-refresh "
       << fmt::format("bash='kill -HUP {}' ", pid)
       << "terminal=false"
       << std::endl;
    return ss.str();
}

} // namespace buds
