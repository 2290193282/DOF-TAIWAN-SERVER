#ifndef OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DATA_H_
#define OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DATA_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x81347E2 in the companion source.


#include <stddef.h>
#include <stdint.h>

#include <map>
#include <vector>

#include "old_game/activity/reward/advance_altar_shop_parameter.h"
#include "old_game/activity/reward/advance_altar.h"
#include "old_data/PVF/dungeon/advance_altar_dungeon_script.h"

class CUser;
class PacketGuard;

namespace CMDPacketStruct {
struct _SetSlotData;
namespace ResetStarType {
enum T {
    BY_GOLD = 0
};
}  // namespace ResetStarType
}

namespace advancealtar {

struct _RewardItem;

// IDA unit: 04_character_select_create/advancealtar/_SlotItem.
#pragma pack(push, 1)
struct _SlotItem {
    _SlotItem();

    uint8_t slot_no;          // +0x00
    uint8_t reserved_01[3];   // +0x01, untouched compiler gap
    int32_t item_id;          // +0x04
    int32_t required_gage;    // +0x08
};

static_assert(sizeof(_SlotItem) == 0x0cu &&
                  offsetof(_SlotItem, slot_no) == 0x00u &&
                  offsetof(_SlotItem, item_id) == 0x04u &&
                  offsetof(_SlotItem, required_gage) == 0x08u,
              "advance-altar slot-item ABI changed");

// IDA unit: 04_character_select_create/advancealtar/_Slot.
struct _Slot {
    _Slot();

    uint8_t locked;
    uint32_t slot_type;
    uint8_t slot_no;
    uint32_t item_id;
};

// IDA unit: 99_misc_global/advancealtar/_Stage.
struct _Stage {
    _Stage();

    int32_t stage_id;
    int32_t star;
};

// IDA unit: 99_misc_global/advancealtar/_BuyItem.
struct _BuyItem {
    _BuyItem();
    ~_BuyItem();

    int32_t shop_type;
    int32_t item_id;
    int16_t buy_count;
    int16_t buy_limit;
};

// IDA unit: 99_misc_global/advancealtar/_Reward.
struct _Reward {
    _Reward();

    int32_t reward_id;
    uint8_t received;
};
#pragma pack(pop)

// IDA unit: 99_misc_global/advancealtar/_AdvanceAltarDbData.
// 04 owns the enclosing character DB payload; AdvanceAltarData itself is a
// shared 99 boundary and consumes this 0x518 byte slice.
struct _AdvanceAltarDbData {
    int32_t ridable_id;
    int32_t free_ticket_count;
    int32_t charged_ticket_count;
    int32_t star_game;
    int32_t star_cera;
    int32_t star_usable;
    uint16_t survival_best;
    uint16_t star_reset_count;
    uint32_t unknown_1c;
    uint8_t is_unlock_stage_effect;
    _Stage stages[50];
    _Slot slots[12];
    _BuyItem buy_items[50];
    _Reward rewards[30];
    uint8_t updated;
};

// IDA unit:
// 04_character_select_create/advancealtar/_CharacAdvanceAltarDbData.
// The record array is held in an anonymous union so the old constructor can
// zero the 0x51c-byte wire payload without invoking the element constructors.
struct _CharacAdvanceAltarDbData {
    _CharacAdvanceAltarDbData();

    uint32_t charac_no;
    union {
        _AdvanceAltarDbData records[1];
    };
};

// Local layout for the old advancealtar::AdvanceAltarData payload embedded in
// CharacAdvanceAltarManager. Its own member functions are 99-owned; 04 calls
// those IDA boundaries but does not re-implement them here.
struct AdvanceAltarData {
    AdvanceAltarData();
    ~AdvanceAltarData();

    bool set(const _AdvanceAltarDbData& db_data);
    int reset();
    bool get(_AdvanceAltarDbData& db_data) const;
    std::map<int, _BuyItem>* getBuyItemMap(
        AdvanceAltarShopType::T shop_type);
    std::map<uint8_t, _Slot>* getSlotMap(AdvanceAltarSlotType::T slot_type);
    bool getBuyItem(const std::map<int, _BuyItem>& source,
                    _BuyItem* dest,
                    int count) const;
    bool getSlot(const std::map<uint8_t, _Slot>& source,
                 _Slot* dest,
                 int count) const;

    int ridable_id;
    int free_ticket_count;
    int charged_ticket_count;
    int star_game;
    int star_cera;
    int star_usable;
    uint16_t survival_best;
    uint16_t star_reset_count;
    uint32_t unknown_1c;
    uint8_t is_unlock_stage_effect;
    std::map<int, _Stage> stages_24;
    std::map<int, _BuyItem> unit_buy_items_3c;
    std::map<int, _BuyItem> skill_buy_items_54;
    std::map<int, _BuyItem> tower_buy_items_6c;
    std::map<uint8_t, _Slot> unit_slots_84;
    std::map<uint8_t, _Slot> skill_slots_9c;
    std::map<int, _Reward> rewards_b4;
    uint8_t updated;
};

// IDA_EA: 0x8141781
// IDA_NAME: _ZN12advancealtar22putRidableInfoToPacketER11PacketGuardRKNS_16AdvanceAltarDataE
// DEMANGLED: advancealtar::putRidableInfoToPacket(PacketGuard &,advancealtar::AdvanceAltarData const&)
bool putRidableInfoToPacket(PacketGuard& packet,
                            const AdvanceAltarData& altar_data);

// IDA unit:
// 04_character_select_create/advancealtar/CharacAdvanceAltarManager.
struct CharacAdvanceAltarManager {
    CharacAdvanceAltarManager();
    ~CharacAdvanceAltarManager();

