#include "old_data/db/event/advance_altar_db.h"

#include "old_game/activity/reward/advance_altar.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace advancealtar {

// IDA_EA: 0x844DDEA
// IDA_NAME: _ZN12advancealtar7DB_LoadC2Ev
// DEMANGLED: advancealtar::DB_Load::DB_Load(void)
DB_Load::DB_Load() {}

// IDA_EA: 0x81420A0
// IDA_NAME: _ZN12advancealtar7DB_LoadD2Ev
// DEMANGLED: advancealtar::DB_Load::~DB_Load()
// IDA_EA: 0x81420D0
// IDA_NAME: _ZN12advancealtar7DB_LoadD0Ev
// DEMANGLED: advancealtar::DB_Load::~DB_Load() deleting destructor
DB_Load::~DB_Load() {}

// IDA_EA: 0x844DE06
// IDA_NAME: _ZN12advancealtar9DB_UpdateC2Ev
// DEMANGLED: advancealtar::DB_Update::DB_Update(void)
DB_Update::DB_Update() {}

// IDA_EA: 0x81420EE
// IDA_NAME: _ZN12advancealtar9DB_UpdateD2Ev
// DEMANGLED: advancealtar::DB_Update::~DB_Update()
// IDA_EA: 0x814211E
// IDA_NAME: _ZN12advancealtar9DB_UpdateD0Ev
// DEMANGLED: advancealtar::DB_Update::~DB_Update() deleting destructor
DB_Update::~DB_Update() {}

// IDA_EA: 0x813DDB2
// IDA_NAME: _ZN12advancealtar7DB_Load8dispatchEiiP6Stream
// DEMANGLED: advancealtar::DB_Load::dispatch(int,int,Stream *)
bool DB_Load::dispatch(int signal, int user_idx, Stream* stream) {
    if (!DBDispatcher::dispatch(signal, user_idx, stream)) {
        return false;
    }

    _CharacAdvanceAltarDbData* data =
        stream->GetOutBuffer<_CharacAdvanceAltarDbData>();
    if (!data) {
        return false;
    }

    const bool selected = Select(*data);
    return SendResult(signal, user_idx, *data, selected);
}

// IDA_EA: 0x813DE40
// IDA_NAME: _ZN12advancealtar7DB_Load6SelectERNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::DB_Load::Select(advancealtar::_CharacAdvanceAltarDbData &)
bool DB_Load::Select(_CharacAdvanceAltarDbData& data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    db_handle->set_query(
        "select ridable_id, ticket_free, ticket_cera, "
        "star_game, star_cera, star_usable, "
        "survival_best, star_reset_count, is_unlock_stage_effect, "
        "stage_list, slot_list, buy_item_list, reward_list "
        "from charac_advance_altar where charac_no = %u",
        data.charac_no);
    if (!db_handle->exec(true)) {
        return false;
    }

    const int row_count = db_handle->get_n_rows();
    for (int index = 0; index < row_count && index < 1; ++index) {
        if (!db_handle->fetch()) {
            return false;
        }

        _AdvanceAltarDbData& record = data.records[index];
        int ridable_id = 0;
        if (!db_handle->get_int(0, ridable_id)) {
            return false;
        }
        if (!isValidRidableId(static_cast<RidableId::T>(ridable_id))) {
            LogManager::logFormat(
                1,
                "localglobal/global_AdvanceAltar_Dispatcher.cpp",
                "bool advancealtar::DB_Load::Select(advancealtar::CharacAdvanceAltarDbData&)",
                177,
                "invalid RidableId=%d",
                ridable_id);
            return false;
        }
        record.ridable_id = ridable_id;

        if (!db_handle->get_short(1, record.free_ticket_count) ||
            !db_handle->get_short(2, record.charged_ticket_count) ||
            !db_handle->get_int(3, record.star_game) ||
            !db_handle->get_int(4, record.star_cera) ||
            !db_handle->get_int(5, record.star_usable)) {
            return false;
        }

        short survival_best = 0;
        short star_reset_count = 0;
        short unlock_stage_effect = 0;
        if (!db_handle->get_short(6, survival_best) ||
            !db_handle->get_short(7, star_reset_count) ||
            !db_handle->get_short(8, unlock_stage_effect)) {
            return false;
        }
        record.survival_best = static_cast<uint16_t>(survival_best);
        record.star_reset_count = static_cast<uint16_t>(star_reset_count);
        record.is_unlock_stage_effect = unlock_stage_effect > 0;

        std::memset(g_StrStageBuffer, 0, sizeof(g_StrStageBuffer));
        std::memset(g_StrSlotBuffer, 0, sizeof(g_StrSlotBuffer));
        std::memset(g_StrBuyItemBuffer, 0, sizeof(g_StrBuyItemBuffer));
        std::memset(g_StrRewardBuffer, 0, sizeof(g_StrRewardBuffer));
        if (!convertZipToString(db_handle, 9,
                                g_ZipStageBuffer,
                                sizeof(g_ZipStageBuffer),
                                g_StrStageBuffer,
                                sizeof(g_StrStageBuffer)) ||
            !convertZipToString(db_handle, 10,
                                g_ZipSlotBuffer,
                                sizeof(g_ZipSlotBuffer),
                                g_StrSlotBuffer,
                                sizeof(g_StrSlotBuffer)) ||
            !convertZipToString(db_handle, 11,
                                g_ZipBuyItemBuffer,
                                sizeof(g_ZipBuyItemBuffer),
                                g_StrBuyItemBuffer,
                                sizeof(g_StrBuyItemBuffer)) ||
            !convertZipToString(db_handle, 12,
                                g_ZipRewardBuffer,
                                sizeof(g_ZipRewardBuffer),
                                g_StrRewardBuffer,
                                sizeof(g_StrRewardBuffer))) {
            return false;
        }
        if (!parseStage(g_StrStageBuffer, record.stages, 50) ||
            !parseSlot(g_StrSlotBuffer, record.slots, 12) ||
            !parseBuyItem(g_StrBuyItemBuffer, record.buy_items, 50) ||
            !parseReward(g_StrRewardBuffer, record.rewards, 30)) {
            return false;
        }
    }
    return true;
}

