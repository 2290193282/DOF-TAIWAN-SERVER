#include "old_game/pvp/battlefield/packet_dispatcher_pvp_assault.h"
#include "infra/network/packet/PacketEnum.h"

#include <stdint.h>

#include "common/config.h"
#include "common/manager/community_server_proxy_abi.h"
#include "common/manager/game_manager.h"
#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/pvp/battlefield/packet_dispatcher_pvp_assault_abi.h"
#include "old_game/pvp/battlefield/pvp_assault.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/concurrent/timer/timer_queue.h"

// IDA_EA: 0x8596000
// IDA_NAME: _ZN26Dispatcher_PVPRequestFightC2Ev
// DEMANGLED: Dispatcher_PVPRequestFight::Dispatcher_PVPRequestFight(void)
Dispatcher_PVPRequestFight::Dispatcher_PVPRequestFight() {}

// IDA_EA: 0x8242BE0
// IDA_NAME: _ZN26Dispatcher_PVPRequestFightD2Ev
// DEMANGLED: Dispatcher_PVPRequestFight::~Dispatcher_PVPRequestFight()
// IDA_EA: 0x8242C10
// IDA_NAME: _ZN26Dispatcher_PVPRequestFightD0Ev
// DEMANGLED: Dispatcher_PVPRequestFight::~Dispatcher_PVPRequestFight()
Dispatcher_PVPRequestFight::~Dispatcher_PVPRequestFight() {}

