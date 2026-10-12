#include "old_game/dungeon/instance/packet_dispatcher_use_coin.h"
#include "old_game/dungeon/instance/zombie_mode_result.h"

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/battlefield/war_room.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// CMD044 owns a concrete message/parameter template specialization.  Keep
// this feature source as its sole vtable/template provider.
template class IPacketDispatcher<MSG_USE_COIN,
                                 stUseCoinParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x85952F4
// IDA_NAME: _ZN18DisPatcher_UseCoinC2Ev
// DEMANGLED: DisPatcher_UseCoin::DisPatcher_UseCoin(void)
DisPatcher_UseCoin::DisPatcher_UseCoin()
    : IPacketDispatcher<MSG_USE_COIN,
                        stUseCoinParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823276A
// IDA_NAME: _ZN18DisPatcher_UseCoinD2Ev
// DEMANGLED: DisPatcher_UseCoin::~DisPatcher_UseCoin()
// IDA_EA: 0x823279A
// IDA_NAME: _ZN18DisPatcher_UseCoinD0Ev
// DEMANGLED: DisPatcher_UseCoin::~DisPatcher_UseCoin()
DisPatcher_UseCoin::~DisPatcher_UseCoin() {}

// IDA_EA: 0x81CA5E4
// IDA_NAME: _ZN18DisPatcher_UseCoin4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_UseCoin::read(PacketBuf &, MSG_BASE &)
int DisPatcher_UseCoin::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_USE_COIN& request = static_cast<MSG_USE_COIN&>(message);
    if (packet.get_short(request.target_unique_id) == 1) {
        return 0;
    }
    return LineFunc(7867,
                    "virtual int DisPatcher_UseCoin::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81CA63A
// IDA_NAME: _ZN18DisPatcher_UseCoin19Dungeon_check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseCoin::Dungeon_check_error(CUser *, MSG_BASE &, ParamBase &)
int DisPatcher_UseCoin::Dungeon_check_error(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    const MSG_USE_COIN& request = static_cast<const MSG_USE_COIN&>(message);
    static_cast<void>(param);
    if (user && zombie_mode::IsRevivalBlocked(user->GetPVPRoom())) {
        return 17;
    }
    CParty* const party = user->GetParty();
    if (!party) {
        return -1;
    }

    CParty* const party_with_dungeon = user->GetParty();
    CDungeon* const dungeon =
        party_with_dungeon
            ? party_with_dungeon->battle_field().get_dungeon()
            : 0;
    if (dungeon) {
        if (dungeon->get_blood_dungeon_type() > 0 &&
            party_with_dungeon->battle_field().getBloodState() > 2) {
            return 19;
        }
        if (dungeon->isTournamentDungeon() &&
            party_with_dungeon->GetTournamentDungeonClearState() > 0) {
            return 19;
        }
    }

    const int coin_check = usedCoinDungeonCheck(user, party);
    return coin_check ? coin_check
                      : party->ReviveUser(user, request.target_unique_id);
}

// IDA_EA: 0x81CA75C
// IDA_NAME: _ZN18DisPatcher_UseCoin19WarRoom_check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseCoin::WarRoom_check_error(CUser *, MSG_BASE &, ParamBase &)
int DisPatcher_UseCoin::WarRoom_check_error(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    const MSG_USE_COIN& request = static_cast<const MSG_USE_COIN&>(message);
    static_cast<void>(param);
    WarRoom* const room = user->GetWarRoom();
    if (!room) {
        return -2;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory->IsCoinUsable()) {
        return 17;
    }
    return room->ReviveUserByCoin(user, request.target_unique_id);
}

// IDA_EA: 0x81CA7D6
// IDA_NAME: _ZN18DisPatcher_UseCoin11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseCoin::check_error(CUser *, MSG_BASE &, ParamBase &)
int DisPatcher_UseCoin::check_error(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    if (user->get_state() == 5) {
        return Dungeon_check_error(user, message, param);
    }
    if (user->get_state() == 8) {
        return WarRoom_check_error(user, message, param);
    }
    return 19;
}

// IDA_EA: 0x81CAB3A
// IDA_NAME: _ZN18DisPatcher_UseCoin20usedCoinDungeonCheckEP5CUserP6CParty
// DEMANGLED: DisPatcher_UseCoin::usedCoinDungeonCheck(CUser *, CParty *)
int DisPatcher_UseCoin::usedCoinDungeonCheck(CUser* user, CParty* party) {
    if (!user || !party) {
        return -1;
    }
    int dungeon_index = user->getPlayingDungeonIndex();
    if (dungeon_index == -1) {
        dungeon_index = party->battle_field().get_dungeon_index();
    }
    CDungeon* const dungeon = G_CDataManager()->find_dungeon(dungeon_index);
    if (!dungeon) {
        LogManager::logFormat(1,
                               "PacketDispatcher_Impl_1.cpp",
                               "int DisPatcher_UseCoin::usedCoinDungeonCheck(CUser*, CParty*)",
                               0x1fa9,
                               "DisPatcher_UseCoin G_CDataManager()->find_dungeon( %d) fail",
                               dungeon_index);
        return 0x7fffffff;
    }
    if (party->checkFreeRevivalCondition(dungeon)) {
        return 0;
    }
    CInventory* const inventory = user->getCurCharacInvenR();
    if (!inventory->IsCoinUsable()) {
        return 17;
    }
    if (party->checkUnLimitUsedCoinCondition(dungeon)) {
        return 0;
    }
    return party->checkLimitUsedCoinCondition(dungeon);
}

// IDA_EA: 0x81CA852
// IDA_NAME: _ZN18DisPatcher_UseCoin7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_UseCoin::process(CUser *, MSG_BASE &, ParamBase &)
int DisPatcher_UseCoin::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    const MSG_USE_COIN& request = static_cast<const MSG_USE_COIN&>(message);
    stUseCoinParam& result = static_cast<stUseCoinParam&>(param);
    result.result_code = check_error(user, message, param);
    if (result.result_code > 0) {
        return 0;
    }
    if (result.result_code < 0) {
        return LineFunc(
            8013,
            "virtual int DisPatcher_UseCoin::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result_code,
            user->get_acc_id());
    }
    result.target_unique_id = request.target_unique_id;

    if (user->get_state() == 5) {
        CParty* const party = user->GetParty();
        if (!party) {
            return LineFunc(
                8026,
                "virtual int DisPatcher_UseCoin::process(CUser*, MSG_BASE&, ParamBase&)",
                0,
                user->get_acc_id());
        }
        const bool free_revive = party->checkFreeRevivalCondition(0);
        result.free_revive_mode = free_revive ? 1 : 0;
        if (!free_revive) {
            CInventory* const inventory = user->getCurCharacInvenW();
            inventory->UseCoin(static_cast<eCoinSubReason>(0));
            user->getCurCharacQuestW()->set_authen_data(
                static_cast<QUEST_CONDITION>(3), 1, 0);
            party->IncreaseUsedCoinCount();
            APSystem::CUserProc::ClearActionAndSendtoUser(
                user,
                static_cast<APSystem::_ActionGroupIndex>(0),
                0,
                ENUM_PACKETCLASS_CMDPACKET);
        }
    } else if (user->get_state() == 8) {
        CInventory* const inventory = user->getCurCharacInvenW();
        inventory->UseCoin(static_cast<eCoinSubReason>(4));
        user->getCurCharacQuestW()->set_authen_data(
            static_cast<QUEST_CONDITION>(3), 1, 0);
        user->IncreaseUsedCoinCount();
    }
    return 0;
}

// IDA_EA: 0x81CAA42
// IDA_NAME: _ZN18DisPatcher_UseCoin4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_UseCoin::send(CUser *, ParamBase &)
void DisPatcher_UseCoin::send(CUser* user, ParamBase& param) {
    const stUseCoinParam& result = static_cast<const stUseCoinParam&>(param);
    if (result.result_code == 0x7fffffff) {
        return;
    }
    if (result.result_code) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_USE_COIN,
                                 static_cast<unsigned char>(result.result_code));
        return;
    }
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_USE_COIN);
    packet.put_byte(1);
    packet.put_short(result.target_unique_id);
    packet.put_byte(result.free_revive_mode);
    packet.finalize(true);
    user->Send(packet);
}
