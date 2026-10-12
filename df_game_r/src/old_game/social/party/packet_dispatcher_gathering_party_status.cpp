#include "old_game/social/party/packet_dispatcher_gathering_party_status.h"

#include "common/manager/global_tools.h"
#include <string.h>

#include "common/manager/global_data.h"
#include "infra/log/format/log_manager.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_GatheringPartyStatus::dispatch_sig(CUser*, PacketBuf&)";
const char kSourceFile[] = "PacketDispatcher_Impl_1.cpp";

enum {
    kMaxTryResults = 30,
    kPvpPartyMemberMax = 8,
    kPartyMemberMax = 4,
};

}  // namespace

// IDA_EA: 0x8203878
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GatheringPartyStatus::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GatheringPartyStatus::dispatch_sig(CUser* user,
                                                   PacketBuf& packet) {
    uint8_t packet_status;
    uint32_t map_id;
    uint32_t dungeon_id;
    uint8_t connected_to_relay;
    uint8_t relay_connected;
    uint32_t relay_connected_time;
    uint16_t try_count;
    SIG_PARTY_STATUS::PartyResult results[kMaxTryResults];
    memset(results, 0, sizeof(results));
    if (packet.get_byte(packet_status) != 1) {
        return LineFunc(39659, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(map_id) != 1) {
        return LineFunc(39663, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(dungeon_id) != 1) {
        return LineFunc(39666, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(connected_to_relay) != 1) {
        return LineFunc(39668, kDispatchSignature, 0, 0);
    }
    if (packet.get_byte(relay_connected) != 1) {
        return LineFunc(39670, kDispatchSignature, 0, 0);
    }
    if (packet.get_int(relay_connected_time) != 1) {
        return LineFunc(39672, kDispatchSignature, 0, 0);
    }
    if (packet.get_short(try_count) != 1) {
        return LineFunc(39674, kDispatchSignature, 0, 0);
    }
    // The first client byte is consumed at 0x8203898 but is not copied into
    // SIG_PARTY_STATUS by the original dispatcher.
    (void)packet_status;
    if (try_count > kMaxTryResults) {
        LogManager::logFormat(1,
                              kSourceFile,
                              kDispatchSignature,
                              39679,
                              "recv_pk_party_status.try_num > %d\n",
                              kMaxTryResults);
        return LineFunc(39680, kDispatchSignature, 0, 0);
    }

    for (uint16_t i = 0; i < try_count; ++i) {
        SIG_PARTY_STATUS::PartyResult& result = results[i];
        if (packet.get_byte(result.party_result) != 1) {
            return LineFunc(39689, kDispatchSignature, 0, 0);
        }
        if (packet.get_byte(result.peer_nat_type) != 1) {
            return LineFunc(39690, kDispatchSignature, 0, 0);
        }
        if (packet.get_binary(reinterpret_cast<char*>(result.peer_ip), 4) != 1) {
            return LineFunc(39691, kDispatchSignature, 0, 0);
        }
        if (packet.get_int(result.established_time) != 1) {
            return LineFunc(39692, kDispatchSignature, 0, 0);
        }
    }

    uint16_t party_count;
    if (packet.get_short(party_count) != 1) {
        return LineFunc(39698, kDispatchSignature, 0, 0);
    }
    if (G_GameWorld()->IsPVPChannel()) {
        if (party_count > kPvpPartyMemberMax) {
            LogManager::logFormat(
                1,
                kSourceFile,
                kDispatchSignature,
                39708,
                "recv_pk_party_status.party_num > MAX_PVP_MEM\n");
            return LineFunc(39709, kDispatchSignature, 0, 0);
        }
    } else if (party_count > kPartyMemberMax) {
        LogManager::logFormat(1,
                              kSourceFile,
                              kDispatchSignature,
                              39716,
                              "recv_pk_party_status.party_num > PARTY_PLAYER_MAX\n");
        return LineFunc(39717, kDispatchSignature, 0, 0);
    }

    SIG_PARTY_STATUS::PartyPing pings[kPvpPartyMemberMax];
    memset(pings, 0, sizeof(pings));
    for (uint16_t i = 0; i < party_count; ++i) {
        SIG_PARTY_STATUS::PartyPing& ping = pings[i];
        if (packet.get_byte(ping.nat_type) != 1) {
            return LineFunc(39726, kDispatchSignature, 0, 0);
        }
        if (packet.get_int(ping.ping) != 1) {
            return LineFunc(39734, kDispatchSignature, 0, 0);
        }
        if (packet.get_int(ping.mtu) != 1) {
            return LineFunc(39742, kDispatchSignature, 0, 0);
        }
        if (packet.get_binary(reinterpret_cast<char*>(ping.ip), 4) != 1) {
            return LineFunc(39751, kDispatchSignature, 0, 0);
        }
    }

    CStreamGuard stream_guard(
        GlobalData::s_stream_pool->Acquire(kSourceFile, 39758), true);
    *stream_guard << 142;
    *stream_guard << -1;
    SIG_PARTY_STATUS* const output =
        stream_guard.GetInBuffer<SIG_PARTY_STATUS>();
    memset(output, 0, sizeof(*output));
    user->network_.GetPeerIP2(output->source_ip, 16);
    output->member_id = static_cast<uint32_t>(user->get_acc_id());
    output->nat_type =
        static_cast<uint32_t>(user->network_.get_nat_type());
    output->map_id = map_id;
    output->dungeon_id = dungeon_id;
    output->end_reason = connected_to_relay;
    output->is_connected_to_relay = relay_connected;
    output->connected_time_to_relay = relay_connected_time;
    output->try_count = try_count;
    memcpy(output->results,
           results,
           sizeof(SIG_PARTY_STATUS::PartyResult) * try_count);
    output->party_count = party_count;
    memcpy(output->pings,
           pings,
           sizeof(SIG_PARTY_STATUS::PartyPing) * party_count);
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::LOGDB_Q, stream_guard);
    return 0;
}

// IDA_EA: 0x8233724
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GatheringPartyStatus::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GatheringPartyStatus::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x823372E
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatus7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GatheringPartyStatus::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GatheringPartyStatus::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x8242D66
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatusD2Ev
// IDA_EA: 0x8242D96
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatusD0Ev
// DEMANGLED: Dispatcher_GatheringPartyStatus::~Dispatcher_GatheringPartyStatus()
Dispatcher_GatheringPartyStatus::~Dispatcher_GatheringPartyStatus() {}

// IDA_EA: 0x8595F74
// IDA_NAME: _ZN31Dispatcher_GatheringPartyStatusC2Ev
// DEMANGLED: Dispatcher_GatheringPartyStatus::Dispatcher_GatheringPartyStatus(void)
Dispatcher_GatheringPartyStatus::Dispatcher_GatheringPartyStatus()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