// IDA_EA: 0x813E3E8
// IDA_NAME: _ZN12advancealtar7DB_Load10parseStageEPcPNS_6_StageEi
// DEMANGLED: advancealtar::DB_Load::parseStage(char *,advancealtar::_Stage *,int)
bool DB_Load::parseStage(char* text, _Stage* stages, int count) {
    if (!text || !stages) {
        return false;
    }

    char* entry_save = 0;
    char* entry = ::strtok_r(text, "|", &entry_save);
    for (int index = 0; entry && index < count; ++index) {
        char* field_save = 0;
        char* field = ::strtok_r(entry, ",", &field_save);
        for (int field_index = 0;
             field && field_index < 2;
             ++field_index) {
            if (field_index == 0) {
                stages[index].stage_id = std::atoi(field);
            } else {
                stages[index].star = std::atoi(field);
            }
            field = ::strtok_r(0, ",", &field_save);
        }
        entry = ::strtok_r(0, "|", &entry_save);
    }
    return true;
}

// IDA_EA: 0x813E542
// IDA_NAME: _ZN12advancealtar7DB_Load9parseSlotEPcPNS_5_SlotEi
// DEMANGLED: advancealtar::DB_Load::parseSlot(char *,advancealtar::_Slot *,int)
bool DB_Load::parseSlot(char* text, _Slot* slots, int count) {
    if (!text || !slots) {
        return false;
    }

    char* entry_save = 0;
    char* entry = ::strtok_r(text, "|", &entry_save);
    for (int index = 0; entry && index < count; ++index) {
        char* field_save = 0;
        char* field = ::strtok_r(entry, ",", &field_save);
        for (int field_index = 0;
             field && field_index < 4;
             ++field_index) {
            const int value = std::atoi(field);
            if (field_index == 0) {
                slots[index].locked = value > 0;
            } else if (field_index == 1) {
                slots[index].slot_type = static_cast<uint32_t>(value);
            } else if (field_index == 2) {
                slots[index].slot_no = static_cast<uint8_t>(value);
            } else {
                slots[index].item_id = static_cast<uint32_t>(value);
            }
            field = ::strtok_r(0, ",", &field_save);
        }
        entry = ::strtok_r(0, "|", &entry_save);
    }
    return true;
}

