#include "old_game/world/region/world_dispatcher_runtime.h"

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_game/economy/inventory/special_item_routing_manager_abi.h"
#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "old_data/db/anti_cheat/db_req_save_hack_user_punish.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/activity/schedule/advance_altar_runtime.h"
#include "old_game/dungeon/instance/zombie_map_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

#include <stdint.h>

typedef char MSG_SET_USER_AREA_Size_Must_Be_0x18[
    (sizeof(MSG_SET_USER_AREA) == 0x18) ? 1 : -1];

// IDA_EA: 0x823245E
// IDA_NAME: _ZN18DisPatcher_MoveMapD2Ev
// DEMANGLED: DisPatcher_MoveMap::~DisPatcher_MoveMap()
// IDA_EA: 0x823248E
// IDA_NAME: _ZN18DisPatcher_MoveMapD0Ev
// DEMANGLED: DisPatcher_MoveMap::~DisPatcher_MoveMap()
DisPatcher_MoveMap::~DisPatcher_MoveMap() {}

// IDA_EA: 0x85951F8
// IDA_NAME: _ZN18DisPatcher_MoveMapC2Ev
// DEMANGLED: DisPatcher_MoveMap::DisPatcher_MoveMap(void)
DisPatcher_MoveMap::DisPatcher_MoveMap()
    : IPacketDispatcher<MSG_MOVE_MAP,
                        stMoveMapParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81C47BA
