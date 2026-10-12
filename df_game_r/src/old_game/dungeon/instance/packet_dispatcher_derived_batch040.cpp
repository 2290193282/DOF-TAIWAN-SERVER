#include "old_game/dungeon/instance/packet_dispatcher_derived_batch040.h"
#include "infra/network/packet/PacketEnum.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x81FD25C
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacter12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_ReturnToSelectCharacter::dispatch_sig(CUser* user,
                                                      PacketBuf& packet) {
    (void)packet;

    // IDA 0x81FD25C: CMD 7 always emits the response.  The original permits
    // the transition only after gameplay state 2 and outside village 7;
    // either rejected path carries failure code 19.
    unsigned char result = 1;
    if (user->get_state() > 2) {
        if (user->getCurCharacVill() == 7u) {
            result = 0;
        } else {
            user->ReturnToSelectCharacList(false);
        }
    } else {
        result = 0;
    }

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                        ENUM_CMDPACKET_RETURN_SELECT_CHARACTER);
    response.put_byte(result);
    if (result != 1u) {
        response.put_byte(19);
    }
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x8233490
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacter4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::read(PacketBuf &,MSG_BASE &)
int DisPatcher_ReturnToSelectCharacter::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823349A
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacter7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_ReturnToSelectCharacter::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8243726
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacterD2Ev
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::~DisPatcher_ReturnToSelectCharacter()
// IDA_EA: 0x8243756
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacterD0Ev
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::~DisPatcher_ReturnToSelectCharacter()
DisPatcher_ReturnToSelectCharacter::~DisPatcher_ReturnToSelectCharacter() {}

// IDA_EA: 0x85959E0
// IDA_NAME: _ZN34DisPatcher_ReturnToSelectCharacterC2Ev
// DEMANGLED: DisPatcher_ReturnToSelectCharacter::DisPatcher_ReturnToSelectCharacter(void)
DisPatcher_ReturnToSelectCharacter::DisPatcher_ReturnToSelectCharacter()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8217AD0
// IDA_NAME: _ZN23Dispatcher_RequestMatch12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestMatch::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestMatch::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return 0;
    }
    PvP_Room* const room = user->GetPVPRoom();
    if (!room || !room->current_match_) {
        return 0;
    }
    unsigned char unregister = 0;
    if (packet.get_byte(unregister) != 1) {
        return LineFunc(51652,
                        "virtual int Dispatcher_RequestMatch::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
    }
    if (unregister) {
        room->current_match_->OnUnregisterMatch(user, room);
    } else {
        room->current_match_->OnRegisterMatch(user, room);
    }
    return 0;
}

// IDA_EA: 0x8233CFE
// IDA_NAME: _ZN23Dispatcher_RequestMatch4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestMatch::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestMatch::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233D08
// IDA_NAME: _ZN23Dispatcher_RequestMatch7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestMatch::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestMatch::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82418FC
// IDA_NAME: _ZN23Dispatcher_RequestMatchD2Ev
// DEMANGLED: Dispatcher_RequestMatch::~Dispatcher_RequestMatch()
// IDA_EA: 0x824192C
// IDA_NAME: _ZN23Dispatcher_RequestMatchD0Ev
// DEMANGLED: Dispatcher_RequestMatch::~Dispatcher_RequestMatch()
Dispatcher_RequestMatch::~Dispatcher_RequestMatch() {}

// IDA_EA: 0x8596888
// IDA_NAME: _ZN23Dispatcher_RequestMatchC2Ev
// DEMANGLED: Dispatcher_RequestMatch::Dispatcher_RequestMatch(void)
Dispatcher_RequestMatch::Dispatcher_RequestMatch()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81F88AE
// IDA_NAME: _ZN23DisPatcher_EndPvPResult12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: DisPatcher_EndPvPResult::dispatch_sig(CUser *,PacketBuf &)
int DisPatcher_EndPvPResult::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 7) {
        return 0;
    }
    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return LineFunc(34433,
                        "virtual int DisPatcher_EndPvPResult::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
    }
    room->set_recv_pvp_end_result_flag(user);
    if (room->recv_pvp_end_result_all()) {
        room->end_pvp_result();
        PacketGuard state_packet;
        room->make_state_info(reinterpret_cast<char*>(&state_packet));
        G_GameWorld()->send_all(state_packet);
        room->end_pvp();
        room->battle_reset();
    } else if (room->isRematchable() && room->recv_pvp_end_result_team(user)) {
        if (room->current_match_) {
            room->current_match_->OnEndPvPResult(room, user);
        } else {
            LogManager::logFormat(
                1,
                "PacketDispatcher_Impl_1.cpp",
                "virtual int DisPatcher_EndPvPResult::dispatch_sig(CUser*, PacketBuf&)",
                0x86B6,
                "error end pvp result team");
        }
    } else if (room->get_recv_pvp_end_result_count() == 1) {
        room->insert_timer_recv_pvp_end_result();
    }
    return 0;
}

