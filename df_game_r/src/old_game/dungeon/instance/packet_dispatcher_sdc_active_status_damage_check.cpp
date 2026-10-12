#include "old_game/dungeon/instance/packet_dispatcher_sdc_active_status_damage_check.h"

#include <stdint.h>
#include <string.h>
#include <zlib.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/creature/active_status_sync_server.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_SDC_ActiveStatus_Damage_Check::dispatch_sig(CUser*, PacketBuf&)";

const int kCompressedBufferSize = 500;
const unsigned int kSDCActiveStatusHackType = 0x25du;

uint64_t StableActiveStatusFingerprint(const uint8_t* bytes,
                                       uLongf length,
                                       uint64_t domain) {
    uint64_t hash = UINT64_C(1469598103934665603);
    for (unsigned int shift = 0; shift < 64u; shift += 8u) {
        hash ^= static_cast<uint8_t>(domain >> shift);
        hash *= UINT64_C(1099511628211);
    }
    for (uLongf index = 0; index < length; ++index) {
        hash ^= bytes[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash == 0u ? UINT64_C(1) : hash;
}

}  // namespace

// IDA_EA: 0x81EA16E
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_Check12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SDC_ActiveStatus_Damage_Check::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SDC_ActiveStatus_Damage_Check::dispatch_sig(CUser* user,
                                                           PacketBuf& packet) {
    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(27446, kDispatchSignature, 0, 0);
    }

    uint8_t uncompressed[kCompressedBufferSize] = {};
    uint8_t compressed[kCompressedBufferSize] = {};
    for (uint8_t index = 0; index < count; ++index) {
        memset(uncompressed, 0, sizeof(uncompressed));
        memset(compressed, 0, sizeof(compressed));
        uLongf uncompressed_size = kCompressedBufferSize;
        int compressed_size = 0;
        if (packet.get_int(compressed_size) != 1) {
            return LineFunc(27466, kDispatchSignature, 0, 0);
        }
        if (compressed_size <= 0 || compressed_size > kCompressedBufferSize) {
            return LineFunc(27469, kDispatchSignature, 0, 0);
        }
        if (packet.get_binary(reinterpret_cast<char*>(compressed),
                              compressed_size) != 1) {
            return LineFunc(27472, kDispatchSignature, 0, 0);
        }
        if (::uncompress(uncompressed,
                         &uncompressed_size,
                         compressed,
                         static_cast<uLong>(compressed_size)) != Z_OK) {
            return LineFunc(27481, kDispatchSignature, 0, 0);
        }
        if (uncompressed_size < sizeof(STSDCActiveStatusSyncDataSend)) {
            return LineFunc(27482, kDispatchSignature, 0, 0);
        }

        const uint64_t request_fingerprint = StableActiveStatusFingerprint(
            uncompressed,
            uncompressed_size,
            UINT64_C(0x5344435f434d4432));
        STSDCActiveStatusSyncDataSend payload;
        memcpy(&payload, uncompressed, sizeof(payload));

        const int client_damage = payload.damage_value;
        const int client_duration = payload.duration;
        int expected_duration = 0;
        int expected_damage = 0;
        SDCActiveStatusSyncServer active_status_calculator;
        active_status_calculator.getActiveStatusDamageDurationForSDC(
            payload.active_status_type,
            payload.active_status_level,
            &expected_duration,
            &expected_damage,
            &payload,
            0);

        const bool accepted = expected_damage == client_damage &&
                              expected_duration == client_duration;
        if (!accepted) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(kSDCActiveStatusHackType),
                1,
                0,
                0);
            cMyTrace trace(kDispatchSignature, 27501, 0);
            trace("[TYPE : %d][EVENT : %d] [Svr] Damage : %d, Duration : %d, [Client] Damage : %d, Duration : %d",
                  static_cast<int>(payload.active_status_type),
                  static_cast<int>(payload.raw_00),
                  expected_damage,
                  expected_duration,
                  client_damage,
                  client_duration);
        }

        CParty* const party =
            G_CGameManager()->GetParty(user->GetPartyIndex());
        if (party) {
            const int damage_value = payload.active_status_type == 11
                                         ? static_cast<int>(
                                               static_cast<float>(client_damage) * 1.5f)
                                         : client_damage;
            const int authoritative_damage = payload.active_status_type == 11
                ? static_cast<int>(static_cast<float>(expected_damage) * 1.5f)
                : expected_damage;
            if (accepted && authoritative_damage > 0 &&
                payload.monster_unique_id > 0 &&
                payload.monster_unique_id <= 0xffff) {
                (void)golden_commerce::ObserveValidatedCartDamage(
                    party,
                    static_cast<uint16_t>(payload.monster_unique_id),
                    static_cast<uint32_t>(authoritative_damage),
                    request_fingerprint);
            }
            party->battle_field().SetDamagedPacketCnt(payload.monster_unique_id);
            party->battle_field().SetDamageValue(payload.monster_unique_id,
                                                 damage_value);
        }
    }
    return 0;
}

// IDA_EA: 0x8233BAA
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_Check4readER9PacketBufR8MSG_BASE
int Dispatcher_SDC_ActiveStatus_Damage_Check::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233BB4
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_Check7processEP5CUserR8MSG_BASER9ParamBase
int Dispatcher_SDC_ActiveStatus_Damage_Check::process(CUser*,
                                                       MSG_BASE&,
                                                       ParamBase&) {
    return 0;
}

// IDA_EA: 0x8243996
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_CheckD2Ev
// DEMANGLED: Dispatcher_SDC_ActiveStatus_Damage_Check::~Dispatcher_SDC_ActiveStatus_Damage_Check()
// IDA_EA: 0x82439C6
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_CheckD0Ev
// DEMANGLED: Dispatcher_SDC_ActiveStatus_Damage_Check::~Dispatcher_SDC_ActiveStatus_Damage_Check()
Dispatcher_SDC_ActiveStatus_Damage_Check::~Dispatcher_SDC_ActiveStatus_Damage_Check() {}

// IDA_EA: 0x8596674
// IDA_NAME: _ZN40Dispatcher_SDC_ActiveStatus_Damage_CheckC2Ev
Dispatcher_SDC_ActiveStatus_Damage_Check::Dispatcher_SDC_ActiveStatus_Damage_Check()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
