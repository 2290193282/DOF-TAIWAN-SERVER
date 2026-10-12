#include "old_data/PVF/stackable/arad_gift_item_set.h"

#include <new>
#include <cstring>
#include <stdint.h>
#include <utility>

#include "common/config.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/stackable/stackable_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/arad_mail_runtime.h"
#include "old_game/player/core/cuser.h"

Arad_GiftItem_Set::Data::Data() : item_index(0), title(), content() {}

Arad_GiftItem_Set::Arad_GiftItem_Set() : Arad_ScriptLoader(), data_(), kinds_() {
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM", ST_EVENT_GIFT_ITEM));
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM2", ST_EVENT_GIFT_ITEM2));
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM3", ST_EVENT_GIFT_ITEM3));
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM4", ST_EVENT_GIFT_ITEM4));
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM5", ST_EVENT_GIFT_ITEM5));
    kinds_.insert(std::make_pair("ST_EVENT_GIFT_ITEM6", ST_EVENT_GIFT_ITEM6));
}

Arad_GiftItem_Set::~Arad_GiftItem_Set() {}

int Arad_GiftItem_Set::str_to_enum(const char* tag) const {
    const std::map<std::string, int>::const_iterator it = kinds_.find(tag);
    return it == kinds_.end() ? 0 : it->second;
}

int Arad_GiftItem_Set::open(STStackableScript* script) {
    if (script->arad.value_00 == 0) return 0;

    const int kind = str_to_enum(script->arad.value_08.c_str());
    if (kind == 0) return 0;

    for (int charac_job = 0;
         charac_job != oldsrv::config::kCharacterJobCount;
         ++charac_job) {
        if (script->item.usable_job_flags[charac_job] == 0) continue;

        const long long key =
            (static_cast<long long>(kind) << 32) |
            (static_cast<long long>(charac_job) << 16) |
            static_cast<uint32_t>(script->item.values_5c[0]);
        Data value;
        value.item_index = script->item.index;
        value.title = script->arad.value_0C;
        value.content = script->arad.value_10;
        if (!data_.insert(std::make_pair(key, value)).second) return 290;
    }
    return 0;
}

const Arad_GiftItem_Set::Data* Arad_GiftItem_Set::find(
    int kind,
    int charac_job,
    int usable_level) const {
    const long long key =
        (static_cast<long long>(kind) << 32) |
        (static_cast<long long>(static_cast<uint16_t>(charac_job)) << 16) |
        static_cast<uint32_t>(usable_level);
    const std::map<long long, Data>::const_iterator it = data_.find(key);
    return it == data_.end() ? 0 : &it->second;
}

// IDA_EA: 0x81A329E
// IDA_NAME: _ZN17Arad_GiftItem_Set9send_mailEP5CUseriiii
// DEMANGLED: Arad_GiftItem_Set::send_mail(CUser *,int,int,int,int)
int Arad_GiftItem_Set::send_mail(CUser* user,
                                 int receiver_charac_no,
                                 int usable_level,
                                 int charac_job,
                                 int item_kind) {
    const Data* const reward = find(item_kind, charac_job, usable_level);
    if (!reward) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Script_Loader.cpp",
            "bool Arad_GiftItem_Set::send_mail(CUser*, int, int, int, int)",
            0x15d,
            "[ERROR] send mail - Data None ");
        return 0;
    }

    ARAD_DATAS::AradMailInfo mail;
    mail.server_group = static_cast<uint8_t>(user->GetServerGroup());
    mail.receiver_charac_no = receiver_charac_no;
    mail.letter_text_length = static_cast<int>(reward->content.length());
    mail.content_address = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
        reward->content.c_str()));
    mail.title_address = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(
        reward->title.c_str()));

    mail.item_info.item_count = 1;
    mail.item_info.item_category = 2;
    mail.item_info.item_index = reward->item_index;

    CItem* const item = G_CDataManager()->find_item(reward->item_index);
    if (!item) {
        LogManager::logFormat(
            1,
            "localjapan/Arad_Script_Loader.cpp",
            "bool Arad_GiftItem_Set::send_mail(CUser*, int, int, int, int)",
            0x175,
            "[ARAD_RYOSIKA] Item Null. (index:%d)",
            reward->item_index);
        return 0;
    }

    Inven_Item generated;
    item->vtable_->make_item(item, generated);
    InvenItemMailRecord generated_mail_record;
    generated.ExportMailRecord(generated_mail_record);
    mail.item_info.endurance = generated_mail_record.endurance;
    return ARAD::send_mail(mail);
}

// IDA_EA: 0x81A31C4
// IDA_NAME: _ZN17Arad_GiftItem_Set11level_checkEP5CUserPK14CStackableItem
// DEMANGLED: Arad_GiftItem_Set::level_check(CUser *,CStackableItem const*)
int Arad_GiftItem_Set::level_check(CUser* user,
                                   const CStackableItem* item) {
    const int item_kind = str_to_enum(item->arad_item_info().string_08.c_str());
    const int usable_level = item->getUsableLevel();
    const int charac_job = user->get_charac_job();
    if (!find(item_kind, charac_job, usable_level)) {
        return 1;
    }
    const int charac_level = user->get_charac_level();
    if (charac_level >= item->getUsableLevel()) {
        return 1;
    }
    LogManager::logFormat(
        1,
        "localjapan/Arad_Script_Loader.cpp",
        "bool Arad_GiftItem_Set::level_check(CUser*, const CStackableItem*)",
        0x14B,
        "[ERROR] user level limit charac_no(%d)",
        charac_level);
    return 0;
}

template <>
Arad_GiftItem_Set* GlobalInstance<Arad_GiftItem_Set>::m_p = 0;

template <>
Arad_GiftItem_Set* GlobalInstance<Arad_GiftItem_Set>::create() {
    if (m_p == 0) m_p = new Arad_GiftItem_Set();
    return m_p;
}

template <>
Arad_GiftItem_Set* GlobalInstance<Arad_GiftItem_Set>::inst() {
    return create();
}