// IDA_EA: 0x813E708
// IDA_NAME: _ZN12advancealtar7DB_Load12parseBuyItemEPcPNS_8_BuyItemEi
// DEMANGLED: advancealtar::DB_Load::parseBuyItem(char *,advancealtar::_BuyItem *,int)
bool DB_Load::parseBuyItem(char* text,
                           _BuyItem* buy_items,
                           int count) {
    if (!text || !buy_items) {
        return false;
    }

    char* entry_save = 0;
    char* entry = ::strtok_r(text, "|", &entry_save);
    for (int index = 0; entry && index < count; ++index) {
        char* field_save = 0;
        char* field = ::strtok_r(entry, ",", &field_save);
        for (int field_index = 0;
             field && field_index < 4;
             ++field_index) {
            const int value = std::atoi(field);
            if (field_index == 0) {
                buy_items[index].shop_type = value;
            } else if (field_index == 1) {
                buy_items[index].item_id = value;
            } else if (field_index == 2) {
                buy_items[index].buy_count = static_cast<int16_t>(value);
            } else {
                buy_items[index].buy_limit = static_cast<int16_t>(value);
            }
            field = ::strtok_r(0, ",", &field_save);
        }
        entry = ::strtok_r(0, "|", &entry_save);
    }
    return true;
}

// IDA_EA: 0x813E8CC
// IDA_NAME: _ZN12advancealtar7DB_Load11parseRewardEPcPNS_7_RewardEi
// DEMANGLED: advancealtar::DB_Load::parseReward(char *,advancealtar::_Reward *,int)
bool DB_Load::parseReward(char* text, _Reward* rewards, int count) {
    if (!text || !rewards) {
        return false;
    }

    char* entry_save = 0;
    char* entry = ::strtok_r(text, "|", &entry_save);
    for (int index = 0; entry && index < count; ++index) {
        char* field_save = 0;
        char* field = ::strtok_r(entry, ",", &field_save);
        for (int field_index = 0;
             field && field_index < 2;
             ++field_index) {
            const int value = std::atoi(field);
            if (field_index == 0) {
                rewards[index].reward_id = value;
            } else {
                rewards[index].received = value ? 1 : 0;
            }
            field = ::strtok_r(0, ",", &field_save);
        }
        entry = ::strtok_r(0, "|", &entry_save);
    }
    return true;
}

// IDA_EA: 0x813EA46
// IDA_NAME: _ZN12advancealtar7DB_Load10SendResultEiiRNS_25_CharacAdvanceAltarDbDataEb
// DEMANGLED: advancealtar::DB_Load::SendResult(int,int,advancealtar::_CharacAdvanceAltarDbData &,bool)
bool DB_Load::SendResult(int signal,
                         int user_idx,
                         _CharacAdvanceAltarDbData& data,
                         bool select_succeeded) {
    if (select_succeeded) {
        Stream* stream = GlobalData::stream_pool()->Acquire(
            "localglobal/global_AdvanceAltar_Dispatcher.cpp", 447);
        CStreamGuard guard(stream, true);
        // Official 0627 DB_Load::SendResult @0x813EAA7/0x813EAC1 calls
        // CStreamGuard::operator<<(int) for both outer dispatch fields.
        // Narrowing either value to char shortens the queue header by three
        // bytes and makes InterDispatcher consume payload bytes as the code.
        *guard << signal;
        *guard << user_idx;
        guard->put_binary(&data, sizeof(data));
        GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    }
    return true;
}

// IDA_EA: 0x813EB38
// IDA_NAME: _ZN12advancealtar9DB_Update11makeRequestEiRKNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::DB_Update::makeRequest(int,advancealtar::_CharacAdvanceAltarDbData const&)
void DB_Update::makeRequest(int user_idx,
                            const _CharacAdvanceAltarDbData& data) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "localglobal/global_AdvanceAltar_Dispatcher.cpp", 473);
    CStreamGuard guard(stream, true);
    *guard << 26;
    *guard << user_idx;
    _CharacAdvanceAltarDbData* dest =
        guard.GetInBuffer<_CharacAdvanceAltarDbData>();
    std::memcpy(dest, &data, sizeof(data));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x813EC24
// IDA_NAME: _ZN12advancealtar9DB_Update8dispatchEiiP6Stream
// DEMANGLED: advancealtar::DB_Update::dispatch(int,int,Stream *)
bool DB_Update::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;
    _CharacAdvanceAltarDbData* data =
        stream->GetOutBuffer<_CharacAdvanceAltarDbData>();
    if (!data) {
        return false;
    }
    update(*data);
    return true;
}

