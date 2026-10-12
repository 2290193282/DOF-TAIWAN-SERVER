#include "old_game/activity/schedule/advance_altar_runtime.h"

#include "common/manager/game_manager.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/advance_altar_stage_control_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/packet_guard.h"

// IDA data: 0x09370094 contains 1 in the original 0627 image.
int advancealtar::Manager::gmTimeSpeed_ = 1;

// IDA_EA: 0x813090E
// IDA_NAME: _ZN12advancealtar7Manager11selectStageEP5CUseriNS_15StageDifficulty1TE
// DEMANGLED: advancealtar::Manager::selectStage(CUser *,int,advancealtar::StageDifficulty::T)
int advancealtar::Manager::selectStage(CUser* user,
                                       int stage_index,
                                       int difficulty) {
    if (!user) {
        return 3;
    }
    if (difficulty > 2) {
        return 8;
    }

    StageControl* const stage_control = G_CGameManager()->getAdvanceAltar();
    if (!stage_control) {
        cMyTrace trace(
            "static ENUM_ERROR advancealtar::Manager::selectStage(CUser*, int, advancealtar::StageDifficulty::T)",
            1707, 5);
        trace("StaticPool Defence DungeonNULL");
        return 3;
    }

    const int result =
        stage_control->selectStage(user, stage_index, difficulty);
    if (result != 0) {
        stage_control->destroy();
        return result;
    }
    user->setAdvanceAltarIndex(
        static_cast<short>(stage_control->getIndex()));
    return 0;
}

// IDA_EA: 0x81309F0
// IDA_NAME: _ZN12advancealtar7Manager20procErrorSelectStageEP5CUser10ENUM_ERROR
// DEMANGLED: advancealtar::Manager::procErrorSelectStage(CUser *,ENUM_ERROR)
void advancealtar::Manager::procErrorSelectStage(CUser* user, int error) {
    if (!user || error == 0) {
        return;
    }

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      static_cast<ENUM_CMDPACKET>(16));
    packet.put_byte(0);
    packet.put_int(error);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x8130AB2
// IDA_NAME: _ZN12advancealtar7Manager9dieCharacEP5CUser
// DEMANGLED: advancealtar::Manager::dieCharac(CUser *)
void advancealtar::Manager::dieCharac(CUser* user) {
    if (!user) {
        return;
    }

    StageControl* const stage_control = user->getAdvanceAltar();
    if (!stage_control) {
        return;
    }

    ProcStage* const stage = stage_control->getProcStage();
    if (!stage || !stage->isRewardState()) {
        return;
    }

    if (stage->getStageType() == StageType::SURVIVAL) {
        stage->clearStageAndReward();
        stage->setState(static_cast<ProcStage::State::T>(5));
        Timer_StageControl::RegistTimerClearReward(
            stage_control->getIndex(), static_cast<StageEndType::T>(1));
        return;
    }

    stage->setState(static_cast<ProcStage::State::T>(6));
    Timer_StageControl::RegistTimerEPLP(
        stage_control->getIndex(), static_cast<StageEndType::T>(1));
    HistoryLog::dungeonClearInfo(*user, *stage_control, false);
}

// IDA_EA: 0x8130EB2
// IDA_NAME: _ZN12advancealtar7Manager10finishLoadEP5CUser
// DEMANGLED: advancealtar::Manager::finishLoad(CUser *)
int advancealtar::Manager::finishLoad(CUser* user) {
    StageControl* const stage_control = user->getAdvanceAltar();
    if (!stage_control) {
        return 0;
    }
    if (!user->advance_altar_manager_.isAvailableTciket()) {
        return 1;
    }
    stage_control->getProcStage()->setState(
        static_cast<ProcStage::State::T>(1));
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_FINISH_LOADING);
    packet.put_int(0);
    packet.put_byte(0);
    packet.finalize(true);
    user->Send(packet);
    return 1;
}

// IDA_EA: 0x8130FA8
// IDA_NAME: _ZN12advancealtar7Manager10giveUpGameEP5CUser
// DEMANGLED: advancealtar::Manager::giveUpGame(CUser *)
void advancealtar::Manager::giveUpGame(CUser* user) {
    if (!user) {
        return;
    }
    StageControl* const stage_control = user->getAdvanceAltar();
    if (!stage_control) {
        return;
    }
    stage_control->getProcStage()->setState(
        static_cast<ProcStage::State::T>(6));
    Timer_StageControl::RegistTimerEPLP(
        stage_control->getIndex(), static_cast<StageEndType::T>(2));
    HistoryLog::dungeonClearInfo(*user, *stage_control, false);
}

// IDA_EA: 0x813101E
// IDA_NAME: _ZN12advancealtar7Manager4eplpEP5CUserNS_8EplpType1TE
// DEMANGLED: advancealtar::Manager::eplp(CUser *,advancealtar::EplpType::T)
void advancealtar::Manager::eplp(CUser* user, EplpType::T command) {
    if (!user) {
        return;
    }

    StageControl* const stage_control = user->getAdvanceAltar();
    if (!stage_control) {
        return;
    }

    if (static_cast<unsigned int>(command) <= 2u) {
        stage_control->leaveUser();
    }
}

// IDA_EA: 0x813105C
// IDA_NAME: _ZN12advancealtar7Manager5pauseEP5CUserb
// DEMANGLED: advancealtar::Manager::pause(CUser *,bool)
int advancealtar::Manager::pause(CUser* user, bool paused) {
    if (!user) {
        return 0;
    }
    StageControl* const stage_control = user->getAdvanceAltar();
    if (!stage_control) {
        return 0;
    }
    return stage_control->getProcStage()->pause(paused);
}

// IDA_EA: 0x81310B2
// IDA_NAME: _ZN12advancealtar7Manager9leaveUserEP5CUser
// DEMANGLED: advancealtar::Manager::leaveUser(CUser *)
void advancealtar::Manager::leaveUser(CUser* user) {
    if (!user) {
        return;
    }

    StageControl* const stage_control = user->getAdvanceAltar();
    if (stage_control && stage_control->charac_data.getUser() == user) {
        (void)stage_control->leaveUser();
        return;
    }

    // A stale index must not survive an abnormal leave and later resolve to
    // a recycled StageControl owned by another character.
    user->setAdvanceAltarIndex(-1);
    GameWorld* const world = G_GameWorld();
    world->out_from_dungeon(user);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, 2);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    world->send_all(packet, user);
}