    int setCurrentAdvanceAltarData(RidableId::T ridable_id);
    int getCurrentRidableId() const;
    bool isAvailableTciket() const;
    bool isEnterStage(int stage_id) const;
    // IDA_EA: 0x8132638
    // IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager11getSlotItemERSt3mapIhNS_9_SlotItemESt4lessIhESaISt4pairIKhS2_EEE
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::getSlotItem(std::map<unsigned char,advancealtar::_SlotItem> &)const
    char getSlotItem(std::map<uint8_t, _SlotItem>& slot_items) const;
    // IDA_EA: 0x81328EA
    // IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager12getTowerInfoEiRiS1_
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::getTowerInfo(int,int &,int &)const
    int getTowerInfo(int monster_id,
                     int& tower_index,
                     int& tower_level) const;
    int decTicket();
    // IDA_EA: 0x813159A
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager16refillTicketFreeEv
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::refillTicketFree(void)
    bool refillTicketFree();
    // IDA_EA: 0x813175A
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager10clearStageEiNS_15StageDifficulty1TENS_9StageType1TEiRiRbS6_
    int clearStage(int stage_index,
                   StageDifficulty::T difficulty,
                   StageType::T stage_type,
                   int clear_survival_round_no,
                   int& reward_star,
                   bool& unlocked_stage,
                   bool& improved_survival_record);
    // IDA_EA: 0x81318E2
    // IDA_NAME: _ZNK12advancealtar25CharacAdvanceAltarManager20getClearStageSummaryEPi
    char getClearStageSummary(int summary[3]) const;
    // IDA_EA: 0x8132ABE
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager13addRewardDataEiRSt3mapIiNS_7_RewardESt4lessIiESaISt4pairIKiS2_EEE
    int addRewardData(int achievement_id, std::map<int, _Reward>& rewards);
    bool set(const _CharacAdvanceAltarDbData& data);
    bool reset();
    int isUpdated() const;
    AdvanceAltarData* getAdvanceAltarDataByIndex(int index);
    bool getCharacAdvanceAltarDbData(_CharacAdvanceAltarDbData& data) const;
    int setSlot(bool set_flag, const CMDPacketStruct::_SetSlotData& data);
    int exchangeSlot(CMDPacketStruct::_SetSlotData& from,
                     CMDPacketStruct::_SetSlotData& to);
    int resetSlot(AdvanceAltarData& altar_data,
                  const CMDPacketStruct::_SetSlotData& data);
    int gmSetStar(int star);
    int gmSetTicket(int ticket_count);
    // IDA_EA: 0x81333B2
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager9resetStarEP5CUserN15CMDPacketStruct13ResetStarType1TE
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::resetStar(CUser *,CMDPacketStruct::ResetStarType::T)
    int resetStar(CUser* user, CMDPacketStruct::ResetStarType::T type);
    // IDA_EA: 0x8132D20 / 0x81331D4
    int setUnlockBuyItem(AdvanceAltarShopType::T shop_type,
                         int stage_index,
                         std::vector<_BuyItem>* unlocked_items);
    int setUnlockItemToEmptySlot(AdvanceAltarShopType::T shop_type,
                                 int item_id);
    int setNextStage(int stage_index);
    // IDA_EA: 0x8131426 / 0x8132CB8 / 0x8132F72 / 0x8132F8E
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager28checkAndCreateDefaultRidableEv /
    //           _ZN12advancealtar25CharacAdvanceAltarManager17setDefaultBuyItemENS_20AdvanceAltarShopType1TE /
    //           _ZN12advancealtar25CharacAdvanceAltarManager15setDefaultStageEv /
    //           _ZN12advancealtar25CharacAdvanceAltarManager14setDefaultSlotEv
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::checkAndCreateDefaultRidable(void) /
    //            advancealtar::CharacAdvanceAltarManager::setDefaultBuyItem(advancealtar::AdvanceAltarShopType::T) /
    //            advancealtar::CharacAdvanceAltarManager::setDefaultStage(void) /
    //            advancealtar::CharacAdvanceAltarManager::setDefaultSlot(void)
    char checkAndCreateDefaultRidable();
    int setDefaultBuyItem(AdvanceAltarShopType::T shop_type);
    int setDefaultStage();
    char setDefaultSlot();
    // IDA_EA: 0x81319AE
    // IDA_NAME: _ZN12advancealtar25CharacAdvanceAltarManager7buyItemENS_20AdvanceAltarShopType1TEissiRi
    // DEMANGLED: advancealtar::CharacAdvanceAltarManager::buyItem(advancealtar::AdvanceAltarShopType::T,int,short,short,int,int &)
    int buyItem(AdvanceAltarShopType::T shop_type,
                int upgrade_id,
                int16_t first_level,
                int16_t last_level,
                int price,
                int& remaining_star);
    AdvanceAltarData* getCurrentAdvanceAltarData() const;
    int sendAchievementReward(
        CUser* user,
        int achievement_id,
        const std::vector<_RewardItem>** reward_items);

    AdvanceAltarData* current;
    AdvanceAltarData data[1];
};

}  // namespace advancealtar

#endif  // OLDSRV_CHARACTER_SELECT_ADVANCE_ALTAR_DATA_H_
