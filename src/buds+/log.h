#pragma once

#include <string>
#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define LOG_TRACE buds::log::logger().trace
#define LOG_DEBUG buds::log::logger().debug
#define LOG_INFO buds::log::logger().info
#define LOG_WARN buds::log::logger().warn
#define LOG_ERROR buds::log::logger().error
#define LOG_CRITICAL buds::log::logger().critical

namespace fmt {

#define FMT_FORMATTER(type, ...)                \
    template <>                                 \
    struct formatter<type> {                    \
        auto parse(format_parse_context& ctx) { \
            return ctx.begin();                 \
        }                                                    \
        template <typename FormatContext>                    \
        auto format(const type& value, FormatContext& ctx) { \
            return format_to(ctx.out(), __VA_ARGS__);        \
        }                                                    \
    }


template <typename T>
struct formatter<std::optional<T>> {
    auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const std::optional<T>& opt, FormatContext& ctx) {
        return opt ?
            format_to(ctx.out(), "Some({})", *opt) :
            format_to(ctx.out(), "None");
    }
};

} // namespace fmt

namespace buds::log {

spdlog::logger& logger();

} // namespace buds::log