// IDA_EA: 0x8233344
// IDA_NAME: _ZN23DisPatcher_EndPvPResult7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_EndPvPResult::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_EndPvPResult::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x823334E
// IDA_NAME: _ZN23DisPatcher_EndPvPResultD2Ev
// DEMANGLED: DisPatcher_EndPvPResult::~DisPatcher_EndPvPResult()
// IDA_EA: 0x823337E
// IDA_NAME: _ZN23DisPatcher_EndPvPResultD0Ev
// DEMANGLED: DisPatcher_EndPvPResult::~DisPatcher_EndPvPResult()
DisPatcher_EndPvPResult::~DisPatcher_EndPvPResult() {}

// IDA_EA: 0x8595660
// IDA_NAME: _ZN23DisPatcher_EndPvPResultC2Ev
// DEMANGLED: DisPatcher_EndPvPResult::DisPatcher_EndPvPResult(void)
DisPatcher_EndPvPResult::DisPatcher_EndPvPResult()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8226F30
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficulty12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_SelectUltimateDifficulty::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    if (user->get_state() != 5) {
        return 0;
    }
    CParty* const party = user->GetParty();
    if (!party || party->getManager() != user) {
        return 0;
    }
    CBattle_Field& battle_field = party->battle_field();
    if (battle_field.getBloodState() > 2) {
        return 0;
    }
    char difficulty = 0;
    if (packet.get_byte(difficulty) != 1) {
        return LineFunc(58073,
                        "virtual int Dispatcher_SelectUltimateDifficulty::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
    }
    if (difficulty != 1 && difficulty != 2) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            "virtual int Dispatcher_SelectUltimateDifficulty::dispatch_sig(CUser*, PacketBuf&)",
            0xE2DD,
            "ULTIMATE_LOG : Difficulty ERROR!! difficulty (%d)\n",
            difficulty);
        return 0;
    }
    CDungeon* const dungeon = battle_field.GetDungeon();
    if (!dungeon || dungeon->get_index() != 11007) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            "virtual int Dispatcher_SelectUltimateDifficulty::dispatch_sig(CUser*, PacketBuf&)",
            0xE2E3,
            "ULTIMATE_LOG : Dispatcher_SelectUltimateDifficulty ERROR!!\n");
        return 0;
    }
    if (battle_field.getUltimateDifficylty()) {
        return 0;
    }
    battle_field.setUltimateDifficulty(difficulty);
    PacketGuard notify_packet;
    notify_packet.put_header(1, ENUM_CMDPACKET_SELECT_ULTIMATE_DIFFICULTY);
    notify_packet.put_byte(1);
    notify_packet.put_byte(difficulty);
    notify_packet.finalize(1);
    party->send_to_party(notify_packet);
    battle_field.startUltimateRound();
    return 0;
}

