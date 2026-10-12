#include "old_data/db/npc/db_update_quest_shop_buying_status_list.h"
#include "old_data/db/npc/quest_shop_persistence_layout.h"
#include "old_data/db/core/db_mgr.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>

namespace {

enum {
    kSigQuestShopDataSize = 0x155,
};

#define DB_QUEST_SHOP_U32(payload, offset) \
    (*reinterpret_cast<uint32_t*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844F7BE
// IDA_NAME: _ZN34DB_UpdateQuestShopBuyingStatusListC2Ev
// DEMANGLED: DB_UpdateQuestShopBuyingStatusList::DB_UpdateQuestShopBuyingStatusList(void)
DB_UpdateQuestShopBuyingStatusList::DB_UpdateQuestShopBuyingStatusList() {}

// IDA_EA: 0x845FB86
// IDA_NAME: _ZN34DB_UpdateQuestShopBuyingStatusListD2Ev
// DEMANGLED: DB_UpdateQuestShopBuyingStatusList::~DB_UpdateQuestShopBuyingStatusList()
// IDA_EA: 0x845FBB6
// IDA_NAME: _ZN34DB_UpdateQuestShopBuyingStatusListD0Ev
// DEMANGLED: DB_UpdateQuestShopBuyingStatusList::~DB_UpdateQuestShopBuyingStatusList() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateQuestShopBuyingStatusList::~DB_UpdateQuestShopBuyingStatusList() {}

// IDA_EA: 0x840568A
// IDA_NAME: _ZN34DB_UpdateQuestShopBuyingStatusList8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateQuestShopBuyingStatusList::dispatch(int,int,Stream *)
bool DB_UpdateQuestShopBuyingStatusList::dispatch(int signal,
                                                  int user_idx,
                                                  Stream* stream) {
    (void)signal;
    (void)user_idx;

    if (!stream || !GlobalData::s_db_mgr) {
        return false;
    }
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2), static_cast<ENUM_SERVER_GROUP>(0));
    if (!db_handle) {
        return false;
    }
    unsigned char payload[kSigQuestShopDataSize];
    if (stream->get_binary(payload, sizeof(payload)) != 1) {
        return false;
    }
    uint16_t buying_status[
        quest_shop_persistence::kBuyingStatusColumnCount];
    for (int column = 0;
         column < quest_shop_persistence::kBuyingStatusColumnCount;
         ++column) {
        buying_status[column] =
            quest_shop_persistence::ReadBuyingStatusColumn(payload, column);
    }

    // UPDATE reports zero affected rows both when the row is absent and when
    // all persisted values are already identical.  Keep the original single
    // row keyed by charac_no, but express create/reselect/repeat-save as one
    // idempotent database operation so an unchanged QP layout can never fall
    // through to a duplicate-key INSERT.
    if (!db_handle->set_query(
        "inSert into charac_quest_shop("
        "charac_no, qp, init_count, max_hp, max_mp, psy_attack, psy_defense, "
        "mag_attack, mag_defence, move_speed, attack_speed, hp_regen, mp_regen, "
        "all_element_resist, fire_element_resist, water_element_resist, "
        "light_element_resist, dark_element_resist, all_element_attack, "
        "fire_element_attack, water_element_attack, light_element_attack, "
        "dark_element_attack, psy_critical, mag_critical, good_hit, evasion, "
        "hit_recovery, separate_psy_mag_attack, quest_piece) "
        "values(%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
        "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u) "
        "on duplicate key upDate "
        "qp=values(qp), init_count=values(init_count), max_hp=values(max_hp), "
        "max_mp=values(max_mp), psy_attack=values(psy_attack), "
        "psy_defense=values(psy_defense), mag_attack=values(mag_attack), "
        "mag_defence=values(mag_defence), move_speed=values(move_speed), "
        "attack_speed=values(attack_speed), hp_regen=values(hp_regen), "
        "mp_regen=values(mp_regen), all_element_resist=values(all_element_resist), "
        "fire_element_resist=values(fire_element_resist), "
        "water_element_resist=values(water_element_resist), "
        "light_element_resist=values(light_element_resist), "
        "dark_element_resist=values(dark_element_resist), "
        "all_element_attack=values(all_element_attack), "
        "fire_element_attack=values(fire_element_attack), "
        "water_element_attack=values(water_element_attack), "
        "light_element_attack=values(light_element_attack), "
        "dark_element_attack=values(dark_element_attack), "
        "psy_critical=values(psy_critical), mag_critical=values(mag_critical), "
        "good_hit=values(good_hit), evasion=values(evasion), "
        "hit_recovery=values(hit_recovery), "
        "separate_psy_mag_attack=values(separate_psy_mag_attack), "
        "quest_piece=values(quest_piece)",
        DB_QUEST_SHOP_U32(payload, 0),
        DB_QUEST_SHOP_U32(payload, 4),
        DB_QUEST_SHOP_U32(payload, 8),
        buying_status[0],
        buying_status[1],
        buying_status[2],
        buying_status[3],
        buying_status[4],
        buying_status[5],
        buying_status[6],
        buying_status[7],
        buying_status[8],
        buying_status[9],
        buying_status[10],
        buying_status[11],
        buying_status[12],
        buying_status[13],
        buying_status[14],
        buying_status[15],
        buying_status[16],
        buying_status[17],
        buying_status[18],
        buying_status[19],
        buying_status[20],
        buying_status[21],
        buying_status[22],
        buying_status[23],
        buying_status[24],
        buying_status[25],
        DB_QUEST_SHOP_U32(payload, 166))) {
        return false;
    }
    return db_handle->exec(true) == true;
}

#undef DB_QUEST_SHOP_U32
