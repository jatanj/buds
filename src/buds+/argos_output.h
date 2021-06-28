#pragma once

#include <optional>
#include <string>
#include <utility>

#include "config.h"
#include "output.h"
#include "message.h"
#include "buds_client.h"

namespace buds {

struct BudsTrayState {
    std::optional<uint8_t> leftBattery;
    std::optional<uint8_t> rightBattery;
    std::optional<PlacementParser::Placement> leftPlacement;
    std::optional<PlacementParser::Placement> rightPlacement;
    bool isConnected = false;
};

class ArgosOutput : public Output {
public:
    explicit ArgosOutput(std::string outputFile) : outputFile_(std::move(outputFile)) {}

    ~ArgosOutput() override = default;

    void update(const BudsState& state) override;

    void update(const StatusUpdatedData& data) override;

    void update(const ExtendedStatusUpdatedData &data) override;

    void render() override;

private:
    const std::string outputFile_;
    std::string tempFile_;
    BudsTrayState state_;
    std::mutex updateMutex_;

    std::string buildScript() const;

    static std::optional<uint8_t> batteryInfo(const BudsTrayState& state);

    static std::string wearStatusInfo(const BudsTrayState& state);
};

} // namespace buds

namespace fmt {

FMT_FORMATTER(
    buds::BudsTrayState,
    "BudsTrayInfo{{leftBattery={}, rightBattery={}, leftPlacement={}, rightPlacement={}}}",
    value.leftBattery ? *value.leftBattery : -1,
    value.rightBattery ? *value.rightBattery : -1,
    value.leftPlacement ? *value.leftPlacement : -1,
    value.rightPlacement ? *value.rightPlacement : -1);

} // namespace fmt
