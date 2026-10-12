#include "old_game/gm/dungeon/game_master_pvp_dungeon_commands.h"

#include "common/config.h"
#include <cstdlib>
#include <string>

#include "old_data/PVF/map/datamanager/map_runtime.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_game/pvp/match/fair_pvp_match_interface.h"
#include "old_game/pvp/arena/pvp_room.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"

namespace game_master {

// IDA_EA: 0x82A8028
// IDA_NAME: _ZN11game_master12CPvPStartCmdC2Ev
// DEMANGLED: game_master::CPvPStartCmd::CPvPStartCmd(void)
CPvPStartCmd::CPvPStartCmd() : CCommand() {}

// IDA_EA: 0x84A8F6A
// IDA_NAME: _ZN11game_master12CPvPStartCmd7executeEv
// DEMANGLED: game_master::CPvPStartCmd::execute(void)
void CPvPStartCmd::execute() {
    CUser* const user = GetUser();
    PvP_Room* const room = user->GetPVPRoom();
    if (!room) {
        LogManager::logFormat(
            1,
            "GameMaster.cpp",
            "virtual void game_master::CPvPStartCmd::execute()",
            0x169,
            "PVPRoom pRoom is NULL");
        return;
    }

    // 0x84A8FBE loads PvP_Room::current_match_ (+0x6e4), then dispatches
    // IMatch vtable +0x30, which is OnRegisterMatch(CUser *, PvP_Room *).
    // The original asks the command for the user again here.
    room->current_match_->OnRegisterMatch(GetUser(), room);
}

// IDA_EA: 0x82A84B2
// IDA_NAME: _ZN11game_master18CItemDropInDungeonC2Ev
// DEMANGLED: game_master::CItemDropInDungeon::CItemDropInDungeon(void)
CItemDropInDungeon::CItemDropInDungeon() : CCommand() {
    // The original C2 leaves +0x08 untouched.  CGameMasterMgr zeroes the
    // allocation before this constructor and parse() owns its next write.
}

// IDA_EA: 0x84ABCB0
// IDA_NAME: _ZN11game_master18CItemDropInDungeon5parseEv
// DEMANGLED: game_master::CItemDropInDungeon::parse(void)
void CItemDropInDungeon::parse() {
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    item_id_ = std::atoi(command_tail.c_str());
}

// IDA_EA: 0x84ABCE8
// IDA_NAME: _ZN11game_master18CItemDropInDungeon7executeEv
// DEMANGLED: game_master::CItemDropInDungeon::execute(void)
void CItemDropInDungeon::execute() {
    const unsigned int item_id = static_cast<unsigned int>(item_id_);
    CItem* const item = G_CDataManager()->find_item(item_id);
    if (!item) {
        return;
    }

    const int item_type = item->vtable_->get_item_type(item);
    if (item->is_stackable() ||
        item_type == oldsrv::config::kEquipmentTypeCreature ||
        (item_type >= oldsrv::config::kEquipmentTypeArtifactRed &&
         item_type <= oldsrv::config::kEquipmentTypeArtifactGreen)) {
        return;
    }

    CUser* const user = GetUser();
    if (!user) {
        return;
    }

    CParty* const party = user->GetParty();
    if (party) {
        party->drop_item_gm(user, item_id);
    }
}

// IDA_EA: 0x82A8892
// IDA_NAME: _ZN11game_master17DungeonDifficultyC2Ev
// DEMANGLED: game_master::DungeonDifficulty::DungeonDifficulty(void)
DungeonDifficulty::DungeonDifficulty() : CCommand() {
    // 0x82A8892 leaves the byte at +0x08 untouched.  parse() first clears it
    // and CGameMasterMgr pre-zeroes its allocation.
}

// IDA_EA: 0x84B2110
// IDA_NAME: _ZN11game_master17DungeonDifficulty5parseEv
// DEMANGLED: game_master::DungeonDifficulty::parse(void)
void DungeonDifficulty::parse() {
    difficulty_ = 0;
    std::string& command_tail = parser_->command_tail();
    trim(command_tail);
    difficulty_ = static_cast<uint8_t>(std::atoi(command_tail.c_str()));
}

// IDA_EA: 0x84B2152
// IDA_NAME: _ZN11game_master17DungeonDifficulty7executeEv
// DEMANGLED: game_master::DungeonDifficulty::execute(void)
void DungeonDifficulty::execute() {
    CUser* const user = GetUser();
    const uint8_t difficulty = difficulty_;
    if (!user || difficulty == 0u || difficulty > 3u) {
        return;
    }

    CDungeonList* const dungeons = G_CDataManager()->getDungeonIndexList();
    for (CDungeonList::const_iterator it = dungeons->begin();
         it != dungeons->end(); ++it) {
        CDungeon* const dungeon = it->second;
        if (!dungeon || dungeon->get_blood_dungeon_type() == 1 ||
            dungeon->get_blood_dungeon_type() == 2 ||
            dungeon->isTowerOfDespairDungeon() ||
            dungeon->isTournamentDungeon() || dungeon->getDungeonKind() == 1) {
            continue;
        }

        // 0x84B224D..0x84B2274 repeats getDungeonClearRefW() for each
        // eligible entry, then passes the short dungeon index and raw byte.
        user->getDungeonClearRefW()->addClearedDungeon(
            static_cast<short>(dungeon->get_index()),
            static_cast<char>(difficulty));
    }
}

}  // namespace game_master