// IDA_NAME: _ZN18DisPatcher_MoveMap4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_MoveMap::read(PacketBuf &,MSG_BASE &)
int DisPatcher_MoveMap::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MOVE_MAP& input = static_cast<MSG_MOVE_MAP&>(message);
    if (packet.get_byte(input.map_x) != 1) {
        return LineFunc(4434,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_byte(input.map_y) != 1) {
        return LineFunc(4437,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(input.dword_0f) != 1) {
        return LineFunc(4440,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(input.dword_13) != 1) {
        return LineFunc(4441,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_byte(input.layered_map) != 1) {
        return LineFunc(4445,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }

    // The original assigns PacketBuf::get_short's success byte directly to
    // MSG_MOVE_MAP+0x1a and deliberately continues when this read fails.
    input.trap_bits_read = static_cast<uint8_t>(packet.get_short(input.trap_bits));
    // CLIENT_IDA_EVIDENCE: the native CMD45 sender at 0x9C4680 writes
    // eight clear counters followed by eight map-play durations.
    for (int seat = 0; seat < 8; ++seat) {
        if (packet.get_short(input.clear_counts[seat]) != 1) {
            return LineFunc(
                4452,
                "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }
    }
    for (int seat = 0; seat < 8; ++seat) {
        if (packet.get_int(input.map_play_times[seat]) != 1) {
            return LineFunc(
                4457,
                "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                0,
                0);
        }
    }
    if (packet.get_short(input.map_clear_count) != 1) {
        return LineFunc(4462,
                        "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                        0,
                        0);
    }
    if (packet.get_int(input.dword_4d) == 1) {
        return 0;
    }
    return LineFunc(4481,
                    "virtual int DisPatcher_MoveMap::read(PacketBuf&, MSG_BASE&)",
                    0,
                    0);
}

// IDA_EA: 0x81C4A90
// IDA_NAME: _ZN18DisPatcher_MoveMap11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_MoveMap::check_error(CUser *,MSG_BASE &)
int DisPatcher_MoveMap::check_error(CUser* user, MSG_BASE& message) {
    const MSG_MOVE_MAP& input = static_cast<const MSG_MOVE_MAP&>(message);
    if (!user) {
        return -3;
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return -4;
    }
    if (input.map_y >= party->battle_field().getMapHeight()) {
        return -5;
    }
    if (input.map_x >= party->battle_field().getMapWidth()) {
        return -5;
    }

    user->lock();
    user->GetCSHashSet()->clear();
    user->unlock();
    return Global::CGlobalFunc::Get_State_Check(user->get_state()) == 1 ? 0
                                                                         : -1;
}

// IDA_EA: 0x81C4B7A
// IDA_NAME: _ZN18DisPatcher_MoveMap20CheckTrapBitsHackCntEP5CUserR12MSG_MOVE_MAP
// DEMANGLED: DisPatcher_MoveMap::CheckTrapBitsHackCnt(CUser *,MSG_MOVE_MAP &)
char DisPatcher_MoveMap::CheckTrapBitsHackCnt(CUser* user,
                                               MSG_MOVE_MAP& message) {
    const uint16_t quotient = static_cast<uint16_t>(message.trap_bits / 7u);
    const uint16_t reconstructed = static_cast<uint16_t>(quotient * 7u);
    if (message.trap_bits == reconstructed) {
        return static_cast<char>(reconstructed);
    }

    user->getHackAnalyzer()->addServerHackCnt(
        user, static_cast<WongWork::ENUM_HACKTYPE>(0x1f9), 1u, 0u, 0u);
    if (user->getHackAnalyzer()->getServerHackAccumulatedCnt(
            static_cast<WongWork::ENUM_HACKTYPE>(505)) <= 4) {
        return static_cast<char>(reconstructed);
    }

    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }
    for (int seat = 0; seat <= 3; ++seat) {
        if (party->checkValidUser(seat) == 1) {
            CUser* const member = party->get_user(seat);
            if (member != user) {
                member->getHackAnalyzer()->addServerHackCnt(
                    member,
                    static_cast<WongWork::ENUM_HACKTYPE>(0x1fa),
                    1u,
                    0u,
                    0u);
            }
            DB_ReqSaveHackUserPunish::makeRequest(
                static_cast<unsigned int>(member->get_acc_id()),
                0u,
                member->network_.get_str_ip());
            member->DisConnSig(static_cast<DISCONN_SIG>(24), true, 0);
        }
    }
    // The original loop exits with its loop-condition byte in the return
    // register.  Its caller ignores this helper result, so return that final
    // false condition rather than fabricating a success result.
    return 0;
}

// IDA_EA: 0x81C4D26
// IDA_NAME: _ZN18DisPatcher_MoveMap26CheckOneMapPlayTimeHackCntEP5CUserR12MSG_MOVE_MAP
// DEMANGLED: DisPatcher_MoveMap::CheckOneMapPlayTimeHackCnt(CUser *,MSG_MOVE_MAP &)
char DisPatcher_MoveMap::CheckOneMapPlayTimeHackCnt(
    CUser* user, MSG_MOVE_MAP& message) {
    CParty* const party = user->GetParty();
    if (!party) {
        return 0;
    }

    uint32_t permitted_seconds = 0;
    if (user->get_state() == 10) {
        WongWork::CDeathTower* const tower = user->getDeathTower();
        if (tower) {
            permitted_seconds =
                static_cast<uint32_t>(tower->getCStage()->getStageClearTime()) /
                    1000u +
                5u;
        }
    } else {
        permitted_seconds =
            static_cast<uint32_t>(party->GetMapPlayingTime() / 1000u);
    }

    for (int seat = 0; seat <= 3; ++seat) {
        if (party->checkValidUser(seat) != 1) {
            continue;
        }
        CUser* const member = party->get_user(seat);
        if (!member) {
            continue;
        }

        uint32_t& reported_seconds = message.map_play_times[seat];
        CParty* const member_party = member->GetParty();
        CDungeon* const dungeon =
            member_party ? member_party->battle_field().GetDungeon() : 0;
        const bool special_dungeon = dungeon && dungeon->get_index() == 11006;

        if (reported_seconds > 10000u) {
            const uint32_t quotient = reported_seconds / 10000u;
            reported_seconds %= 10000u;
            member->getHackAnalyzer()->addServerHackCnt(
                member,
                static_cast<WongWork::ENUM_HACKTYPE>(
                    special_dungeon ? 0x82 : 0x77),
                1u,
                quotient,
                0u);
        }

        if (member->get_charac_job() == oldsrv::config::kCharacterJobMage ||
            (member->get_charac_job() ==
                 oldsrv::config::kCharacterJobPriest &&
             member->getCurCharFirstGrowType() == 2)) {
            reported_seconds = static_cast<uint32_t>(
                static_cast<long double>(reported_seconds) * 0.5L);
        } else {
            reported_seconds = static_cast<uint32_t>(
                static_cast<long double>(reported_seconds) * 0.8L);
        }

        if (party->battle_field().check_start_point() ||
            reported_seconds <= permitted_seconds) {
            continue;
        }

        const WongWork::ENUM_HACKTYPE hack_type =
            static_cast<WongWork::ENUM_HACKTYPE>(
                special_dungeon ? 0x81 : 0x74);
        const int reports =
            static_cast<long double>(reported_seconds) <=
                    static_cast<long double>(permitted_seconds) * 1.5L
                ? 1
                : 10;
        for (int count = 0; count < reports; ++count) {
            member->getHackAnalyzer()->addServerHackCnt(
                member, hack_type, 1u, reported_seconds, 0u);
        }
    }
    return 0;
}

// IDA_EA: 0x81C5330
// IDA_NAME: _ZN18DisPatcher_MoveMap7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_MoveMap::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_MoveMap::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    MSG_MOVE_MAP& input = static_cast<MSG_MOVE_MAP&>(message);
    stMoveMapParam& move_param = static_cast<stMoveMapParam&>(param);
    move_param.result = check_error(user, message);
    if (move_param.result > 0) {
        return 0;
    }
    if (move_param.result < 0) {
        return LineFunc(
            4712,
            "virtual int DisPatcher_MoveMap::process(CUser*, MSG_BASE&, ParamBase&)",
            move_param.result,
            static_cast<unsigned int>(user->get_acc_id()));
    }

    if (input.trap_bits_read != 1) {
        user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(602), 1u, 0u, 0u);
    }
    user->GetHackCheck()->MoveMap();
    CheckTrapBitsHackCnt(user, input);
    CheckOneMapPlayTimeHackCnt(user, input);
    if (Global::CBossTowerFunc::MoveMap(user, input.clear_counts)) {
        return 0;
    }

    CParty* const party = user->GetParty();
    if (party->IsRoutingState()) {
        CSpecialItemRoutingManager* const routing_manager =
            G_CGameManager()->GetSpecialItemRoutingManager();
        if (routing_manager) {
            routing_manager->ProcessFailEtc(party, 0);
        }
    }
    int zombie_move_result = 0;
    const zombie_map_runtime::HookResult zombie_move =
        zombie_map_runtime::ConsumeMoveMap(
            user, input, &zombie_move_result);
    if (zombie_move != zombie_map_runtime::kHookNotManaged) {
        static_cast<void>(zombie_move_result);
        if (zombie_move == zombie_map_runtime::kHookRejected &&
            zombie_move_result < 0) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MOVE_MAP, 19);
        }
        return 0;
    }
    input.party_seat = static_cast<uint8_t>(party->get_party_seatno(user));
    party->move_map(input);
    return 0;
}

// IDA_EA: 0x82324AC
// IDA_NAME: _ZN18DisPatcher_MoveMap11check_stateEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_MoveMap::check_state(CUser *,PacketBuf &)
int DisPatcher_MoveMap::check_state(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (Global::CGlobalFunc::Get_State_Check(user->get_state())) {
        return 0;
    }
    return LineFunc(
        577,
        "virtual int DisPatcher_MoveMap::check_state(CUser*, PacketBuf&)",
        0,
        0);
}

// The shared A-owned primary template is instantiated once after this exact
// message and parameter pair are complete.
// IDA_EA: 0x8236660/0x8236690/0x8259302/0x8259452/0x825945C/0x825AD2A/0x85980F2
template class IPacketDispatcher<MSG_MOVE_MAP,
                                 stMoveMapParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x81C91E6
// IDA_NAME: _ZN18DisPatcher_SetArea4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetArea::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetArea::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SET_USER_AREA& input = reinterpret_cast<MSG_SET_USER_AREA&>(message);
    if (packet.get_byte(input.area) != 1) {
        return LineFunc(
            7190,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_byte(input.before_area) != 1) {
        return LineFunc(
            7191,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_short(input.village_index) != 1) {
        return LineFunc(
            7192,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_short(input.move_index) != 1) {
        return LineFunc(
            7193,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_byte(input.move_type) != 1) {
        return LineFunc(
            7194,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }

    input.move_arg0 = 0;
    input.move_arg1 = 0;
    if (packet.get_short(input.move_arg0) != 1) {
        return LineFunc(
            7199,
            "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_short(input.move_arg1) == 1) {
        return 0;
    }
    return LineFunc(
        7200,
        "virtual int DisPatcher_SetArea::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81C93E8
// IDA_NAME: _ZN18DisPatcher_SetArea11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetArea::check_error(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetArea::check_error(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    const MSG_SET_USER_AREA& input =
        reinterpret_cast<const MSG_SET_USER_AREA&>(message);
    stSetAreaParam& set_area_param = reinterpret_cast<stSetAreaParam&>(param);

    if (user->get_state() != 3) {
        return 0x7FFFFFFF;
    }
    if (static_cast<unsigned char>(user->IsBusyDisjointStore())) {
        set_area_param.area = input.area;
        return 191;
    }

    private_store::CPrivateStoreMgr* private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (static_cast<unsigned char>(
            private_store_manager->IsBusyPrivateStore(user))) {
        set_area_param.area = input.area;
        return 60;
    }
    if (static_cast<unsigned char>(user->IsCurCharacGhost())) {
        set_area_param.area = input.area;
        return 19;
    }

    const char* charac_name = user->getCurCharacName();
    GameWorld* world = G_GameWorld();
    if (static_cast<unsigned char>(world->IsForbiddenMove(charac_name))) {
        const char* forbidden_name = user->getCurCharacName();
        if (G_GameWorld()->GetLeftTimeOfForbiddenMove(forbidden_name)) {
            set_area_param.forbidden_area = input.before_area;
            return 68;
        }
        G_GameWorld()->EnableMove(user->getCurCharacName());
    }
    return 0;
}

// IDA_EA: 0x81C951C
// IDA_NAME: _ZN18DisPatcher_SetArea7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetArea::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetArea::process(CUser* user,
                                MSG_BASE& message,
                                ParamBase& param) {
    const MSG_SET_USER_AREA& input =
        reinterpret_cast<const MSG_SET_USER_AREA&>(message);
    stSetAreaParam& set_area_param = reinterpret_cast<stSetAreaParam&>(param);
    set_area_param.result = check_error(user, message, param);

    if (set_area_param.result > 0) {
        return 0;
    }
    if (set_area_param.result >= 0) {
        G_GameWorld()->move_area(
            user,
            input.area,
            input.before_area,
            input.village_index,
            input.move_index,
            input.move_type,
            true,
            0,
            input.move_arg0,
            input.move_arg1);
        return 0;
    }

    return LineFunc(
        7277,
        "virtual int DisPatcher_SetArea::process(CUser*, MSG_BASE&, ParamBase&)",
        set_area_param.result,
        static_cast<unsigned int>(user->get_acc_id()));
}

// IDA_EA: 0x81C9656
// IDA_NAME: _ZN18DisPatcher_SetArea4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SetArea::send(CUser *,ParamBase &)
void DisPatcher_SetArea::send(CUser* user, ParamBase& param) {
    stSetAreaParam& set_area_param = reinterpret_cast<stSetAreaParam&>(param);
    if (set_area_param.result == 0x7FFFFFFF || !set_area_param.result) {
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, ENUM_CMDPACKET_SET_USER_AREA);
    packet.put_byte(0);
    packet.put_byte(set_area_param.result);
    if (set_area_param.result == 68) {
        packet.put_byte(set_area_param.forbidden_area);
        const unsigned int left_time =
            G_GameWorld()->GetLeftTimeOfForbiddenMove(
                user->getCurCharacName());
        packet.put_int(static_cast<int>(left_time / 0xEA60));
    } else {
        packet.put_byte(set_area_param.area);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8232632
// IDA_NAME: _ZN18DisPatcher_SetAreaD2Ev
// DEMANGLED: DisPatcher_SetArea::~DisPatcher_SetArea()
// IDA_EA: 0x8232662
// IDA_NAME: _ZN18DisPatcher_SetAreaD0Ev
// DEMANGLED: DisPatcher_SetArea::~DisPatcher_SetArea()
DisPatcher_SetArea::~DisPatcher_SetArea() {}

// IDA_EA: 0x8595284
// IDA_NAME: _ZN18DisPatcher_SetAreaC2Ev
// DEMANGLED: DisPatcher_SetArea::DisPatcher_SetArea(void)
DisPatcher_SetArea::DisPatcher_SetArea()
    : IPacketDispatcher<MSG_SET_USER_AREA,
                        stSetAreaParam,
                        static_cast<ch_state>(3)>() {}

// The shared A-owned primary template is instantiated exactly once here after
// the complete message and parameter types are visible.
// IDA_EA: 0x8258768
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82588B8
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82588C2
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825AD0C
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x82367E6
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8236816
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x859817E
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_SET_USER_AREA14stSetAreaParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SET_USER_AREA,stSetAreaParam,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_SET_USER_AREA,
                                 stSetAreaParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x81F8602
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_SetPVPMapIndex::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_SetPVPMapIndex::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 6) {
        return 0;
    }

    PacketGuard out_packet;
    short map_no = 0;
    if (packet.get_short(map_no) != 1) {
        return LineFunc(
            34384,
            "virtual int DisPatcher_SetPVPMapIndex::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* pvp_room = user->GetPVPRoom();
    if (!pvp_room) {
        return LineFunc(
            34387,
            "virtual int DisPatcher_SetPVPMapIndex::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    bool changed_to_individual = false;
    if (static_cast<unsigned char>(pvp_room->CheckPvPMapMode(
            user,
            static_cast<int>(map_no),
            changed_to_individual)) != 1u) {
        out_packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                              ENUM_CMDPACKET_SET_PVP_MAP_INDEX);
        out_packet.put_byte(0);
        out_packet.put_int(19);
        out_packet.finalize(true);
        user->Send(out_packet);
        return 0;
    }

    pvp_room->SetRequestedMapNo(map_no);
    pvp_room->make_state_info(reinterpret_cast<char*>(&out_packet));
    G_GameWorld()->send_all(out_packet);

    if (!changed_to_individual && pvp_room->get_pvp_battle_mode() == 4) {
        const int set_mode_result = pvp_room->set_pvp_mode(user, 5);
        if (set_mode_result > 0) {
            out_packet.clear();
            out_packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                  ENUM_CMDPACKET_SET_PVP_MAP_INDEX);
            out_packet.put_byte(0);
            out_packet.put_byte(19);
            out_packet.finalize(true);
            user->Send(out_packet);
            return 0;
        }

        out_packet.clear();
        pvp_room->make_seat_info(reinterpret_cast<char*>(&out_packet), 0xFFFF);
        G_GameWorld()->send_all(out_packet);
    }
    return 0;
}

// IDA_EA: 0x82332D8
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SetPVPMapIndex::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SetPVPMapIndex::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82332E2
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndex7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SetPVPMapIndex::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SetPVPMapIndex::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82332EC
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndexD2Ev
// DEMANGLED: DisPatcher_SetPVPMapIndex::~DisPatcher_SetPVPMapIndex()
// IDA_EA: 0x823331C
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndexD0Ev
// DEMANGLED: DisPatcher_SetPVPMapIndex::~DisPatcher_SetPVPMapIndex()
DisPatcher_SetPVPMapIndex::~DisPatcher_SetPVPMapIndex() {}

// IDA_EA: 0x8595644
// IDA_NAME: _ZN25DisPatcher_SetPVPMapIndexC2Ev
// DEMANGLED: DisPatcher_SetPVPMapIndex::DisPatcher_SetPVPMapIndex(void)
DisPatcher_SetPVPMapIndex::DisPatcher_SetPVPMapIndex()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8233876
// IDA_NAME: _ZN23Dispatcher_Back2Village4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Back2Village::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Back2Village::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233880
// IDA_NAME: _ZN23Dispatcher_Back2Village7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Back2Village::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Back2Village::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x820559A
// IDA_NAME: _ZN23Dispatcher_Back2Village12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Back2Village::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Back2Village::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() == 13) {
        advancealtar::Manager::leaveUser(user);
        return 0;
    }
    if (user->get_state() != 5) {
        return 0;
    }

    CParty* const party = user->GetParty();
    if (!party || party->GetEPLPState()) {
        return 0;
    }
    if (party->getManager() == user) {
        if (party->get_quick_party_index() != -1) {
            G_CGameManager()->CheckOutQuickParty(party, true);
        }
        party->ReturnToVillage();
        return 0;
    }

    if (party->leave_user(user, static_cast<ENUM_PARTY_INFO_TYPE>(2)) == 1) {
        return 0;
    }
    G_GameWorld()->out_from_dungeon(user);
    PacketGuard response;
    response.clear();
    response.put_header(ENUM_PACKETCLASS_NOTIPACKET, 2);
    response.put_byte(0);
    response.put_short(1);
    user->make_basic_info(response, 0);
    response.finalize(true);
    G_GameWorld()->send_all(response, user);
    return 0;
}

// IDA_EA: 0x82429BE
// IDA_NAME: _ZN23Dispatcher_Back2VillageD2Ev
// DEMANGLED: Dispatcher_Back2Village::~Dispatcher_Back2Village()
// IDA_EA: 0x82429EE
// IDA_NAME: _ZN23Dispatcher_Back2VillageD0Ev
// DEMANGLED: Dispatcher_Back2Village::~Dispatcher_Back2Village()
Dispatcher_Back2Village::~Dispatcher_Back2Village() {}

// IDA_EA: 0x8596150
// IDA_NAME: _ZN23Dispatcher_Back2VillageC2Ev
// DEMANGLED: Dispatcher_Back2Village::Dispatcher_Back2Village(void)
Dispatcher_Back2Village::Dispatcher_Back2Village()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
