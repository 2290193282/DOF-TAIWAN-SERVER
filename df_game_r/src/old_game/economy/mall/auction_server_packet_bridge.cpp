#include "old_game/economy/mall/auction_server_packet_bridge.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/network/transport/network_endpoint.h"
#include "old_game/economy/mall/auction_inter_return_payloads.h"
#include "old_game/economy/mall/auction_regist_item_return_abi.h"
#include "old_game/economy/mall/auction_runtime.h"

namespace {

// Auction service frames use the six-byte nsl header, followed by twelve
// service-reserved bytes.  User-routed replies therefore start their UID at
// +0x12.  CNetwork::parse_auction_packet enforces the same 0x12-byte floor.
const size_t kAuctionFramePrefixSize = 0x12u;
const size_t kAuctionRoutingUidOffset = 0x12u;
const size_t kAuctionCharacNoOffset = 0x16u;
const size_t kAuctionMaximumFrameSize = 0x5000u;
const uint8_t kPointAuctionReplyCategory = 19u;

const int kInterAuctionNotifyMail = 239;
const int kInterAuctionResultAskPrice = 240;
const int kInterAuctionResultAskRegistedItemNum = 241;
const int kInterAuctionResultRegist = 242;
const int kInterAuctionResultBidding = 243;
const int kInterAuctionResultRegistCancel = 244;
const int kInterAuctionResultItemList = 245;
const int kInterAuctionResultMyRegistedItems = 246;
const int kInterAuctionResultMyBidding = 247;
const int kInterAuctionLogMessage = 248;
const int kInterAuctionNotifyService = 249;
const int kInterPointAuctionLogMessage = 250;
const int kInterAuctionResultBuyItemApiece = 525;
const int kInterAuctionOwnerIsVip = 730;

const size_t kAuctionItemListPrefixSize = 0x21u;
const size_t kAuctionItemListRecordSize = 137u;
const size_t kAuctionMyRegistedPrefixSize = 0x1cu;
const size_t kAuctionMyRegistedRecordSize = 117u;
const size_t kAuctionMyBiddingPrefixSize = 0x1du;
const size_t kAuctionMyBiddingRecordSize = 125u;

#pragma pack(push, 1)
struct AuctionBiddingRelayPayload {
    AuctionResultBiddingPayload result;  // +0x00..+0x0d
    uint32_t relay_value_0;              // +0x0e
    uint32_t relay_value_1;              // +0x12
};

struct PointAuctionBiddingRelayPayload {
    AuctionBiddingRelayPayload common;   // +0x00..+0x15
    char billing_text[32];               // +0x16
    uint32_t billing_context;            // +0x36
};
#pragma pack(pop)

static_assert(sizeof(AuctionBiddingRelayPayload) == 0x16u,
              "gold auction bidding Inter payload must stay 0x16 bytes");
static_assert(sizeof(PointAuctionBiddingRelayPayload) == 0x3au,
              "point auction bidding Inter payload must stay 0x3A bytes");

uint16_t ReadU16(const char* packet, size_t offset) {
    uint16_t value = 0;
    std::memcpy(&value, packet + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* packet, size_t offset) {
    uint32_t value = 0;
    std::memcpy(&value, packet + offset, sizeof(value));
    return value;
}

int32_t ReadI32(const char* packet, size_t offset) {
    int32_t value = 0;
    std::memcpy(&value, packet + offset, sizeof(value));
    return value;
}

Stream* AcquireDispatchStream(size_t payload_size) {
    const size_t queued_size = payload_size + 8u;
    if (queued_size <= 0x0fffu) {
        SmallStreamPool* const pool = GlobalData::small_stream_pool();
        return pool ? pool->Acquire("AuctionServerPacketBridge.cpp", 110) : 0;
    }
    if (queued_size <= 0x3fffu) {
        StreamPool* const pool = GlobalData::stream_pool();
        return pool ? pool->Acquire("AuctionServerPacketBridge.cpp", 114) : 0;
    }
    BigStreamPool* const pool = GlobalData::big_stream_pool();
    return pool ? pool->Acquire("AuctionServerPacketBridge.cpp", 117) : 0;
}

bool QueuePayload(int inter_signal,
                  int routing_uid,
                  const void* payload,
                  size_t payload_size) {
    MsgQueueMgr* const queue = GlobalData::msgq_mgr();
    if (!queue || (payload_size && !payload)) {
        return false;
    }

    CStreamGuard guard(AcquireDispatchStream(payload_size), true);
    if (!guard.Get()) {
        return false;
    }

    (*guard) << inter_signal;
    (*guard) << routing_uid;
    if (payload_size) {
        guard->put_binary(const_cast<void*>(payload),
                          static_cast<int>(payload_size));
    }
    if (inter_signal == kInterAuctionNotifyMail) {
        LogManager::logFormat(
            1,
            "AuctionServerPacketBridge.cpp",
            "QueuePayload",
            0,
            "[AUCTION_MAIL_DIAG][BRIDGE_QUEUE] signal=%d payload_size=%u",
            inter_signal,
            static_cast<unsigned int>(payload_size));
    }
    queue->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

bool QueueServiceState(uint32_t service_type, bool running) {
    SIG_AUCTION_NOTIFY_AUCTION_SERVICE payload = {};
    payload.service_type = static_cast<int32_t>(service_type);
    payload.running = running ? 1u : 0u;
    return QueuePayload(kInterAuctionNotifyService, -1,
                        &payload, sizeof(payload));
}

void ScheduleAuctionReadyCheck() {
    // RegistNextTimer does not consume object state, but constructing the
    // concrete TimerSig keeps the restored C++ call well-defined.
    Timer_CheckAuctionReady timer;
    timer.RegistNextTimer();
}

bool ValidateVariableFrame(size_t packet_size,
                           size_t prefix_size,
                           size_t count,
                           size_t record_size,
                           size_t* records_size) {
    if (!records_size || prefix_size > kAuctionMaximumFrameSize ||
        count > (kAuctionMaximumFrameSize - prefix_size) / record_size) {
        return false;
    }
    *records_size = count * record_size;
    return packet_size == prefix_size + *records_size;
}

bool QueueItemList(const char* packet, size_t packet_size) {
    if (packet_size < kAuctionItemListPrefixSize) {
        return false;
    }
    const uint16_t count = ReadU16(packet, 0x1fu);
    size_t records_size = 0;
    if (!ValidateVariableFrame(packet_size,
                               kAuctionItemListPrefixSize,
                               count,
                               kAuctionItemListRecordSize,
                               &records_size)) {
        return false;
    }

    const size_t payload_size = 4u + 1u + 4u + 2u + records_size;
    MsgQueueMgr* const queue = GlobalData::msgq_mgr();
    CStreamGuard guard(AcquireDispatchStream(payload_size), true);
    if (!queue || !guard.Get()) {
        return false;
    }
    (*guard) << kInterAuctionResultItemList;
    (*guard) << ReadI32(packet, kAuctionRoutingUidOffset);
    (*guard) << ReadI32(packet, kAuctionCharacNoOffset);
    (*guard) << static_cast<uint8_t>(packet[0x1a]);
    (*guard) << ReadU32(packet, 0x1bu);
    (*guard) << count;
    if (records_size) {
        guard->put_binary(const_cast<char*>(packet + kAuctionItemListPrefixSize),
                          static_cast<int>(records_size));
    }
    queue->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

bool QueueMyRegistedItems(const char* packet, size_t packet_size) {
    if (packet_size < kAuctionMyRegistedPrefixSize) {
        return false;
    }
    const uint8_t count = static_cast<uint8_t>(packet[0x1b]);
    size_t records_size = 0;
    if (!ValidateVariableFrame(packet_size,
                               kAuctionMyRegistedPrefixSize,
                               count,
                               kAuctionMyRegistedRecordSize,
                               &records_size)) {
        return false;
    }

    const size_t payload_size = 4u + 1u + 1u + records_size;
    MsgQueueMgr* const queue = GlobalData::msgq_mgr();
    CStreamGuard guard(AcquireDispatchStream(payload_size), true);
    if (!queue || !guard.Get()) {
        return false;
    }
    (*guard) << kInterAuctionResultMyRegistedItems;
    (*guard) << ReadI32(packet, kAuctionRoutingUidOffset);
    (*guard) << ReadI32(packet, kAuctionCharacNoOffset);
    (*guard) << static_cast<uint8_t>(packet[0x1a]);
    (*guard) << count;
    if (records_size) {
        guard->put_binary(
            const_cast<char*>(packet + kAuctionMyRegistedPrefixSize),
            static_cast<int>(records_size));
    }
    queue->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

bool QueueMyBidding(const char* packet, size_t packet_size) {
    if (packet_size < kAuctionMyBiddingPrefixSize) {
        return false;
    }
    const uint16_t count = ReadU16(packet, 0x1bu);
    size_t records_size = 0;
    if (!ValidateVariableFrame(packet_size,
                               kAuctionMyBiddingPrefixSize,
                               count,
                               kAuctionMyBiddingRecordSize,
                               &records_size)) {
        return false;
    }

    const size_t payload_size = 4u + 1u + 2u + records_size;
    MsgQueueMgr* const queue = GlobalData::msgq_mgr();
    CStreamGuard guard(AcquireDispatchStream(payload_size), true);
    if (!queue || !guard.Get()) {
        return false;
    }
    (*guard) << kInterAuctionResultMyBidding;
    (*guard) << ReadI32(packet, kAuctionRoutingUidOffset);
    (*guard) << ReadI32(packet, kAuctionCharacNoOffset);
    (*guard) << static_cast<uint8_t>(packet[0x1a]);
    (*guard) << count;
    if (records_size) {
        guard->put_binary(const_cast<char*>(packet + kAuctionMyBiddingPrefixSize),
                          static_cast<int>(records_size));
    }
    queue->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

bool DispatchFixedAuctionReply(const char* packet,
                               size_t packet_size,
                               uint8_t packet_id,
                               bool point_auction) {
    const uint32_t payment_type = point_auction ? 1u : 0u;
    const uint32_t service_type = point_auction ? 1u : 0u;

    switch (packet_id) {
        case 0: {
            if (packet_size < 0x12u) {
                return false;
            }
            const uint32_t result = ReadU32(packet, 0x0eu);
            if (result == 0u) {
                return QueueServiceState(service_type, true);
            }
            if (result == 2u) {
                ScheduleAuctionReadyCheck();
            } else {
                LogManager::logFormat(
                    1,
                    "AuctionServerPacketBridge.cpp",
                    "DispatchAuctionServerPacket",
                    317,
                    "AUCTION REGIST ERROR service=%u result=%u",
                    service_type,
                    result);
            }
            return true;
        }
        case 1:
            return QueueServiceState(service_type, false);

        case 2: {
            const size_t minimum_size = point_auction ? 0x1eu : 0x36u;
            if (packet_size < minimum_size) {
                return false;
            }
            AuctionResultAskPricePayload payload = {};
            payload.payment_type = payment_type;
            payload.charac_no = ReadI32(packet, 0x16u);
            payload.representative_price = ReadI32(packet, 0x1au);
            if (!point_auction) {
                for (size_t index = 0; index < 3u; ++index) {
                    payload.first_price_values[index] =
                        ReadI32(packet, 0x1eu + index * 4u);
                    payload.second_price_values[index] =
                        ReadI32(packet, 0x2au + index * 4u);
                }
            }
            return QueuePayload(kInterAuctionResultAskPrice,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                &payload,
                                sizeof(payload));
        }

        case 3: {
            const size_t minimum_size = point_auction ? 0x2fu : 0x3fu;
            if (packet_size < minimum_size) {
                return false;
            }
            AuctionResultAskRegistedItemNumPayload payload = {};
            payload.payment_type = payment_type;
            payload.charac_no = ReadI32(packet, 0x16u);
            payload.registed_item_count =
                static_cast<int16_t>(ReadU16(packet, 0x1au));
            payload.item_space = static_cast<uint8_t>(packet[0x1c]);
            payload.item_slot = ReadU16(packet, 0x1du);
            payload.item_index = ReadU32(packet, 0x1fu);
            payload.item_count = ReadI32(packet, 0x23u);
            payload.field_15 = ReadI32(packet, 0x27u);
            payload.field_19 = ReadI32(packet, 0x2bu);
            if (!point_auction) {
                payload.field_1d = ReadI32(packet, 0x2fu);
                std::memcpy(&payload.category, packet + 0x33u,
                            sizeof(payload.category));
            }
            return QueuePayload(kInterAuctionResultAskRegistedItemNum,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                &payload,
                                sizeof(payload));
        }

        case 4:
        case 6: {
            if (packet_size < 0x1cu) {
                return false;
            }
            AuctionSimpleResultPayload payload = {};
            payload.payment_type = payment_type;
            payload.charac_no = ReadI32(packet, 0x16u);
            payload.result = static_cast<uint8_t>(packet[0x1a]);
            payload.error_code = static_cast<uint8_t>(packet[0x1b]);
            const int inter_signal = packet_id == 4
                                         ? kInterAuctionResultRegist
                                         : kInterAuctionResultRegistCancel;
            return QueuePayload(inter_signal,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                &payload,
                                sizeof(payload));
        }

        case 5: {
            const size_t minimum_size = point_auction ? 0x4cu : 0x28u;
            if (packet_size < minimum_size) {
                return false;
            }
            PointAuctionBiddingRelayPayload payload = {};
            payload.common.result.payment_type = payment_type;
            payload.common.result.charac_no = ReadI32(packet, 0x16u);
            payload.common.result.requested_money = ReadI32(packet, 0x1au);
            payload.common.result.result = static_cast<uint8_t>(packet[0x1e]);
            payload.common.result.error_code =
                static_cast<uint8_t>(packet[0x1f]);
            payload.common.relay_value_0 = ReadU32(packet, 0x20u);
            payload.common.relay_value_1 = ReadU32(packet, 0x24u);
            size_t payload_size = sizeof(payload.common);
            if (point_auction) {
                std::memcpy(payload.billing_text, packet + 0x28u,
                            sizeof(payload.billing_text));
                payload.billing_context = ReadU32(packet, 0x48u);
                payload_size = sizeof(payload);
            }
            return QueuePayload(kInterAuctionResultBidding,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                &payload,
                                payload_size);
        }

        case 11: {
            const size_t payload_size = point_auction
                                            ? sizeof(SIG_AUCTION_LOG_MESSAGE_PG)
                                            : sizeof(SIG_AUCTION_LOG_MESSAGE_AG);
            if (packet_size < 0x16u + payload_size) {
                return false;
            }
            return QueuePayload(point_auction
                                    ? kInterPointAuctionLogMessage
                                    : kInterAuctionLogMessage,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                packet + 0x16u,
                                payload_size);
        }

        case 12:
            return QueueServiceState(service_type, true);

        case 13:
            if (packet_size < 0x13u) {
                return false;
            }
            if (packet[0x12]) {
                return QueueServiceState(service_type, true);
            }
            ScheduleAuctionReadyCheck();
            return true;

        case 14: {
            if (!point_auction) {
                if (packet_size < 0x28u) {
                    return false;
                }
                SIG_AUCTION_RESULT_BUY_ITEM_APIECE payload = {};
                payload.charac_no = ReadI32(packet, 0x16u);
                payload.refund_money = ReadU32(packet, 0x1au);
                payload.result = static_cast<uint8_t>(packet[0x1e]);
                payload.response_detail = static_cast<uint8_t>(packet[0x1f]);
                std::memcpy(payload.relay_reserved_0a, packet + 0x20u,
                            sizeof(payload.relay_reserved_0a));
                return QueuePayload(kInterAuctionResultBuyItemApiece,
                                    ReadI32(packet, kAuctionRoutingUidOffset),
                                    &payload,
                                    sizeof(payload));
            }

            if (packet_size < 0x3cu) {
                return false;
            }
            SIG_AUCTION_OWNER_IS_VIP payload = {};
            payload.user_uid = ReadI32(packet, 0x12u);
            payload.auction_key_low = ReadU32(packet, 0x16u);
            payload.auction_key_high = ReadU32(packet, 0x1au);
            payload.is_vip = static_cast<uint8_t>(packet[0x1e]);
            payload.tax_divisor = ReadI32(packet, 0x1fu);
            // The original relay copies the complete contiguous 17-byte
            // name/reserved region from service +0x23 to Inter +0x11.
            std::memcpy(payload.character_name,
                        packet + 0x23u,
                        sizeof(payload.character_name) +
                            sizeof(payload.relay_reserved_1e));
            payload.bid_cera = ReadI32(packet, 0x34u);
            payload.billing_context = ReadI32(packet, 0x38u);
            return QueuePayload(kInterAuctionOwnerIsVip,
                                ReadI32(packet, kAuctionRoutingUidOffset),
                                &payload,
                                sizeof(payload));
        }

        default:
            return false;
    }
}

void DispatchAuctionServerPacketCallback(const char* packet, int packet_size) {
    (void)DispatchAuctionServerPacket(packet, packet_size);
}

}  // namespace

bool DispatchAuctionServerPacket(const char* packet, int packet_size) {
    if (!packet || packet_size < static_cast<int>(kAuctionFramePrefixSize) ||
        packet_size > static_cast<int>(kAuctionMaximumFrameSize)) {
        return false;
    }

    const size_t frame_size = static_cast<size_t>(packet_size);
    if (ReadU32(packet, 2u) != frame_size) {
        return false;
    }

    const uint8_t packet_id = static_cast<uint8_t>(packet[1]);
    if (packet_id == 10u) {
        LogManager::logFormat(
            1,
            "AuctionServerPacketBridge.cpp",
            "DispatchAuctionServerPacket",
            0,
            "[AUCTION_MAIL_DIAG][BRIDGE_ID10_UNCLAIMED] category=%u "
            "size=%d auction_mail_route=monitor_opcode_3100_signal_%d",
            static_cast<unsigned int>(static_cast<uint8_t>(packet[0])),
            packet_size,
            kInterAuctionNotifyMail);
    }
    if (packet_id == 7u) {
        return QueueItemList(packet, frame_size);
    }
    if (packet_id == 8u) {
        return QueueMyRegistedItems(packet, frame_size);
    }
    if (packet_id == 9u) {
        return QueueMyBidding(packet, frame_size);
    }

    const bool point_auction =
        static_cast<uint8_t>(packet[0]) == kPointAuctionReplyCategory;
    const bool dispatched = DispatchFixedAuctionReply(
        packet, frame_size, packet_id, point_auction);
    if (!dispatched) {
        LogManager::logFormat(
            1,
            "AuctionServerPacketBridge.cpp",
            "DispatchAuctionServerPacket",
            515,
            "[AUCTION PACKET ERROR] category=%u id=%u size=%d",
            static_cast<unsigned int>(static_cast<uint8_t>(packet[0])),
            static_cast<unsigned int>(packet_id),
            packet_size);
    }
    return dispatched;
}

void InstallAuctionServerPacketBridge() {
    SetAuctionPacketDispatch(&DispatchAuctionServerPacketCallback);
}
