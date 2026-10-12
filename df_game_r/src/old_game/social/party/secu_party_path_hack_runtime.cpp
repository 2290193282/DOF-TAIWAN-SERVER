#include "old_game/social/party/party.h"

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/global_tools.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"

// IDA_EA: 0x827A96E
// IDA_NAME: _ZN24Secu_HackLogCheckByParty28CheckMoveMapFitInPathHackCntEP5CUserP6CPartyjji
// DEMANGLED: Secu_HackLogCheckByParty::CheckMoveMapFitInPathHackCnt(CUser *,CParty *,unsigned int,unsigned int,int)
void Secu_HackLogCheckByParty::CheckMoveMapFitInPathHackCnt(
    CUser* user,
    CParty* party,
    unsigned int client_x,
    unsigned int client_y,
    int old_map_index) {
    if (!party || !user) {
        return;
    }

    if (party->IsFirstMapClear() == 1) {
        CMap* const map = G_CDataManager()->find_map(old_map_index);
        if ((party->IsAutoCreated() || party->get_member_count() == 1) &&
            map && map->checkFitInPathObject(
                       static_cast<int>(client_x),
                       static_cast<int>(client_y)) != 1) {
            LogManager::logFormat(
                1,
                "Secu_HackLogCheckByParty.cpp",
                "void Secu_HackLogCheckByParty::CheckMoveMapFitInPathHackCnt(CUser*, CParty*, unsigned int, unsigned int, int)",
                44,
                "Path gate position error. (User: %s %d,%d)",
                NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
                client_x,
                client_y);
            user->getHackAnalyzer()->addServerHackCnt(
                user,
                static_cast<WongWork::ENUM_HACKTYPE>(0x1f7),
                1,
                client_x,
                client_y);
        }
    } else {
        party->SetFirstMapClear(true);
    }
}
