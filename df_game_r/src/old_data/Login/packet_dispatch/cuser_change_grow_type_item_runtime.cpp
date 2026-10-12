#include "old_game/player/core/cuser.h"

#include "old_game/progression/skill/create_character_skill_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/progression/quest/force_change_grow_type_quest_runtime.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/progression/skill/skill_changer_runtime.h"

// IDA_EA: 0x867B196
// IDA_NAME: _ZN5CUser19ChangeGrowType_ItemEis
// DEMANGLED: CUser::ChangeGrowType_Item(int,short)
int CUser::ChangeGrowType_Item(int first_grow_type, short item_slot) {
    if (!getCurCharacR()) {
        return 1;
    }
    if (!getCurCharFirstGrowType() ||
        getCurCharFirstGrowType() == first_grow_type) {
        return 19;
    }

    setCurCharChangingGrowthType(static_cast<char>(first_grow_type), -1);
    getCurCharacInvenW()->delete_item(static_cast<INVEN_TYPE>(1),
                                      item_slot,
                                      1,
                                      static_cast<eItemDelReason>(10),
                                      true);

    SkillSlot* const skill_slot = getCurCharacSkillW();
    skill_slot->clear_all_skills_both();
    skill_slot->set_parent(this);

    WongWork::CSkillChanger skill_changer;
    skill_changer.SkillInitialize(this, 0, true);

    const int character_job = get_charac_job();
    // get_charac_level() is a dead-result call in the old body but remains in
    // the original transaction between skill initialization and tree seeding.
    (void)get_charac_level();
    addSkillOnCreateCharacter(
        skill_slot->get_mastered_skill_tree(ENUM_SKILL_TREE_0), character_job);
    addSkillOnCreateCharacter(
        skill_slot->get_mastered_skill_tree(ENUM_SKILL_TREE_1), character_job);

    const int previous_first_grow_type = getCurCharFirstGrowType();
    const unsigned char current_second_grow_type =
        static_cast<unsigned char>(getCurCharSecondGrowType());
    set_grow_type(static_cast<unsigned char>(first_grow_type),
                  current_second_grow_type,
                  0,
                  static_cast<eChangeGrowTypeReason>(1));
    adjust_charac_stat();
    reset_tag_info_to_me(get_charac_no(-1));
    _postCheckForceChangeGrowType(this,
                                  first_grow_type,
                                  getCurCharSecondGrowType());

    SendNotiPacket(static_cast<eSendTarget>(0),
                   static_cast<ENUM_NOTIPACKET>(2),
                   0);
    send_skill_info();
    SendNotiPacket(static_cast<eSendTarget>(1),
                   static_cast<ENUM_NOTIPACKET>(2),
                   1);
    SendUpdateItem(static_cast<eSendTarget>(1),
                   static_cast<ENUM_ITEMSPACE>(0),
                   item_slot);

    const unsigned int character_no = get_charac_no(-1);
    cMyTrace trace("int CUser::ChangeGrowType_Item(int, short int)", 30061, 0);
    trace("[USE GROWTYPE ITEM]characNo(%d) before(%d), after(%d)",
          character_no,
          previous_first_grow_type,
          first_grow_type);
    return 0;
}