// IDA_EA: 0x820464E
// IDA_NAME: _ZN26Dispatcher_PVPRequestFight12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PVPRequestFight::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PVPRequestFight::dispatch_sig(CUser* user, PacketBuf& packet) {
    (void)packet;
    if (user->get_state() != 7) {
        return LineFunc(
            39973,
            "virtual int Dispatcher_PVPRequestFight::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        return LineFunc(
            39979,
            "virtual int Dispatcher_PVPRequestFight::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (room->request_fight(user) == 1) {
        return 0;
    }
    return LineFunc(
        39984,
        "virtual int Dispatcher_PVPRequestFight::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x82337EA
// IDA_NAME: _ZN26Dispatcher_PVPRequestFight4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PVPRequestFight::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PVPRequestFight::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82337F4
// IDA_NAME: _ZN26Dispatcher_PVPRequestFight7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PVPRequestFight::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PVPRequestFight::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596268
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultC2Ev
// DEMANGLED: Dispatcher_CompleteLoadAssault::Dispatcher_CompleteLoadAssault(void)
Dispatcher_CompleteLoadAssault::Dispatcher_CompleteLoadAssault()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824274E
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultD2Ev
// DEMANGLED: Dispatcher_CompleteLoadAssault::~Dispatcher_CompleteLoadAssault()
// IDA_EA: 0x824277E
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssaultD0Ev
// DEMANGLED: Dispatcher_CompleteLoadAssault::~Dispatcher_CompleteLoadAssault()
Dispatcher_CompleteLoadAssault::~Dispatcher_CompleteLoadAssault() {}

// IDA_EA: 0x8206F10
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompleteLoadAssault::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompleteLoadAssault::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 1) {
        return LineFunc(
            41369,
            "virtual int Dispatcher_CompleteLoadAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    PvP_Room* const room = user->GetPVPRoom();
    if (room && room->current_match_) {
        room->current_match_->OnCompleteLoad(user);
        return 0;
    }

    if (pvp_assault::GetInstanceAssaultMgr()->OnCompleteLoad(user) == 1) {
        return 0;
    }
    return LineFunc(
        41384,
        "virtual int Dispatcher_CompleteLoadAssault::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233916
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompleteLoadAssault::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompleteLoadAssault::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233920
// IDA_NAME: _ZN30Dispatcher_CompleteLoadAssault7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompleteLoadAssault::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompleteLoadAssault::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85962D8
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssaultC2Ev
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::Dispatcher_CompleteLoadAfterAssault(void)
Dispatcher_CompleteLoadAfterAssault::Dispatcher_CompleteLoadAfterAssault()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x82427EA
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssaultD2Ev
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::~Dispatcher_CompleteLoadAfterAssault()
// IDA_EA: 0x824281A
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssaultD0Ev
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::~Dispatcher_CompleteLoadAfterAssault()
Dispatcher_CompleteLoadAfterAssault::~Dispatcher_CompleteLoadAfterAssault() {}

// IDA_EA: 0x8206B5C
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssault12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CompleteLoadAfterAssault::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 1) {
        return LineFunc(
            41297,
            "virtual int Dispatcher_CompleteLoadAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    if (user->GetCurCharacSuperState()) {
        G_TimerQueue()->InsertTimer(
            0,
            user->GetUID(),
            127,
            G_CDataManager()
                ->get_power_war_assault_superstate_after_duration_seconds(),
            static_cast<unsigned int>(user->GetLoginTick()),
            user->getCurCharacNo());
    }

    CParty* const party = user->GetParty();
    if (party && party->CheckRecvFromAllMember(user)) {
        PacketGuard response;
        response.clear();
        response.put_header(1, ENUM_CMDPACKET_LOAD_COMPLETE_AFTER_ASSAULT);
        response.put_byte(1);
        response.finalize(true);
        party->send_to_party(response);
        party->SetRecvResultFlag(true);
    }
    return 0;
}

// IDA_EA: 0x82338EE
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssault4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CompleteLoadAfterAssault::read(PacketBuf& packet,
                                              MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82338F8
// IDA_NAME: _ZN35Dispatcher_CompleteLoadAfterAssault7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CompleteLoadAfterAssault::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CompleteLoadAfterAssault::process(CUser* user,
                                                 MSG_BASE& message,
                                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596284
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssaultC2Ev
// DEMANGLED: Dispatcher_ConnectP2PAssault::Dispatcher_ConnectP2PAssault(void)
Dispatcher_ConnectP2PAssault::Dispatcher_ConnectP2PAssault() {}

// IDA_EA: 0x8242700
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssaultD2Ev
// DEMANGLED: Dispatcher_ConnectP2PAssault::~Dispatcher_ConnectP2PAssault()
// IDA_EA: 0x8242730
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssaultD0Ev
// DEMANGLED: Dispatcher_ConnectP2PAssault::~Dispatcher_ConnectP2PAssault()
Dispatcher_ConnectP2PAssault::~Dispatcher_ConnectP2PAssault() {}

// IDA_EA: 0x8206FEA
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssault12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ConnectP2PAssault::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(
            41392,
            "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(
            41396,
            "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (count > 8u) {
        return LineFunc(
            41400,
            "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char states[8] = {};
    for (uint8_t index = 0; index < count; ++index) {
        uint8_t slot = 0;
        if (packet.get_byte(slot) != 1) {
            return LineFunc(
                41407,
                "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (slot >= 8u) {
            return LineFunc(
                41410,
                "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        uint8_t state = 0;
        if (packet.get_byte(state) != 1) {
            return LineFunc(
                41413,
                "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        states[slot] = static_cast<char>(state);
    }

    if (pvp_assault::GetInstanceAssaultMgr()->OnConnectP2P(user, states, count) == 1) {
        return 0;
    }
    return LineFunc(
        41419,
        "virtual int Dispatcher_ConnectP2PAssault::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x823392A
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssault4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ConnectP2PAssault::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ConnectP2PAssault::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233934
// IDA_NAME: _ZN28Dispatcher_ConnectP2PAssault7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ConnectP2PAssault::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ConnectP2PAssault::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85962BC
// IDA_NAME: _ZN19Dispatcher_ChangeHpC2Ev
// DEMANGLED: Dispatcher_ChangeHp::Dispatcher_ChangeHp(void)
Dispatcher_ChangeHp::Dispatcher_ChangeHp() {}

// IDA_EA: 0x8242664
// IDA_NAME: _ZN19Dispatcher_ChangeHpD2Ev
// DEMANGLED: Dispatcher_ChangeHp::~Dispatcher_ChangeHp()
// IDA_EA: 0x8242694
// IDA_NAME: _ZN19Dispatcher_ChangeHpD0Ev
// DEMANGLED: Dispatcher_ChangeHp::~Dispatcher_ChangeHp()
Dispatcher_ChangeHp::~Dispatcher_ChangeHp() {}

// IDA_EA: 0x820748A
// IDA_NAME: _ZN19Dispatcher_ChangeHp12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ChangeHp::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ChangeHp::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(
            41478,
            "virtual int Dispatcher_ChangeHp::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    int encoded_hp = 0;
    if (packet.get_int(encoded_hp) != 1) {
        return LineFunc(
            41482,
            "virtual int Dispatcher_ChangeHp::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    // The IDA body stores the dword then forwards its low signed byte.
    const int hp = static_cast<int>(static_cast<int8_t>(encoded_hp));
    if (pvp_assault::GetInstanceAssaultMgr()->OnChangeHp(user, hp) == 1) {
        return 0;
    }
    return LineFunc(
        41485,
        "virtual int Dispatcher_ChangeHp::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233952
// IDA_NAME: _ZN19Dispatcher_ChangeHp4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeHp::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeHp::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823395C
// IDA_NAME: _ZN19Dispatcher_ChangeHp7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeHp::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeHp::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85962F4
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssaultC2Ev
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::Dispatcher_ConnectP2PAfterAssault(void)
Dispatcher_ConnectP2PAfterAssault::Dispatcher_ConnectP2PAfterAssault() {}

// IDA_EA: 0x824279C
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssaultD2Ev
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::~Dispatcher_ConnectP2PAfterAssault()
// IDA_EA: 0x82427CC
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssaultD0Ev
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::~Dispatcher_ConnectP2PAfterAssault()
Dispatcher_ConnectP2PAfterAssault::~Dispatcher_ConnectP2PAfterAssault() {}

// IDA_EA: 0x8206CFA
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssault12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() <= 1) {
        return LineFunc(
            41327,
            "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(
            41331,
            "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (count > 4u) {
        return LineFunc(
            41335,
            "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char states[4] = {};
    for (uint8_t index = 0; index < count; ++index) {
        uint8_t slot = 0;
        if (packet.get_byte(slot) != 1) {
            return LineFunc(
                41342,
                "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        if (slot >= 4u) {
            return LineFunc(
                41346,
                "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        uint8_t state = 0;
        if (packet.get_byte(state) != 1) {
            return LineFunc(
                41353,
                "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        states[slot] = static_cast<char>(state);
    }

    if (pvp_assault::GetInstanceAssaultMgr()->OnConnectP2PAfterAssault(user, states, count) == 1) {
        return 0;
    }
    return LineFunc(
        41359,
        "virtual int Dispatcher_ConnectP2PAfterAssault::dispatch_sig(CUser*, PacketBuf&)",
        0,
        0);
}

// IDA_EA: 0x8233902
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssault4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ConnectP2PAfterAssault::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x823390C
// IDA_NAME: _ZN33Dispatcher_ConnectP2PAfterAssault7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ConnectP2PAfterAssault::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ConnectP2PAfterAssault::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596D74
// IDA_NAME: _ZN26Dispatcher_DieBloodMonsterC2Ev
// DEMANGLED: Dispatcher_DieBloodMonster::Dispatcher_DieBloodMonster(void)
Dispatcher_DieBloodMonster::Dispatcher_DieBloodMonster() {}

// IDA_EA: 0x8240F8A
// IDA_NAME: _ZN26Dispatcher_DieBloodMonsterD2Ev
// DEMANGLED: Dispatcher_DieBloodMonster::~Dispatcher_DieBloodMonster()
// IDA_EA: 0x8240FBA
// IDA_NAME: _ZN26Dispatcher_DieBloodMonsterD0Ev
// DEMANGLED: Dispatcher_DieBloodMonster::~Dispatcher_DieBloodMonster()
Dispatcher_DieBloodMonster::~Dispatcher_DieBloodMonster() {}

// IDA_EA: 0x8220856
// IDA_NAME: _ZN26Dispatcher_DieBloodMonster12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DieBloodMonster::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DieBloodMonster::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user) {
        return LineFunc(
            55883,
            "virtual int Dispatcher_DieBloodMonster::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(
            55884,
            "virtual int Dispatcher_DieBloodMonster::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint8_t count = 0;
    if (packet.get_byte(count) != 1) {
        return LineFunc(
            55889,
            "virtual int Dispatcher_DieBloodMonster::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    uint16_t monster_ids[256] = {};
    for (uint16_t index = 0; index < count; ++index) {
        if (packet.get_short(monster_ids[index]) != 1) {
            return LineFunc(
                55892,
                "virtual int Dispatcher_DieBloodMonster::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
    }

    if (user->get_state() == 5) {
        CParty* const party = G_CGameManager()->GetParty(user->GetPartyIndex());
        if (!party) {
            return LineFunc(
                55898,
                "virtual int Dispatcher_DieBloodMonster::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }
        party->kill_blood_monster(user, static_cast<int>(count), monster_ids);
    }
    return 0;
}

// IDA_EA: 0x8233F56
// IDA_NAME: _ZN26Dispatcher_DieBloodMonster4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DieBloodMonster::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DieBloodMonster::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F60
// IDA_NAME: _ZN26Dispatcher_DieBloodMonster7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DieBloodMonster::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DieBloodMonster::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596DAC
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHackC2Ev
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::Dispatcher_CheckAssaultMotionHack(void)
Dispatcher_CheckAssaultMotionHack::Dispatcher_CheckAssaultMotionHack() {}

// IDA_EA: 0x8240EEE
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHackD2Ev
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::~Dispatcher_CheckAssaultMotionHack()
// IDA_EA: 0x8240F1E
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHackD0Ev
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::~Dispatcher_CheckAssaultMotionHack()
Dispatcher_CheckAssaultMotionHack::~Dispatcher_CheckAssaultMotionHack() {}

// IDA_EA: 0x8221F3C
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHack12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CheckAssaultMotionHack::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (!user) {
        return LineFunc(
            56179,
            "virtual int Dispatcher_CheckAssaultMotionHack::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (!user->getCurCharacR()) {
        return 0;
    }

    uint32_t motion_value = 0;
    uint32_t reported_value = 0;
    if (static_cast<unsigned char>(packet.get_int(motion_value)) != 1u) {
        return LineFunc(
            56190,
            "virtual int Dispatcher_CheckAssaultMotionHack::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (static_cast<unsigned char>(packet.get_int(reported_value)) != 1u) {
        return LineFunc(
            56191,
            "virtual int Dispatcher_CheckAssaultMotionHack::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const bool power_war_village =
        static_cast<unsigned char>(CPowerManager::IsPowerWarEventOn()) != 0u &&
        static_cast<unsigned char>(user->getCurCharacVill()) == 7u;
    if (motion_value > 10000u) {
        const uint32_t quotient = motion_value / 10000u;
        motion_value %= 10000u;
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(
                power_war_village ? 0x7Cu : 0x7Au),
            1u,
            quotient,
            0u);
    }

    if (user->get_charac_job() == oldsrv::config::kCharacterJobMage ||
        (user->get_charac_job() == oldsrv::config::kCharacterJobPriest &&
         user->getCurCharFirstGrowType() == 2)) {
        motion_value = static_cast<uint32_t>(
            static_cast<long double>(motion_value) * 0.5L);
    } else {
        motion_value = static_cast<uint32_t>(
            static_cast<long double>(motion_value) * 0.8L);
    }

    if (reported_value < motion_value) {
        user->getHackAnalyzer()->addServerHackCnt(
            user,
            static_cast<WongWork::ENUM_HACKTYPE>(
                power_war_village ? 0x7Bu : 0x79u),
            1u,
            motion_value,
            0u);
    }
    return 0;
}

// IDA_EA: 0x8233F7E
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHack4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CheckAssaultMotionHack::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F88
// IDA_NAME: _ZN33Dispatcher_CheckAssaultMotionHack7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CheckAssaultMotionHack::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CheckAssaultMotionHack::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596B28
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoC2Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::Dispatcher_PowerWarProcessInfo(void)
Dispatcher_PowerWarProcessInfo::Dispatcher_PowerWarProcessInfo() {}

// IDA_EA: 0x82411FA
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoD2Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo()
// IDA_EA: 0x824122A
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfoD0Ev
// DEMANGLED: Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo()
Dispatcher_PowerWarProcessInfo::~Dispatcher_PowerWarProcessInfo() {}

// IDA_EA: 0x821EA32
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    (void)packet;
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            55073,
            "virtual int Dispatcher_PowerWarProcessInfo::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    const unsigned int charac_no =
        static_cast<unsigned int>(user->get_charac_no(-1));
    CGuildServerProxy* const proxy =
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPowerWarProcessInfo(charac_no);

    PacketGuard result;
    result.put_header(1, ENUM_CMDPACKET_POWER_WAR_PROCESS_INFO);
    result.put_byte(1);
    result.finalize(true);
    user->Send(result);
    return 0;
}

// IDA_EA: 0x8233EB6
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_PowerWarProcessInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_PowerWarProcessInfo::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233EC0
// IDA_NAME: _ZN30Dispatcher_PowerWarProcessInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_PowerWarProcessInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_PowerWarProcessInfo::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596FC0
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddyC2Ev
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::Dispatcher_RequestAddPvPBuddy(void)
Dispatcher_RequestAddPvPBuddy::Dispatcher_RequestAddPvPBuddy() {}

// IDA_EA: 0x82409C0
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddyD2Ev
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::~Dispatcher_RequestAddPvPBuddy()
// IDA_EA: 0x82409F0
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddyD0Ev
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::~Dispatcher_RequestAddPvPBuddy()
Dispatcher_RequestAddPvPBuddy::~Dispatcher_RequestAddPvPBuddy() {}

// IDA_EA: 0x822490A
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddy12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_RequestAddPvPBuddy::dispatch_sig(CUser* user,
                                                 PacketBuf& packet) {
    if (!G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        return 0;
    }

    uint8_t request_kind = 0;
    int target_name_size = 0;
    char target_name[30] = {};
    if (packet.get_byte(request_kind) != 1) {
        return LineFunc(
            57148,
            "virtual int Dispatcher_RequestAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_int(target_name_size) != 1) {
        return LineFunc(
            57151,
            "virtual int Dispatcher_RequestAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_str(target_name, sizeof(target_name), target_name_size) !=
        1) {
        return LineFunc(
            57154,
            "virtual int Dispatcher_RequestAddPvPBuddy::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    GlobalData::s_community_proxy->SendReqAddPvPBuddy(
        static_cast<uint32_t>(user->get_acc_id()),
        static_cast<uint32_t>(user->getCurCharacNo()),
        static_cast<char>(request_kind),
        target_name,
        target_name_size);
    return 0;
}

// IDA_EA: 0x82340D2
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddy4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestAddPvPBuddy::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82340DC
// IDA_NAME: _ZN29Dispatcher_RequestAddPvPBuddy7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestAddPvPBuddy::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestAddPvPBuddy::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
