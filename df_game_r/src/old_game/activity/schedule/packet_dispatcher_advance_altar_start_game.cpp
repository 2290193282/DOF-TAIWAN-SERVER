#include "old_game/activity/schedule/packet_dispatcher_advance_altar_start_game.h"

#include <stdint.h>

#include "old_game/activity/reward/advance_altar.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/activity/schedule/advance_altar_data.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace advancealtar {

namespace {

// The raw state-0 IPacketDispatcher specialization constructs exactly the
// four-byte ParamBase stack object then Dispatcher_StartGame writes its result
// at +4 (0x813F8E0) and returns the first eight bytes (0x813F96B).  This is
// the old response ABI, not a new ParamBase-derived wire type.
inline int& StartGameResult(ParamBase& param) {
    return *reinterpret_cast<int*>(
        reinterpret_cast<unsigned char*>(&param) + sizeof(ParamBase));
}

inline void PutBinary(PacketGuard& packet, const void* source,
                      unsigned int size) {
    packet.put_binary(const_cast<char*>(
        reinterpret_cast<const char*>(source)), size);
}

}  // namespace

// IDA_EA: 0x8141781
// IDA_NAME: _ZN12advancealtar22putRidableInfoToPacketER11PacketGuardRKNS_16AdvanceAltarDataE
// DEMANGLED: advancealtar::putRidableInfoToPacket(PacketGuard &,advancealtar::AdvanceAltarData const&)
bool putRidableInfoToPacket(PacketGuard& packet,
                            const AdvanceAltarData& altar_data) {
    PutBinary(packet, &altar_data, 0x21u);

    packet.put_int(static_cast<int>(altar_data.stages_24.size()));
    for (std::map<int, _Stage>::const_iterator it = altar_data.stages_24.begin();
         it != altar_data.stages_24.end(); ++it) {
        PutBinary(packet, &it->second, 0x08u);
    }

    packet.put_int(static_cast<int>(altar_data.unit_buy_items_3c.size()));
    for (std::map<int, _BuyItem>::const_iterator it =
             altar_data.unit_buy_items_3c.begin();
         it != altar_data.unit_buy_items_3c.end(); ++it) {
        PutBinary(packet, &it->second, 0x0cu);
    }

    packet.put_int(static_cast<int>(altar_data.skill_buy_items_54.size()));
    for (std::map<int, _BuyItem>::const_iterator it =
             altar_data.skill_buy_items_54.begin();
         it != altar_data.skill_buy_items_54.end(); ++it) {
        PutBinary(packet, &it->second, 0x0cu);
    }

    packet.put_int(static_cast<int>(altar_data.tower_buy_items_6c.size()));
    for (std::map<int, _BuyItem>::const_iterator it =
             altar_data.tower_buy_items_6c.begin();
         it != altar_data.tower_buy_items_6c.end(); ++it) {
        PutBinary(packet, &it->second, 0x0cu);
    }

    packet.put_int(static_cast<int>(altar_data.unit_slots_84.size()));
    for (std::map<uint8_t, _Slot>::const_iterator it =
             altar_data.unit_slots_84.begin();
         it != altar_data.unit_slots_84.end(); ++it) {
        PutBinary(packet, &it->second, 0x0au);
    }

    packet.put_int(static_cast<int>(altar_data.skill_slots_9c.size()));
    for (std::map<uint8_t, _Slot>::const_iterator it =
             altar_data.skill_slots_9c.begin();
         it != altar_data.skill_slots_9c.end(); ++it) {
        PutBinary(packet, &it->second, 0x0au);
    }

    packet.put_int(static_cast<int>(altar_data.rewards_b4.size()));
    for (std::map<int, _Reward>::const_iterator it = altar_data.rewards_b4.begin();
         it != altar_data.rewards_b4.end(); ++it) {
        PutBinary(packet, &it->second, 0x05u);
    }
    return false;
}

// IDA_EA: 0x813F7B0
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame4readER9PacketBufR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_StartGame::read(PacketBuf &,MSG_BASE &)
int Dispatcher_StartGame::read(PacketBuf& packet, MSG_BASE& message) {
    // CLIENT_IDA_EVIDENCE: both native CMD641 send paths finalize the
    // command immediately after writing its command number (0x4E55EE and
    // 0x6E0E32).  The request therefore has no payload.
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x813F7DE
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: advancealtar::Dispatcher_StartGame::check_error(CUser *,MSG_BASE &)
int Dispatcher_StartGame::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!isOpenAdvanceAltar()) {
        return 1;
    }
    if (user->get_state() != 3) {
        return 3;
    }
    if (user->get_charac_level() <= 49) {
        return 4;
    }
    if (user->isCompetitionMercenary()) {
        return 3;
    }
    if (private_store::GetInstancePrivateStoreMgr()->IsBusyPrivateStore(user)) {
        return 3;
    }
    if (user->CheckInTrade()) {
        return 2;
    }
    if (user->IsBusyDisjointStore()) {
        return 3;
    }
    if (user->CheckInParty()) {
        return 3;
    }
    return G_GameWorld()->IsPVPChannel() ? 3 : 0;
}

// IDA_EA: 0x813F8CE
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: advancealtar::Dispatcher_StartGame::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_StartGame::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    StartGameResult(param) = check_error(user, message);
    if (!StartGameResult(param)) {
        user->advance_altar_manager_.checkAndCreateDefaultRidable();
    }
    return 0;
}

// IDA_EA: 0x813F92A
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGame4sendEP5CUserR9ParamBase
// DEMANGLED: advancealtar::Dispatcher_StartGame::send(CUser *,ParamBase &)
void Dispatcher_StartGame::send(CUser* user, ParamBase& param) {
    if (!StartGameResult(param)) {
        StageControl::gotoStage(user);
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_ADVANCE_ALTAR_START_GAME);
    packet.put_byte(1);
    PutBinary(packet, &param, 0x08u);

    if (!StartGameResult(param)) {
        CharacAdvanceAltarManager& manager = user->advance_altar_manager_;
        manager.setCurrentAdvanceAltarData(RidableId::FIRST);
        user->set_state(13);
        packet.put_int(1);
        for (int index = 0; index <= 0; ++index) {
            AdvanceAltarData* const altar_data =
                manager.getAdvanceAltarDataByIndex(index);
            if (!altar_data) {
                continue;
            }
            putRidableInfoToPacket(packet, *altar_data);
            if (altar_data->is_unlock_stage_effect) {
                altar_data->is_unlock_stage_effect = 0;
                altar_data->updated = 1;
            }
        }
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x814218A
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGameD2Ev
// DEMANGLED: advancealtar::Dispatcher_StartGame::~Dispatcher_StartGame()
// IDA_EA: 0x81421BA
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGameD0Ev
// DEMANGLED: advancealtar::Dispatcher_StartGame::~Dispatcher_StartGame()
Dispatcher_StartGame::~Dispatcher_StartGame() {}

// IDA_EA: 0x85956D0
// IDA_NAME: _ZN12advancealtar20Dispatcher_StartGameC2Ev
// DEMANGLED: advancealtar::Dispatcher_StartGame::Dispatcher_StartGame(void)
Dispatcher_StartGame::Dispatcher_StartGame()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

}  // namespace advancealtar