// IDA_EA: 0x8234262
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficulty4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SelectUltimateDifficulty::read(PacketBuf& packet,
                                               MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823426C
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficulty7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SelectUltimateDifficulty::process(CUser* user,
                                                  MSG_BASE& message,
                                                  ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82403A8
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficultyD2Ev
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::~Dispatcher_SelectUltimateDifficulty()
// IDA_EA: 0x82403D8
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficultyD0Ev
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::~Dispatcher_SelectUltimateDifficulty()
Dispatcher_SelectUltimateDifficulty::~Dispatcher_SelectUltimateDifficulty() {}

// IDA_EA: 0x859732A
// IDA_NAME: _ZN35Dispatcher_SelectUltimateDifficultyC2Ev
// DEMANGLED: Dispatcher_SelectUltimateDifficulty::Dispatcher_SelectUltimateDifficulty(void)
Dispatcher_SelectUltimateDifficulty::Dispatcher_SelectUltimateDifficulty()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8227862
// IDA_NAME: _ZN21Dispatcher_AskRematch12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_AskRematch::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_AskRematch::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }
    char answer = 0;
    if (packet.get_byte(answer) != 1) {
        return LineFunc(58287,
                        "virtual int Dispatcher_AskRematch::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
    }
    PvP_Room* const room = user->GetPVPRoom();
    if (room && room->current_match_) {
        room->current_match_->OnAskRematch(user, static_cast<int>(answer));
    }
    return 0;
}

// IDA_EA: 0x82342DA
// IDA_NAME: _ZN21Dispatcher_AskRematch4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_AskRematch::read(PacketBuf &,MSG_BASE &)
int Dispatcher_AskRematch::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82342E4
// IDA_NAME: _ZN21Dispatcher_AskRematch7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_AskRematch::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_AskRematch::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x82401D4
// IDA_NAME: _ZN21Dispatcher_AskRematchD2Ev
// DEMANGLED: Dispatcher_AskRematch::~Dispatcher_AskRematch()
// IDA_EA: 0x8240204
// IDA_NAME: _ZN21Dispatcher_AskRematchD0Ev
// DEMANGLED: Dispatcher_AskRematch::~Dispatcher_AskRematch()
Dispatcher_AskRematch::~Dispatcher_AskRematch() {}

// IDA_EA: 0x85975CA
// IDA_NAME: _ZN21Dispatcher_AskRematchC2Ev
// DEMANGLED: Dispatcher_AskRematch::Dispatcher_AskRematch(void)
Dispatcher_AskRematch::Dispatcher_AskRematch()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81E371A
// IDA_NAME: _ZN26Dispatcher_TutorialLevelUp4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_TutorialLevelUp::read(PacketBuf &,MSG_BASE &)
int Dispatcher_TutorialLevelUp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x81E3724
// IDA_NAME: _ZN26Dispatcher_TutorialLevelUp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_TutorialLevelUp::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_TutorialLevelUp::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)message;
    (void)param;
    if (user->get_state() != 5) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP, 19);
        return 0;
    }
    CParty* const party = user->GetParty();
    if (!party) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP, 19);
        return 0;
    }
    const int map_no = party->battle_field().get_cur_map();
    const bool at_slayer_tutorial =
        user->get_charac_job() == oldsrv::config::kCharacterJobAtSwordman &&
        map_no == 61024;
    if (map_no != 61001 && map_no != 61009 && map_no != 61016 &&
        !at_slayer_tutorial) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP, 19);
        return 0;
    }
    if (at_slayer_tutorial && user->get_charac_level() == 2) {
        // CMD481 is empty and the client may have queued more frames before
        // receiving the first success.  Acknowledge replays without awarding
        // experience again so its local tutorial state can converge.
        user->SendCmdOkPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP);
        LogManager::logFormat(
            1,
            "tutorial_atslayer_0828.cpp",
            "Dispatcher_TutorialLevelUp::process",
            0,
            "[TUTORIAL481][ACK_REPLAY] account=%d charac=%d map=%d level=%d",
            user->get_acc_id(),
            user->getCurCharacNo(),
            map_no,
            user->get_charac_level());
        return 0;
    }
    if (user->get_charac_level() != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP, 19);
        return 0;
    }
    char level_up_item_state = 0;
    int remaining_exp = 0;
    if (user->CalLevelUpItemState(level_up_item_state,
                                  remaining_exp,
                                  1,
                                  59) == 1) {
        int gained_sp = 0;
        int gained_sfp = 0;
        int previous_sp = 0;
        int current_sp = 0;
        user->rewardExp(remaining_exp,
                        gained_sp,
                        gained_sfp,
                        previous_sp,
                        current_sp,
                        static_cast<eExpAddReason>(13),
                        true);
        user->SendCmdOkPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP);
        return 0;
    }
    user->SendCmdErrorPacket(ENUM_CMDPACKET_TUTORIAL_LEVEL_UP, 19);
    return 0;
}

// IDA_EA: 0x8243F60
// IDA_NAME: _ZN26Dispatcher_TutorialLevelUpD2Ev
// DEMANGLED: Dispatcher_TutorialLevelUp::~Dispatcher_TutorialLevelUp()
// IDA_EA: 0x8243F90
// IDA_NAME: _ZN26Dispatcher_TutorialLevelUpD0Ev
// DEMANGLED: Dispatcher_TutorialLevelUp::~Dispatcher_TutorialLevelUp()
Dispatcher_TutorialLevelUp::~Dispatcher_TutorialLevelUp() {}

// IDA_EA: 0x8597D56
// IDA_NAME: _ZN26Dispatcher_TutorialLevelUpC2Ev
// DEMANGLED: Dispatcher_TutorialLevelUp::Dispatcher_TutorialLevelUp(void)
Dispatcher_TutorialLevelUp::Dispatcher_TutorialLevelUp()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
