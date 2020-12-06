#pragma once

#include <string>
#include "log.h"
#include "message.h"
#include <boost/algorithm/string.hpp>

namespace buds
{

enum OutputConfig {
    ARGOS
};

struct Config {
    std::string beforeConnect;
    std::string address;
    std::optional<OutputConfig> outputType;
    std::string outputFile;
    bool lockTouchpad = false;
    std::optional<buds::MainEarbud> mainEarbud;
};

Config parseConfig(const std::string &path);

} // namespace buds

namespace fmt {

FMT_FORMATTER(
    buds::Config,
    "Config{{address={}, lockTouchpad={}, mainEarbud={}}}",
    value.address, value.lockTouchpad, value.mainEarbud);

} // namespace fmt