// IDA_EA: 0x813EC5E
// IDA_NAME: _ZN12advancealtar9DB_Update6updateERKNS_25_CharacAdvanceAltarDbDataE
// DEMANGLED: advancealtar::DB_Update::update(advancealtar::_CharacAdvanceAltarDbData const&)
bool DB_Update::update(const _CharacAdvanceAltarDbData& data) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    bool result = true;
    for (int index = 0; index < 1; ++index) {
        const _AdvanceAltarDbData& record = data.records[index];
        if (record.updated != 1 ||
            !isValidRidableId(
                static_cast<RidableId::T>(record.ridable_id))) {
            continue;
        }

        std::string stage_text;
        std::string slot_text;
        std::string buy_item_text;
        std::string reward_text;
        makeStageString(stage_text, record.stages, 50);
        makeSlotString(slot_text, record.slots, 12);
        makeBuyItemString(buy_item_text, record.buy_items, 50);
        makeRewardString(reward_text, record.rewards, 30);

        unsigned int stage_zip_length = 0;
        unsigned int slot_zip_length = 0;
        unsigned int buy_item_zip_length = 0;
        unsigned int reward_zip_length = 0;
        std::memset(g_ZipStageBuffer, 0, sizeof(g_ZipStageBuffer));
        std::memset(g_ZipSlotBuffer, 0, sizeof(g_ZipSlotBuffer));
        std::memset(g_ZipBuyItemBuffer, 0, sizeof(g_ZipBuyItemBuffer));
        std::memset(g_ZipRewardBuffer, 0, sizeof(g_ZipRewardBuffer));
        if (!convertStringToZip(g_ZipStageBuffer,
                                sizeof(g_ZipStageBuffer),
                                stage_text.c_str(),
                                stage_text.size(),
                                stage_zip_length) ||
            !convertStringToZip(g_ZipSlotBuffer,
                                sizeof(g_ZipSlotBuffer),
                                slot_text.c_str(),
                                slot_text.size(),
                                slot_zip_length) ||
            !convertStringToZip(g_ZipBuyItemBuffer,
                                sizeof(g_ZipBuyItemBuffer),
                                buy_item_text.c_str(),
                                buy_item_text.size(),
                                buy_item_zip_length) ||
            !convertStringToZip(g_ZipRewardBuffer,
                                sizeof(g_ZipRewardBuffer),
                                reward_text.c_str(),
                                reward_text.size(),
                                reward_zip_length)) {
            return false;
        }

        const char* reward_blob = db_handle->blob_to_str(
            3, g_ZipRewardBuffer, reward_zip_length);
        const char* buy_item_blob = db_handle->blob_to_str(
            2, g_ZipBuyItemBuffer, buy_item_zip_length);
        const char* slot_blob = db_handle->blob_to_str(
            1, g_ZipSlotBuffer, slot_zip_length);
        const char* stage_blob = db_handle->blob_to_str(
            0, g_ZipStageBuffer, stage_zip_length);
        db_handle->set_query(
            "update charac_advance_altar set ticket_free = %d, "
            "ticket_cera = %d, star_game = %d, star_cera = %d, "
            "star_usable = %d, survival_best = %d, "
            "star_reset_count = %d, is_unlock_stage_effect = %d, "
            "stage_list = '%s', slot_list = '%s', "
            "buy_item_list = '%s', reward_list = '%s' "
            "where charac_no = %u and ridable_id = %d",
            record.free_ticket_count,
            record.charged_ticket_count,
            record.star_game,
            record.star_cera,
            record.star_usable,
            static_cast<int16_t>(record.survival_best),
            static_cast<int16_t>(record.star_reset_count),
            record.is_unlock_stage_effect,
            stage_blob,
            slot_blob,
            buy_item_blob,
            reward_blob,
            data.charac_no,
            record.ridable_id);
        result = db_handle->exec(true);
        if (!result || !db_handle->getAffectedRowCount()) {
            reward_blob = db_handle->blob_to_str(
                3, g_ZipRewardBuffer, reward_zip_length);
            buy_item_blob = db_handle->blob_to_str(
                2, g_ZipBuyItemBuffer, buy_item_zip_length);
            slot_blob = db_handle->blob_to_str(
                1, g_ZipSlotBuffer, slot_zip_length);
            stage_blob = db_handle->blob_to_str(
                0, g_ZipStageBuffer, stage_zip_length);
            db_handle->set_query(
                "insert into charac_advance_altar "
                "(charac_no, ridable_id, ticket_free, ticket_cera, "
                "star_game, star_cera, star_usable, survival_best, "
                "star_reset_count, is_unlock_stage_effect, stage_list, "
                "slot_list, buy_item_list, reward_list) values "
                "(%u, %d, %d, %d, %d, %d, %d, %d, %d, %d, "
                "'%s', '%s', '%s', '%s')",
                data.charac_no,
                record.ridable_id,
                record.free_ticket_count,
                record.charged_ticket_count,
                record.star_game,
                record.star_cera,
                record.star_usable,
                static_cast<int16_t>(record.survival_best),
                static_cast<int16_t>(record.star_reset_count),
                record.is_unlock_stage_effect,
                stage_blob,
                slot_blob,
                buy_item_blob,
                reward_blob);
            result = db_handle->exec(true);
        }
    }
    return result;
}

