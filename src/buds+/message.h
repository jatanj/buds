#pragma once

#include <bits/stdint-uintn.h>
#include <stdexcept>
#include <cstdint>
#include <sys/types.h>
#include <vector>
#include <optional>
#include <fmt/format.h>

#include "constants.h"
#include "log.h"
#include "util.h"

namespace buds {

using namespace constants;

class Message {
public:
    Message() = default;

    virtual ~Message() = default;

    virtual std::vector<uint8_t> payload() const = 0;

    virtual uint8_t id() const = 0;

    virtual bool isResponse() const = 0;
};

template <uint8_t Id, bool IsResponse = false>
class MessageT : public Message {
public:
    ~MessageT() override = default;

    std::vector<uint8_t> payload() const override
    {
        if constexpr (IsResponse) {
            throw std::runtime_error("Attempted to call payload on response message");
        } else {
            throw std::runtime_error("Payload is not implemented");
        }
    }

    uint8_t id() const override { return Id; }

    bool isResponse() const override { return IsResponse; }
};

struct LockTouchpadMessage : MessageT<MSG_ID_LOCK_TOUCHPAD> {
    bool enabled;

    explicit LockTouchpadMessage(bool enabled) : enabled(enabled) {}

    ~LockTouchpadMessage() override = default;

    std::vector<uint8_t> payload() const override { return {static_cast<uint8_t>(enabled)}; }
};

enum MainEarbud : uint8_t {
    RIGHT = 0,
    LEFT = 1
};

struct MainChangeMessage : MessageT<MSG_ID_MAIN_CHANGE> {
    MainEarbud earbud;

    explicit MainChangeMessage(MainEarbud earbud) : earbud(earbud) {}

    std::vector<uint8_t> payload() const override { return {earbud}; }
};

enum EqualizerMode : uint8_t {
    NORMAL = 0,
    BASS_BOOST = 1,
    SOFT = 2,
    DYNAMIC = 3,
    CLEAR = 4,
    TREBLE_BOOST = 5
};

struct EqualizerMessage : MessageT<MSG_ID_EQUALIZER> {
    EqualizerMode mode;

    explicit EqualizerMessage(EqualizerMode mode) : mode(mode) {}

    std::vector<uint8_t> payload() const override { return {mode}; }
};

enum TouchpadPredefinedAction : uint8_t {
    UNUSED = 0,
    VOICE_ASSISTANT = 1,
    AMBIENT_SOUND = 2,
    VOLUME = 3,
    SPOTIFY = 4
};

struct TouchpadActions {
    uint8_t left = TouchpadPredefinedAction::UNUSED;
    uint8_t right = TouchpadPredefinedAction::UNUSED;
};

struct TouchpadOptionMessage : MessageT<MSG_ID_SET_TOUCHPAD_OPTION> {
    TouchpadActions actions;

    explicit TouchpadOptionMessage(TouchpadActions actions) : actions(actions) {}

    std::vector<uint8_t> payload() const override
    {
        return {actions.left, actions.right};
    }
};

struct TouchpadOtherOptionData {
    uint8_t action;
};

struct TouchpadOtherOptionMessage : MessageT<MSG_ID_SET_TOUCHPAD_OTHER_OPTION> {
    TouchpadOtherOptionData data;

    explicit TouchpadOtherOptionMessage(TouchpadOtherOptionData data) : data(data) {}
};

struct ManagerInfoData {
    enum ClientType : uint8_t {
        WEARABLE_APP = 1
    };

    enum IsSamsungDevice : uint8_t {
        YES = 1,
        NO = 2
    };

    static constexpr auto VERSION_MARSHMALLOW = 23;

    ClientType clientType = ClientType::WEARABLE_APP;
    IsSamsungDevice isSamsungDevice = IsSamsungDevice::NO;
    uint8_t androidSdk = VERSION_MARSHMALLOW;
};

struct ManagerInfoMessage : MessageT<MSG_ID_MANAGER_INFO> {
public:
    const ManagerInfoData data;

    explicit ManagerInfoMessage(ManagerInfoData data)
        : data(data), payload_({data.clientType, data.isSamsungDevice, data.androidSdk}) {}

    ~ManagerInfoMessage() override = default;

