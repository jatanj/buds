#include "buds_client.h"

#include <cstdlib>
#include <variant>
#include "message.h"

namespace buds {

int BudsClient::connect()
{
    if (auto error = btClient_.connect(config_.address)) {
        return error;
    }

    readTask_ = std::async(std::launch::async, [this]{
        return btClient_.read([this](auto&& p) {
            read(std::forward<decltype(p)>(p));
        });
    });

    lockTouchpad(config_.lockTouchpad);

    if (config_.mainEarbud) {
        changeMainEarbud(*config_.mainEarbud);
    }

    if (config_.equalizer) {
        changeEqualizerMode(*config_.equalizer);
    }

    configureTouchpadActions(config_.touchpadAction);

    readTask_.wait_for(std::chrono::hours::max());
    return readTask_.get();
}

int BudsClient::close()
{
    return btClient_.close();
}

void BudsClient::lockTouchpad(bool enabled)
{
    LOG_INFO("lockTouchpad({})", enabled);
    write(LockTouchpadMessage{enabled});
}

void BudsClient::changeMainEarbud(MainEarbud earbud)
{
    LOG_INFO("changeMainEarbud({})", static_cast<uint8_t>(earbud));
    write(MainChangeMessage{earbud});
}

void BudsClient::changeEqualizerMode(EqualizerMode mode)
{
    LOG_INFO("changeEqualizerMode({})", static_cast<uint8_t>(mode));
    write(EqualizerMessage{mode});
}

void BudsClient::setTouchpadOption(TouchpadActions actions)
{
    LOG_INFO("setTouchpadOption({}, {})", actions.left, actions.right);
    write(TouchpadOptionMessage{actions});
}

void BudsClient::read(const std::vector<uint8_t>& msg)
{
    MessageParser parser{msg};
    if (auto parsed = parser.parse()) {
        handle(parsed.get());
    }
}

void BudsClient::write(const Message& msg)
{
    btClient_.write(MessageBuilder::build(msg));
}

void BudsClient::handle(Message* msg)
{
    if (auto *m = dynamic_cast<ExtendedStatusUpdatedMessage*>(msg)) {
        handleStatusUpdate(m);
    } else if (auto* m = dynamic_cast<StatusUpdatedMessage*>(msg)) {
        handleStatusUpdate(m);
    } else if (auto* m = dynamic_cast<VersionInfoMessage*>(msg)) {
        if (m->data) {
            LOG_INFO("{}", *m->data);
        }
        write(VersionInfoMessage{});
    } else if (auto* m = dynamic_cast<TouchpadOtherOptionMessage*>(msg)) {
        LOG_INFO("{}", m->data);
        if (auto it = touchpadActions_.find(m->data.action); it != touchpadActions_.end()) {
            it->second();
        }
    }
    if (output_) {
        output_->render();
    }
}

void BudsClient::configureTouchpadActions(const Config::TouchpadActions& actions)
{
    static constexpr uint8_t LEFT_ID = 200;
    static constexpr uint8_t RIGHT_ID = 201;

    static auto configure = [this](const Config::TouchpadAction& action, uint8_t id) -> std::optional<uint8_t> {
        if (const auto* custom = std::get_if<Config::BashAction>(&action)) {
            LOG_INFO("Registering bash touchpad action {} => {}", id, custom->command);
            touchpadActions_.insert_or_assign(id, [c = custom->command]() {
                // FIXME: Avoid using system()
                system(c.c_str()); // NOLINT
            });
            return id;
        }
        if (const auto* predefined = std::get_if<TouchpadPredefinedAction>(&action)) {
            return *predefined;
        }
        return std::nullopt;
    };

    TouchpadActions request{};

    if (actions.left) {
        if (auto id = configure(*actions.left, LEFT_ID)) {
            request.left = *id;
        }
    }
    if (actions.right) {
        if (auto id = configure(*actions.right, RIGHT_ID)) {
            request.right = *id;
        }
    }

    setTouchpadOption(request);
}

} // namespace buds