// IDA_EA: 0x813F380
// IDA_NAME: _ZN12advancealtar9DB_Update15makeStageStringERSsPKNS_6_StageEi
// DEMANGLED: advancealtar::DB_Update::makeStageString(std::string &,advancealtar::_Stage const*,int)
bool DB_Update::makeStageString(std::string& text,
                                const _Stage* stages,
                                int count) {
    if (!stages) {
        return false;
    }

    text.clear();
    char buffer[128];
    for (int index = 0; index < count; ++index) {
        if (stages[index].stage_id || stages[index].star) {
            std::sprintf(buffer,
                         "%d%s%d%s",
                         stages[index].stage_id,
                         ",",
                         stages[index].star,
                         "|");
            text += buffer;
        }
    }
    return false;
}

// IDA_EA: 0x813F442
// IDA_NAME: _ZN12advancealtar9DB_Update14makeSlotStringERSsPKNS_5_SlotEi
// DEMANGLED: advancealtar::DB_Update::makeSlotString(std::string &,advancealtar::_Slot const*,int)
bool DB_Update::makeSlotString(std::string& text,
                               const _Slot* slots,
                               int count) {
    if (!slots) {
        return false;
    }

    text.clear();
    char buffer[128];
    for (int index = 0; index < count; ++index) {
        std::sprintf(buffer,
                     "%d%s%d%s%d%s%d%s",
                     slots[index].locked,
                     ",",
                     slots[index].slot_type,
                     ",",
                     slots[index].slot_no,
                     ",",
                     slots[index].item_id,
                     "|");
        text += buffer;
    }
    return false;
}

// IDA_EA: 0x813F53C
// IDA_NAME: _ZN12advancealtar9DB_Update17makeBuyItemStringERSsPKNS_8_BuyItemEi
// DEMANGLED: advancealtar::DB_Update::makeBuyItemString(std::string &,advancealtar::_BuyItem const*,int)
bool DB_Update::makeBuyItemString(std::string& text,
                                  const _BuyItem* buy_items,
                                  int count) {
    if (!buy_items) {
        return false;
    }

    text.clear();
    char buffer[128];
    for (int index = 0; index < count; ++index) {
        if (buy_items[index].item_id || buy_items[index].buy_count) {
            std::sprintf(buffer,
                         "%d%s%d%s%d%s%d%s",
                         buy_items[index].shop_type,
                         ",",
                         buy_items[index].item_id,
                         ",",
                         buy_items[index].buy_count,
                         ",",
                         buy_items[index].buy_limit,
                         "|");
            text += buffer;
        }
    }
    return false;
}

// IDA_EA: 0x813F66A
// IDA_NAME: _ZN12advancealtar9DB_Update16makeRewardStringERSsPKNS_7_RewardEi
// DEMANGLED: advancealtar::DB_Update::makeRewardString(std::string &,advancealtar::_Reward const*,int)
bool DB_Update::makeRewardString(std::string& text,
                                 const _Reward* rewards,
                                 int count) {
    if (!rewards) {
        return false;
    }

    text.clear();
    char buffer[128];
    for (int index = 0; index < count; ++index) {
        if (rewards[index].reward_id > 0) {
            std::sprintf(buffer,
                         "%d%s%d%s",
                         rewards[index].reward_id,
                         ",",
                         rewards[index].received,
                         "|");
            text += buffer;
        }
    }
    return false;
}

}  // namespace advancealtar
