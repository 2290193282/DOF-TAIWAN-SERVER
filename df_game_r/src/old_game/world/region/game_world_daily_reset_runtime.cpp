#include "old_game/world/region/game_world.h"

#include "infra/log/format/trace.h"
#include "old_data/db/auth_security/db_passpad_update_fail_cnt.h"
#include "old_data/PVF/map/datamanager/dungeon_runtime.h"
#include "old_game/progression/quest/quest_state_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x86CC8AC
// IDA_NAME: _ZN9GameWorld21ResetAllCleadpadPointEv
// DEMANGLED: GameWorld::ResetAllCleadpadPoint(void)
int GameWorld::ResetAllCleadpadPoint() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() > 2) {
            user->getMCAPManagerW().reset();
        }
    }

    cMyTrace trace("void GameWorld::ResetAllCleadpadPoint()", 4997, 0);
    return trace("RESET ALL USER CLEANPAD POINT OK!!!!");
}

// IDA_EA: 0x86CC992
// IDA_NAME: _ZN9GameWorld24ResetAllPassPadFailCountEv
// DEMANGLED: GameWorld::ResetAllPassPadFailCount(void)
int GameWorld::ResetAllPassPadFailCount() {
    int reset_count = 0;
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() <= 2) {
            continue;
        }

        Sanicova::CPad* const pad = user->getPad();
        if (pad->isActivate() && pad->getFailCnt() > 0) {
            pad->setFailCnt(0);
            DB_PassPadUpdateFailCnt::makeRequest(
                static_cast<uint32_t>(user->get_acc_id()),
                static_cast<uint32_t>(pad->getFailCnt()),
                user->getWebAddress());
            ++reset_count;
        }
    }

    cMyTrace trace("void GameWorld::ResetAllPassPadFailCount()", 5021, 0);
    return trace("RESET %d USERS GOBLIN PAD RESETT OK!!!!", reset_count);
}

// IDA_EA: 0x86CCB22
// IDA_NAME: _ZN9GameWorld13ResetAllCoinsEv
// DEMANGLED: GameWorld::ResetAllCoins(void)
bool GameWorld::ResetAllCoins() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() > 2) {
            user->RecoverCoin(1u);
            if (!user->GetOpencoin()) {
                user->SendOpenflag();
            }
        }
    }
    return false;
}

// IDA_EA: 0x86CCDCE
// IDA_NAME: _ZN9GameWorld12AddDailyItemEv
// DEMANGLED: GameWorld::AddDailyItem(void)
bool GameWorld::AddDailyItem() {
    for (GameWorldUserMap::iterator it = users_in_world_.begin();
         it != users_in_world_.end(); ++it) {
        CUser* const user = it->second;
        if (user->get_state() <= 2) {
            continue;
        }

        user->AddDailyItem();
        if (user->getCurCharacR()) {
            user->ResetDailyQuest();
            user->ResetTrainingQuest();

            PacketGuard packet;
            user->getCurCharacQuestR()->get_quest_info(
                reinterpret_cast<char*>(&packet));
            user->Send(packet);
        }

        if (user->getCurCharacR()) {
            bool refresh_incount = false;
            if (user->get_charac_level() > 59) {
                user->DimensionInoutUpdate(true, true);
                refresh_incount = true;
            }
            if (dungeon_runtime::IsInCountLoaded(user)) {
                dungeon_runtime::ResetDailyInCount(user);
                refresh_incount = true;
            } else {
                dungeon_runtime::MarkDailyResetPending(user);
            }
            if (refresh_incount) {
                user->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                     ENUM_NOTIPACKET_USERINFO,
                                     1);
            }
        }
    }
    return false;
}
