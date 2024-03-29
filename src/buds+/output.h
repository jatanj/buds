#pragma once

#include "message.h"

namespace buds
{

struct BudsState {
    bool isConnected;
};

class Output {
public:
    virtual ~Output() = default;

    virtual void update(const BudsState& state) = 0;

    virtual void update(const StatusUpdatedData& data) = 0;

    virtual void update(const ExtendedStatusUpdatedData& data) = 0;

    virtual void render() = 0;
};

} // namespace buds
