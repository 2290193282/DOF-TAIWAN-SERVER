#include "old_game/dungeon/instance/packet_dispatcher_sdc_damage_check.h"

#include <stdint.h>
#include <zlib.h>

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/dungeon/instance/sdc_damage_compute.h"
#include "old_game/dungeon/escort/golden_commerce.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_SDC_Damage_Check::dispatch_sig(CUser*, PacketBuf&)";
const int kSDCBufferSize = 500;
const unsigned int kSDCDamageHackType = 0x25du;

uint64_t StableDamageFingerprint(const uint8_t* bytes,
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

// IDA_EA: 0x81E9EF8..0x81E9F22.  The member order is material: computation
// reads the two floats after the 500-byte decompression target, which are the
// first eight bytes of the separately zeroed/compressed source buffer.
struct SdcDamageBuffers {
    uint8_t uncompressed[kSDCBufferSize];
    uint8_t compressed[kSDCBufferSize];
};

union SdcDamageWorkspace {
    SdcDamageBuffers buffers;
    STComputeDamageVariable calculation;
};

static_assert(offsetof(SdcDamageBuffers, compressed) == 0x1f4u,
              "CMD183 compressed storage must immediately follow payload");

}  // namespace

// IDA_EA: 0x81E9E84
// IDA_NAME: _ZN27Dispatcher_SDC_Damage_Check12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SDC_Damage_Check::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SDC_Damage_Check::dispatch_sig(CUser* user, PacketBuf& packet) {
    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(27317, kDispatchSignature, 0, 0);
    }

    SdcDamageWorkspace workspace = {};
    for (uint8_t index = 0; index < count; ++index) {
        workspace = SdcDamageWorkspace();
        uLongf output_size = kSDCBufferSize;
        int compressed_size = 0;
        if (packet.get_int(compressed_size) != 1) {
            return LineFunc(27337, kDispatchSignature, 0, 0);
        }
        if (compressed_size <= 0 || compressed_size > kSDCBufferSize) {
            return LineFunc(27339, kDispatchSignature, 0, 0);
        }
        if (packet.get_binary(reinterpret_cast<char*>(workspace.buffers.compressed),
                              compressed_size) != 1) {
            return LineFunc(27342, kDispatchSignature, 0, 0);
        }
        if (::uncompress(workspace.buffers.uncompressed,
                         &output_size,
                         workspace.buffers.compressed,
                         static_cast<uLong>(compressed_size)) != Z_OK) {
            return LineFunc(27352, kDispatchSignature, 0, 0);
        }

        const uint64_t request_fingerprint = StableDamageFingerprint(
            workspace.buffers.uncompressed,
            output_size,
            UINT64_C(0x5344435f434d4431));
        STComputeDamageVariable* const variable = &workspace.calculation;
        const int client_damage = variable->computed_damage;
        const int computed_damage = computeHpDamageSDC(variable, 0, 0);
        const bool accepted = !(computed_damage - 1 > client_damage ||
                                computed_damage + 1 < client_damage);
        if (!accepted) {
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(kSDCDamageHackType),
                1,
                0,
                0);
        }

        CParty* const party =
            G_CGameManager()->GetParty(user->GetPartyIndex());
        if (party) {
            if (accepted && computed_damage > 0 &&
                variable->monster_unique_id > 0 &&
                variable->monster_unique_id <= 0xffff) {
                (void)golden_commerce::ObserveValidatedCartDamage(
                    party,
                    static_cast<uint16_t>(variable->monster_unique_id),
                    static_cast<uint32_t>(computed_damage),
                    request_fingerprint);
            }
            party->battle_field().SetDamagedPacketCnt(variable->monster_unique_id);
            party->battle_field().SetDamageValue(variable->monster_unique_id,
                                                 client_damage);
        }
    }
    return 0;
}

// IDA_EA: 0x8233B96
// IDA_NAME: _ZN27Dispatcher_SDC_Damage_Check4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SDC_Damage_Check::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SDC_Damage_Check::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x8233BA0
// IDA_NAME: _ZN27Dispatcher_SDC_Damage_Check7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SDC_Damage_Check::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SDC_Damage_Check::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x82439E4
// IDA_NAME: _ZN27Dispatcher_SDC_Damage_CheckD2Ev
// DEMANGLED: Dispatcher_SDC_Damage_Check::~Dispatcher_SDC_Damage_Check()
// IDA_EA: 0x8243A14
// IDA_NAME: _ZN27Dispatcher_SDC_Damage_CheckD0Ev
// DEMANGLED: Dispatcher_SDC_Damage_Check::~Dispatcher_SDC_Damage_Check()
Dispatcher_SDC_Damage_Check::~Dispatcher_SDC_Damage_Check() {}

// IDA_EA: 0x8596658
Dispatcher_SDC_Damage_Check::Dispatcher_SDC_Damage_Check()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
