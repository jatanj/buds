#include "log.h"

namespace buds {
namespace log {

static constexpr auto DEFAULT_NAME = "logger";
static constexpr auto DEFAULT_PATTERN = "%H:%M:%S.%e %z %^%l%$ [%t] %v";

std::shared_ptr<spdlog::logger> createLogger()
{
    auto stderrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>(DEFAULT_NAME, stderrSink);
    logger->set_level(spdlog::level::debug);
    logger->set_pattern(DEFAULT_PATTERN);
    return logger;
}

spdlog::logger& logger()
{
    static auto logger = createLogger();
    return *logger;
}

} // namespace log
} // namespace buds

