#include "old_game/dungeon/instance/dungeon_pvp_dispatchers.h"

#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/battle_field.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

// IDA_EA: 0x859640C
// IDA_NAME: _ZN33Dispatcher_DeathTowerStageCommandC2Ev
// DEMANGLED: Dispatcher_DeathTowerStageCommand::Dispatcher_DeathTowerStageCommand(void)
Dispatcher_DeathTowerStageCommand::Dispatcher_DeathTowerStageCommand()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {
}

// IDA_EA: 0x8242358
// IDA_NAME: _ZN33Dispatcher_DeathTowerStageCommandD2Ev
// DEMANGLED: Dispatcher_DeathTowerStageCommand::~Dispatcher_DeathTowerStageCommand()
Dispatcher_DeathTowerStageCommand::~Dispatcher_DeathTowerStageCommand() {
}

// IDA_EA: 0x8208A9E
// IDA_NAME: _ZN33Dispatcher_DeathTowerStageCommand12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DeathTowerStageCommand::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DeathTowerStageCommand::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    if (user->get_state() != 10) {
        return 0;
    }

    unsigned char command = 0;
    if (packet.get_byte(command) != 1) {
        return LineFunc(
            41970,
            "virtual int Dispatcher_DeathTowerStageCommand::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    WongWork::CDeathTower* const tower = user->getDeathTower();
    if (tower) {
        if (tower->handleStageCommand(user, command) != 1) {
            return LineFunc(
                41977,
                "virtual int Dispatcher_DeathTowerStageCommand::dispatch_sig(CUser*, PacketBuf&)",
                0,
                0);
        }

        CParty* const party = user->GetParty();
        if (party && command == 2) {
            WongWork::CDeathTower::CStage* const stage = tower->getCStage();
            stage->onFinishStage();

            // CLIENT_IDA_EVIDENCE: the native stage-finish sender at
            // 0x1240FE0 writes eight uint16 map-path results (0x12410AF-
            // 0x12410E3).  Preserve the wire slots exactly; the retail
            // four-member party state machine consumes only slots 0..3.
            unsigned short member_map_paths[8] = {0};
            for (int wire_slot = 0; wire_slot < 8; ++wire_slot) {
                if (packet.get_short(member_map_paths[wire_slot]) != 1) {
                    return LineFunc(
                        41989,
                        "virtual int Dispatcher_DeathTowerStageCommand::dispatch_sig(CUser*, PacketBuf&)",
                        0,
                        0);
                }
            }

            for (int member_slot = 0; member_slot < 4; ++member_slot) {
                if (!party->checkValidUser(member_slot)) {
                    continue;
                }

                CUser* const member = party->get_user(member_slot);
                const unsigned short map_path =
                    member_map_paths[member_slot];
                if (map_path == 0 && party->IsSinglePlay()) {
                    member->getHackAnalyzer()->addServerHackCnt(
                        member,
                        static_cast<WongWork::ENUM_HACKTYPE>(0xd5),
                        1,
                        0,
                        0);
                }

                // This is the exact physical CDeathTower+0xb58 member used
                // by the IDA body.  CDeathTower grants this class friendship
                // instead of routing the operation through a new facade.
                member->getDeathTower()->map_monster_kill_checker_.init();
                const unsigned int elapsed_seconds =
                    static_cast<unsigned int>(stage->getStageClearTime()) /
                    1000u;
                member->history_log().MapClear(map_path,
                                               elapsed_seconds,
                                               0);
            }
        }
    }
    return 0;
}

// IDA_EA: 0x8233A1A
// IDA_NAME: _ZN33Dispatcher_DeathTowerStageCommand4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DeathTowerStageCommand::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DeathTowerStageCommand::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233A24
// IDA_NAME: _ZN33Dispatcher_DeathTowerStageCommand7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DeathTowerStageCommand::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DeathTowerStageCommand::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
