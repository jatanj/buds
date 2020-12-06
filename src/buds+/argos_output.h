#pragma once

#include <optional>
#include <string>
#include <utility>

#include "config.h"
#include "output.h"
#include "message.h"

namespace buds {

struct BudsTrayInfo {
    std::optional<uint8_t> leftBattery;
    std::optional<uint8_t> rightBattery;
    std::optional<PlacementParser::Placement> leftPlacement;
    std::optional<PlacementParser::Placement> rightPlacement;
};

class ArgosOutput : public Output {
public:
    explicit ArgosOutput(std::string outputFile) : outputFile_(std::move(outputFile)) {}

    ~ArgosOutput() override = default;

    void update(const StatusUpdatedData& data) override;

    void update(const ExtendedStatusUpdatedData &data) override;

    void render() override;

private:
    const std::string outputFile_;
    std::string tempFile_;
    BudsTrayInfo info_;
    std::mutex updateMutex_;

    std::string buildScript() const;

    static std::optional<uint8_t> batteryInfo(const BudsTrayInfo& info);

    static  std::string wearStatusInfo(const BudsTrayInfo& info);
};

} // namespace buds

namespace fmt {

FMT_FORMATTER(
    buds::BudsTrayInfo,
    "BudsTrayInfo{{leftBattery={}, rightBattery={}, leftPlacement={}, rightPlacement={}}}",
    value.leftBattery ? *value.leftBattery : -1,
    value.rightBattery ? *value.rightBattery : -1,
    value.leftPlacement ? *value.leftPlacement : -1,
    value.rightPlacement ? *value.rightPlacement : -1);

} // namespace fmt
