#include "old_game/gm/progression/game_master_level_up.h"
#include "infra/network/packet/PacketEnum.h"

#include <algorithm>

#include "common/config.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {

// DisPatcher_DebugCommand::_debugCommandSetLevel @0x858EFDE has no object
// state: its C2/D2 only construct/destroy the IPacketDispatcher base and set
// the vtable.  The CLevelUp string command is its only named-command caller.
// Keep the original operation here with its real CUser/quest/packet owners,
// rather than retaining a second synthetic debug-dispatcher object solely to
// reach this stateless member body.
bool ApplyGameMasterLevel(CUser* user, int level) {
    if (user->isGMUser() != 1 ||
        level > oldsrv::config::kCharacterLevelMaximum) {
        return false;
    }

    bool level_changed = true;
    const int delta = level - user->get_charac_level();
    if (delta <= 0) {
        for (int step = 0; step > delta; --step) {
            user->SetCharacLevel(
                std::min(200, user->get_charac_level() - 1));
        }
    } else {
        for (int step = 0; step < delta; ++step) {
            user->SetCharacLevel(
                std::min(200, user->get_charac_level() + 1));
        }
    }
    if (delta == 0) {
        level_changed = false;
    }

    // The old body deliberately fires this level-up hook even for delta == 0.
    user->_onLevelUp();
    user->setCurCharacExp(
        G_CDataManager()->get_level_exp(user->get_charac_level()));

    if (!level_changed) {
        return false;
    }

    user->CalcurateUserMaxLevel();
    PacketGuard mercenary_packet;
    mercenary_packet.put_header(1, ENUM_CMDPACKET_MERCENARY_INFO);
    mercenary_packet.put_byte(1);
    user->SetMercenaryInfoPacket(mercenary_packet);
    mercenary_packet.finalize(true);
    user->Send(mercenary_packet);

    if (user->GetCurCharacMaxEquipLevel() &&
        user->get_charac_level() == user->GetCurCharacMaxEquipLevel()) {
        user->SetCurCharacMaxEquipLevel(0);
    }

    const int state = user->get_state();
    if (state == 3) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                             static_cast<ENUM_NOTIPACKET>(2),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    } else if (state == 5) {
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                             static_cast<ENUM_NOTIPACKET>(37),
                             0);
        user->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                             static_cast<ENUM_NOTIPACKET>(2),
                             1);
        PacketGuard quest_packet;
        user->getCurCharacQuestR()->get_quest_info(
            reinterpret_cast<char*>(&quest_packet));
        user->Send(quest_packet);
    }
    return true;
}

}  // namespace

namespace game_master {

// IDA_EA: 0x82A8956
// IDA_NAME: _ZN11game_master8CLevelUpC2Ev
// DEMANGLED: game_master::CLevelUp::CLevelUp(void)
CLevelUp::CLevelUp() : CCommand(), level_(0) {}

// IDA_EA: 0x84B388C
// IDA_NAME: _ZN11game_master8CLevelUp5parseEv
// DEMANGLED: game_master::CLevelUp::parse(void)
void CLevelUp::parse() {
    if (parser_) {
        setLevel(static_cast<int>(
            StringToNumber(parser_->token_vector()[1].c_str())));
    }
}

// IDA_EA: 0x84B38E2
// IDA_NAME: _ZN11game_master8CLevelUp7executeEv
// DEMANGLED: game_master::CLevelUp::execute(void)
void CLevelUp::execute() {
    CUser* const user = GetUser();
    if (user) {
        ApplyGameMasterLevel(user, getLevel());
    }
}

// IDA_EA: 0x84B395C
// IDA_NAME: _ZN11game_master8CLevelUp8setLevelEi
// DEMANGLED: game_master::CLevelUp::setLevel(int)
CLevelUp* CLevelUp::setLevel(int level) {
    if (level <= oldsrv::config::kCharacterLevelMaximum) {
        level_ = level >= oldsrv::config::kCharacterLevelMinimum
            ? level : oldsrv::config::kCharacterLevelMinimum;
    } else {
        level_ = oldsrv::config::kCharacterLevelMaximum;
    }
    return this;
}

// IDA_EA: 0x84B398E
// IDA_NAME: _ZN11game_master8CLevelUp8getLevelEv
// DEMANGLED: game_master::CLevelUp::getLevel(void)
int CLevelUp::getLevel() {
    return level_;
}

}  // namespace game_master
