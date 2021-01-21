#include "message_parser.h"

#include "constants.h"
#include "log.h"
#include "message.h"

namespace buds {

std::unique_ptr<buds::Message> MessageParser::parse()
{
    auto* preamble = next(SIZE_PREAMBLE);
    if (!preamble || *preamble != MSG_PREAMBLE) {
        LOG_ERROR("Invalid preamble");
    }
    auto* header = next(SIZE_HEADER);
    if (!header) {
        LOG_ERROR("Header not found");
        return {};
    }
    auto* id = next(SIZE_MSG_ID);
    if (!id) {
        LOG_ERROR("Message ID not found");
        return {};
    }
    switch (*id++) {
        case MSG_ID_EXTENDED_STATUS_UPDATED:
            return parseMessage<ExtendedStatusUpdatedMessage, ExtendedStatusUpdatedData>(id);
        case MSG_ID_STATUS_UPDATED:
            return parseMessage<StatusUpdatedMessage, StatusUpdatedData>(id);
        case MSG_ID_VERSION_INFO:
            return parseMessage<VersionInfoMessage, VersionInfoData>(id);
        case MSG_ID_SET_TOUCHPAD_OTHER_OPTION:
            return parseMessage<TouchpadOtherOptionMessage, TouchpadOtherOptionData>(id);
        default:
            break;
    }
    // TODO: Verify checksum and postamble
    return {};
}

const uint8_t* MessageParser::next(size_t n)
{
    if (msg_.size() >= index_ + n) {
        auto* p = msg_.data() + index_;
        index_ += n;
        return p;
    }
    return nullptr;
}

} // namespace buds