    std::vector<uint8_t> payload() const override { return payload_; }

private:
    const std::vector<uint8_t> payload_;
};

struct VersionInfoData {
    std::array<uint8_t, 12> unknown;
};

struct VersionInfoMessage : MessageT<MSG_ID_VERSION_INFO, true> {
public:
    const std::optional<VersionInfoData> data;

    explicit VersionInfoMessage(const std::optional<VersionInfoData>& data = std::nullopt) : data(data) {}

    ~VersionInfoMessage() override = default;

    std::vector<uint8_t> payload() const override
    {
        return {0};
    }
};

struct PlacementParser {
    uint8_t placement;

    enum Placement {
        IN_EAR = 1,
        OUTSIDE_EAR = 2,
        IN_CASE = 3
    };

    Placement left() const
    {
        return static_cast<Placement>(placement >> 4);
    }

    Placement right() const
    {
        return static_cast<Placement>(placement & 0xf);
    }
};

struct ExtendedStatusUpdatedData {
    uint8_t interfaceRevision;
    uint8_t earType;
    uint8_t deviceBatGageL;
    uint8_t deviceBatGageR;
    bool isCoupled;
    uint8_t mainConnection;
    uint8_t placement;
    uint8_t batteryCase;
    bool ambientSoundEnable;
    uint8_t ambientSoundType;
    uint8_t adjustSoundSync;
    uint8_t equalizerMode;
    bool touchLock;
    uint8_t touchOptions;
    uint8_t deviceColor;
    bool sideToneEnable;
    bool extraHighAmbientEnable;
};

struct ExtendedStatusUpdatedMessage : MessageT<MSG_ID_EXTENDED_STATUS_UPDATED, true> {
    const std::optional<ExtendedStatusUpdatedData> data;

    explicit ExtendedStatusUpdatedMessage(const std::optional<ExtendedStatusUpdatedData>& data = std::nullopt) : data(data) {}

    ~ExtendedStatusUpdatedMessage() override = default;

    std::vector<uint8_t> payload() const override
    {
        return {0};
    }
};

struct StatusUpdatedData {
    uint8_t unknown;
    uint8_t deviceBatGageL;
    uint8_t deviceBatGageR;
    uint8_t twsStatus;
    uint8_t mainConnection;
    uint8_t placement;
};

struct StatusUpdatedMessage : MessageT<MSG_ID_STATUS_UPDATED, true> {
    const std::optional<StatusUpdatedData> data;

    explicit StatusUpdatedMessage(const std::optional<StatusUpdatedData>& data = std::nullopt) : data(data) {}

    ~StatusUpdatedMessage() override = default;

    std::vector<uint8_t> payload() const override
    {
        return {0};
    }
};

} // namespace buds

namespace fmt {

FMT_FORMATTER(
    buds::ExtendedStatusUpdatedData,
    "ExtendedStatusUpdateData{{"
    "interfaceRevision={}, earType={}, deviceBatGageL={}, "
    "deviceBatGageR={}, isCoupled={}, "
    "mainConnection={}, placement={}, batteryCase={}, "
    "ambientSoundEnable={}, "
    "ambientSoundType={}, adjustSoundSync={}, equalizerMode={}, "
    "touchLock={}, touchOptions={}, deviceColor={}, "
    "sideToneEnable={}, extraHighAmbientEnable={}}}",
    value.interfaceRevision, value.earType, value.deviceBatGageL,
    value.deviceBatGageR, value.isCoupled, value.mainConnection,
    value.placement, value.batteryCase,
    value.ambientSoundEnable, value.ambientSoundType,
    value.adjustSoundSync, value.equalizerMode, value.touchLock,
    value.touchOptions, value.deviceColor, value.sideToneEnable,
    value.extraHighAmbientEnable);

FMT_FORMATTER(
    buds::StatusUpdatedData,
    "StatusUpdateData{{"
    "deviceBatGageL={}, deviceBatGageR={}, twsStatus={}, "
    "mainConnection={}, placement={}}}",
    value.deviceBatGageL, value.deviceBatGageR, value.twsStatus, value.mainConnection, value.placement);

FMT_FORMATTER(
    buds::VersionInfoData,
    "VersionInfoData{{data={}}}",
    buds::util::toHex(std::vector(value.unknown.begin(), value.unknown.end())));

FMT_FORMATTER(
    buds::TouchpadOtherOptionData,
    "TouchpadOtherActionData{{action={}}}",
    value.action);

} // namespace fmt
