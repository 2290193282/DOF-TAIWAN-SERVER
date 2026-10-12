#include "old_data/db/inventory/db_load_inventory.h"

#include "common/config.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/character_db_payloads.h"
#include "old_game/player/core/character_tag_data.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/creature/db_swap_creature_item.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "common/manager/db_mgr.h"
#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/equipment/item_option.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <stdint.h>
#include <cstring>
#include <string.h>

#include <vector>
#include <zlib.h>

namespace {

const unsigned int kCompressedBlobRawSizeMask = 0x3FFFFFFFu;

// `MySQL::get_binary` deliberately truncates to its caller-provided size while
// still reporting success.  Every database blob in this dispatcher must first
// be read at its declared length, then be constrained by the named destination
// type before zlib sees it.  The historical helper trusts the raw-size header,
// so it cannot safely perform this check itself.
bool ReadInventoryCompressedBlob(MySQL* db_handle,
                                 int column,
                                 unsigned int raw_capacity,
                                 std::vector<char>* encoded,
                                 unsigned int* raw_size) {
    if (!db_handle || !encoded || !raw_size) {
        return false;
    }

    const int encoded_length = db_handle->get_binary_length(column);
    if (encoded_length < 0) {
        return false;
    }
    if (encoded_length == 0) {
        encoded->clear();
        *raw_size = 0;
        return true;
    }

    // The repository's compressor stores a four-byte raw-length prefix and
    // uses zlib's `compress2`.  Reject inputs that cannot be a valid encoding
    // of the named destination before allocating/reading them.
    const uLong encoded_capacity = compressBound(raw_capacity);
    if (encoded_length < 4 ||
        static_cast<uLong>(encoded_length) > encoded_capacity + 4u) {
        return false;
    }

    encoded->resize(static_cast<size_t>(encoded_length));
    if (!db_handle->get_binary(column, &(*encoded)[0], encoded_length)) {
        encoded->clear();
        return false;
    }

    uint32_t declared_raw_size = 0;
    std::memcpy(&declared_raw_size, &(*encoded)[0], sizeof(declared_raw_size));
    declared_raw_size &= kCompressedBlobRawSizeMask;
    if (declared_raw_size > raw_capacity) {
        encoded->clear();
        return false;
    }
    *raw_size = declared_raw_size;
    return true;
}

// Call only after ReadInventoryCompressedBlob has checked the four-byte raw
// length against the concrete destination.  Do not route this through
// uncompress_zip: that historical helper advances source by four bytes but
// still passes the header-inclusive source length to zlib.  Exact vector
// storage has no trailing slack to absorb that four-byte overread.
bool DecodeCheckedInventoryBlob(const std::vector<char>& encoded,
                                unsigned int raw_size,
                                char* destination) {
    if (raw_size == 0) {
        return encoded.empty();
    }
    if (encoded.size() < 4 || !destination) {
        return false;
    }

    uLongf decoded_size = static_cast<uLongf>(raw_size);
    return ::uncompress(
               reinterpret_cast<Bytef*>(destination),
               &decoded_size,
               reinterpret_cast<const Bytef*>(encoded.data() + 4),
               static_cast<uLong>(encoded.size() - 4)) == Z_OK &&
           decoded_size == static_cast<uLongf>(raw_size);
}

bool IsItemRecordByteCount(unsigned int byte_count,
                           unsigned int maximum_byte_count) {
    return byte_count <= maximum_byte_count &&
           (byte_count == 0 || byte_count % sizeof(Inven_Item) == 0);
}

}  // namespace

// IDA_EA: 0x840A604
// IDA_NAME: _ZN16DB_LoadInventoryC2Ev
// DEMANGLED: DB_LoadInventory::DB_LoadInventory(void)
DB_LoadInventory::DB_LoadInventory() {}

// IDA_EA: 0x840A620
// IDA_NAME: _ZN16DB_LoadInventoryD2Ev
// DEMANGLED: DB_LoadInventory::~DB_LoadInventory()
// IDA_EA: 0x840A650
// IDA_NAME: _ZN16DB_LoadInventoryD0Ev
// DEMANGLED: DB_LoadInventory::~DB_LoadInventory() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadInventory::~DB_LoadInventory() {}

// IDA_EA: 0x840A66E
// IDA_NAME: _ZN16DB_LoadInventory8dispatchEiiP6Stream
// DEMANGLED: DB_LoadInventory::dispatch(int,int,Stream *)
bool DB_LoadInventory::dispatch(int signal, int user_idx, Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }
    SIG_LOAD_INVENTORY* payload =
        stream->GetOutBuffer<SIG_LOAD_INVENTORY>();
    if (!payload) {
        return false;
    }
    int growth_server_group = -1;
    if ((*stream >> growth_server_group) != 1 || stream->length() != 0 ||
        growth_server_group < 0 || growth_server_group >= 0x12) {
        return false;
    }

    if (GetUserInven(payload) != true) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9060,
                       5);
        trace("DB_LoadInventory::dispatch, GetUserInven ERROR charac_no=%u",
              payload->charac_no);
        throw 1;
    }
    if (GetUserAvatar(payload) != true) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9065,
                       5);
        trace("DB_LoadInventory::dispatch, GetUserAvatar ERROR charac_no=%u",
              payload->charac_no);
        throw 2;
    }

    for (int index = 0; index <= 139; ++index) {
        payload->creature_inventory[index].reset();
    }
    payload->creature_inventory[
        oldsrv::config::kCreatureInventoryEquippedCreatureSlot].reset();

    if (payload->creature_count && GetUserCreature(payload) != true) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9081,
                       5);
        trace("DB_LoadInventory::dispatch, GetUserCreature ERROR charac_no=%u",
              payload->charac_no);
        throw 3;
    }

    const int expand_result = _getCharacInvenExpand(payload);
    if (expand_result) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9089,
                       5);
        trace("DB_LoadInventory::dispatch, _getCharacInvenExpand ERROR charac_no=%u, line(%d)",
              payload->charac_no,
              expand_result);
        throw 4;
    }

    const int lock_result =
        GetItemLock(payload->charac_no, payload->item_locks);
    if (lock_result) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9109,
                       5);
        trace("DB_LoadInventory::dispatch, GetItemLock ERROR charac_no=%u, line(%d)",
              payload->charac_no,
              lock_result);
        throw 6;
    }
    if (_GetEquipMaxItemLevel(payload) != true) {
        cMyTrace trace("virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
                       9124,
                       5);
        trace("DB_LoadInventory::dispatch, _GetEquipMaxItemLevel ERROR charac_no=%u",
              payload->charac_no);
        throw 7;
    }

    // Do not touch SQL006 on the legacy DB thread.  The pointer-free deep
    // copy is a strict fan-in: signal 8 is forwarded to DISPATCH_Q only after
    // the dedicated native worker has installed every GUID state.
    if (!item_growth_db::QueueInventoryPreload(
            user_idx, growth_server_group, *payload)) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_LoadInventory::dispatch(int, int, Stream*)",
            9125,
            "equipment growth continuation queue failed charac_no=%u",
            payload->charac_no);
        return false;
    }
    return true;
}

// IDA_EA: 0x840AAC8
// IDA_NAME: _ZN16DB_LoadInventory21_GetEquipMaxItemLevelEP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::_GetEquipMaxItemLevel(SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::_GetEquipMaxItemLevel(SIG_LOAD_INVENTORY* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(2),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect max_equip_level from charac_manage_info where charac_no = %d",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (!db_handle->getAffectedRowCount()) {
        return true;
    }
    if (db_handle->fetch() != true) {
        return false;
    }
    return db_handle->get_int(0, payload->max_equip_level) == true;
}

// IDA_EA: 0x840ABA2
// IDA_NAME: _ZN16DB_LoadInventory18_GetTagCharacInvenEjP21TAG_CHARAC_INVEN_DATA
// DEMANGLED: DB_LoadInventory::_GetTagCharacInven(unsigned int,TAG_CHARAC_INVEN_DATA *)
bool DB_LoadInventory::_GetTagCharacInven(unsigned int charac_no,
                                          TAG_CHARAC_INVEN_DATA* tag_payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query("seLect equipslot from inventory where charac_no = %d",
                         charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return false;
    }

    // The `equipslot` blob is exactly twelve ordinary-equipment records.  It
    // fills TAG_CHARAC_INVEN_DATA::avatar_inventory[11..22]; equipped avatar
    // records, including weapon avatar slot 10, are loaded by _GetUserAvatar.
    // subsequent _GetUserAvatar call.
    const unsigned int persisted_equip_bytes = static_cast<unsigned int>(
        sizeof(tag_payload->avatar_inventory) -
        oldsrv::config::kAvatarEquipmentTypeCount *
            sizeof(tag_payload->avatar_inventory[0]));
    std::vector<char> equipment_zip;
    unsigned int equipment_raw_size = 0;
    if (!ReadInventoryCompressedBlob(db_handle,
                                     0,
                                     persisted_equip_bytes,
                                     &equipment_zip,
                                     &equipment_raw_size) ||
        !IsItemRecordByteCount(
            equipment_raw_size, persisted_equip_bytes) ||
        !DecodeCheckedInventoryBlob(
            equipment_zip,
            equipment_raw_size,
            reinterpret_cast<char*>(
                tag_payload->avatar_inventory +
                oldsrv::config::kAvatarEquipmentTypeCount))) {
        return false;
    }
    return true;
}

// IDA_EA: 0x840AD2A
// IDA_NAME: _ZNK16DB_LoadInventory21_getCharacInvenExpandEP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::_getCharacInvenExpand(SIG_LOAD_INVENTORY *)const
int DB_LoadInventory::_getCharacInvenExpand(
    SIG_LOAD_INVENTORY* payload) const {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect cargo_capacity,cargo,jewel,expand_equipslot from charac_inven_expand where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true || db_handle->fetch() != true) {
        return 9275;
    }
    if (db_handle->get_int(0, payload->cargo_capacity) != true) {
        return 9279;
    }

    const unsigned int cargo_capacity =
        static_cast<unsigned int>(sizeof(payload->cargo));
    std::vector<char> cargo_zip;
    unsigned int cargo_raw_len = 0;
    if (!ReadInventoryCompressedBlob(db_handle,
                                     1,
                                     cargo_capacity,
                                     &cargo_zip,
                                     &cargo_raw_len)) {
        return 9283;
    }
    if (!IsItemRecordByteCount(cargo_raw_len, cargo_capacity)) {
        return 9304;
    }
    if (!DecodeCheckedInventoryBlob(cargo_zip,
                                    cargo_raw_len,
                                    reinterpret_cast<char*>(payload->cargo))) {
        return 9303;
    }

    const unsigned int jewel_capacity = static_cast<unsigned int>(
        sizeof(payload->inventory) -
        oldsrv::config::kMainInventoryPrimaryItemCount *
            sizeof(payload->inventory[0]));
    std::vector<char> jewel_zip;
    unsigned int jewel_raw_len = 0;
    if (!ReadInventoryCompressedBlob(db_handle,
                                     2,
                                     jewel_capacity,
                                     &jewel_zip,
                                     &jewel_raw_len)) {
        return 9288;
    }
    if (!IsItemRecordByteCount(jewel_raw_len, jewel_capacity)) {
        return 9309;
    }
    if (!DecodeCheckedInventoryBlob(jewel_zip,
                                    jewel_raw_len,
                                    reinterpret_cast<char*>(
                                        payload->inventory +
                                        oldsrv::config::
                                            kMainInventoryPrimaryItemCount))) {
        return 9308;
    }

    std::vector<char> expand_equip_zip;
    unsigned int expand_equip_raw_len = 0;
    if (!ReadInventoryCompressedBlob(db_handle,
                                     3,
                                     sizeof(payload->expanded_equipment),
                                     &expand_equip_zip,
                                     &expand_equip_raw_len)) {
        return 9299;
    }
    if (expand_equip_raw_len != 0 &&
        (expand_equip_raw_len < sizeof(payload->expanded_equipment.charac_no) ||
         (expand_equip_raw_len -
              sizeof(payload->expanded_equipment.charac_no)) %
                 sizeof(Inven_Item) != 0)) {
        return 9315;
    }
    if (!DecodeCheckedInventoryBlob(
            expand_equip_zip,
            expand_equip_raw_len,
            reinterpret_cast<char*>(&payload->expanded_equipment))) {
        return 9314;
    }
    return 0;
}

// IDA_EA: 0x840B0D8
// IDA_NAME: _ZN16DB_LoadInventory12GetUserInvenEP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY* payload) {
    // The database keeps the official split: the primary inventory blob owns
    // slots 0..288 and `charac_inven_expand.jewel` owns slots 289..351.
    const unsigned int primary_inventory_capacity =
        oldsrv::config::kMainInventoryPrimaryItemCount *
        static_cast<unsigned int>(sizeof(Inven_Item));
    const unsigned int equipment_capacity =
        static_cast<unsigned int>(sizeof(payload->equipment));
    const unsigned int creature_tail_capacity = static_cast<unsigned int>(
        sizeof(payload->creature_inventory) -
        140u * sizeof(payload->creature_inventory[0]));
    std::vector<char> inven_zip;
    std::vector<char> equip_zip;
    std::vector<char> creature_zip;
    unsigned int inven_raw_len = 0;
    unsigned int equip_raw_len = 0;
    unsigned int creature_raw_len = 0;
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect money,coin,inventory,equipslot,pay_coin,event_coin,creature,creature_flag,inventory_capacity from inventory where charac_no=%u",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9419,
                       5);
        trace("DB_LoadInventory::GetUserInven, exec ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9426,
                       5);
        trace("DB_LoadInventory::GetUserInven, fetch ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->get_uint(0, payload->money) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9433,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(0, pContext->money) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->get_uint(1, payload->coin) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9440,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(1, pContext->coin) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    if (!ReadInventoryCompressedBlob(db_handle,
                                     2,
                                     primary_inventory_capacity,
                                     &inven_zip,
                                     &inven_raw_len)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9448,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_binary(2) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!IsItemRecordByteCount(inven_raw_len, primary_inventory_capacity)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9593,
                       5);
        trace("incorrect inventory size charac_no(%d), inven, original_size(%d), db_size(%d)\n",
              payload->charac_no,
              static_cast<int>(primary_inventory_capacity),
              static_cast<int>(inven_raw_len));
        return false;
    }

    if (!ReadInventoryCompressedBlob(db_handle,
                                     3,
                                     equipment_capacity,
                                     &equip_zip,
                                     &equip_raw_len)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9456,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_binary(3) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (!IsItemRecordByteCount(equip_raw_len, equipment_capacity)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9587,
                       5);
        trace("incorrect inventory size charac_no(%d), equip, original_size(%d), db_size(%d)\n",
              payload->charac_no,
              static_cast<int>(equipment_capacity),
              static_cast<int>(equip_raw_len));
        return false;
    }
    if (db_handle->get_uint(4, payload->pay_coin) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9463,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(4) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->get_uint(5, payload->event_coin) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9471,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(5) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    if (!ReadInventoryCompressedBlob(db_handle,
                                     6,
                                     creature_tail_capacity,
                                     &creature_zip,
                                     &creature_raw_len)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9499,
                       5);
        trace("DB_LoadCreature, get_binary(8) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->get_int(7, payload->creature_count) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9506,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(9) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    if (db_handle->get_uint(8, payload->inventory_capacity) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9530,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(13) ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }

    if (!IsItemRecordByteCount(creature_raw_len,
                               creature_tail_capacity)) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9608,
                       5);
        trace("incorrect inventory size charac_no(%d), creature, original_size(%d), db_size(%d)\n",
              payload->charac_no,
              static_cast<int>(creature_tail_capacity),
              static_cast<int>(creature_raw_len));
        return false;
    }

    if (!DecodeCheckedInventoryBlob(
            equip_zip,
            equip_raw_len,
            reinterpret_cast<char*>(payload->equipment))) {
        throw 9573u;
    }
    if (!DecodeCheckedInventoryBlob(
            inven_zip,
            inven_raw_len,
            reinterpret_cast<char*>(payload->inventory))) {
        throw 9575u;
    }
    if (!DecodeCheckedInventoryBlob(
            creature_zip,
            creature_raw_len,
            reinterpret_cast<char*>(payload->creature_inventory + 140))) {
        throw 9582u;
    }

    const char* m_id_text = NumberToString(payload->account_id, 0);
    db_handle->set_query(
        "seLect ifnull(min(avatar_coin),0) as avatar_coin from member_avatar_coin where m_id = %s",
        m_id_text);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9638,
                       5);
        trace("DB_LoadInventory::GetUserInven, avatar_coin exec ERROR m_id=%s",
              NumberToString(payload->account_id, 0));
        return false;
    }
    if (db_handle->fetch() != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9644,
                       5);
        trace("DB_LoadInventory::GetUserInven, avatar_coin fetch ERROR m_id=%s",
              NumberToString(payload->account_id, 0));
        return false;
    }
    if (db_handle->get_uint(0, payload->avatar_coin) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserInven(SIG_LOAD_INVENTORY*)",
                       9650,
                       5);
        trace("DB_LoadInventory::GetUserInven, get_uint(%u, pContext->avatar_coin) ERROR m_id=%s",
              0u,
              NumberToString(payload->account_id, 0));
        return false;
    }
    if (payload->tag_charac_no) {
        _GetTagCharacInven(payload->tag_charac_no,
                           &payload->tag_inventory);
    }
    return true;
}

// IDA_EA: 0x840BD3A
// IDA_NAME: _ZN16DB_LoadInventory13GetUserAvatarEP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::GetUserAvatar(SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::GetUserAvatar(SIG_LOAD_INVENTORY* payload) {
    if (_GetUserAvatar(payload->charac_no,
                       payload->avatar_detail,
                       payload->avatar_equipment,
                       payload->avatar_inventory,
                       payload->avatar_count) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserAvatar(SIG_LOAD_INVENTORY*)",
                       9675,
                       5);
        trace("DB_LoadInventory::_GetUserAvatar ERROR::charac_no(%d)",
              payload->charac_no);
        return false;
    }
    if (!payload->tag_charac_no) {
        return true;
    }

    TAG_CHARAC_INVEN_DATA tag_fields;
    memcpy(&tag_fields, &payload->tag_inventory, sizeof(tag_fields));
    const bool result = _GetUserAvatar(payload->tag_charac_no,
                                       tag_fields.avatar_detail,
                                       tag_fields.avatar_equipment,
                                       tag_fields.avatar_inventory,
                                       tag_fields.avatar_count);
    memcpy(&payload->tag_inventory, &tag_fields, sizeof(tag_fields));
    if (result != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserAvatar(SIG_LOAD_INVENTORY*)",
                       9687,
                       5);
        trace("DB_LoadInventory::_GetUserAvatar ERROR::tag_charac_no(%d)",
              payload->tag_charac_no);
        return false;
    }
    return true;
}

// IDA_EA: 0x840BE8A
// IDA_NAME: _ZN16DB_LoadInventory14_GetUserAvatarEjP16AvatarDetailInfoP10Inven_ItemS3_Ri
// DEMANGLED: DB_LoadInventory::_GetUserAvatar(unsigned int,AvatarDetailInfo *,Inven_Item *,Inven_Item *,int &)
bool DB_LoadInventory::_GetUserAvatar(unsigned int charac_no,
                                      AvatarDetailInfo* avatar_detail,
                                      Inven_Item* avatar_equip_slot,
                                      Inven_Item* avatar_inven,
                                      int& avatar_count) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect slot,it_id,unix_timestamp(expire_date),ability_no,ipg_agency_no,unix_timestamp(reg_date),ui_id,clear_avatar_id,jewel_socket,item_lock_key, hidden_option, emblem_endurance, color1, color2, trade_restrict from user_items where charac_no=%u and expire_date>now() and stat=0 limit %d",
        charac_no,
        oldsrv::config::kAvatarDetailItemCount);
    if (db_handle->exec(true) != true) {
        cMyTrace trace(
            "static bool DB_LoadInventory::_GetUserAvatar(unsigned int, AvatarDetailInfo*, Inven_Item*, Inven_Item*, int&)",
            9948,
            5);
        trace("DB_LoadInventory::GetUserAvatar, exec() ERROR charac_no=%u",
              charac_no);
        return false;
    }

    avatar_count = db_handle->get_n_rows();
    if (avatar_count < 0 ||
        avatar_count > oldsrv::config::kAvatarDetailItemCount) {
        return false;
    }
    for (int row = 0; row < avatar_count; ++row) {
        if (db_handle->fetch() != true) {
            cMyTrace trace(
                "static bool DB_LoadInventory::_GetUserAvatar(unsigned int, AvatarDetailInfo*, Inven_Item*, Inven_Item*, int&)",
                9971,
                5);
            trace("DB_LoadInventory::GetUserAvatar, fetch() ERROR charac_no=%u,%d",
                  charac_no,
                  row);
            // The old 0x840C05C path logs the short fetch and returns success.
            return true;
        }
        int slot = 0;
        int it_id = 0;
        unsigned int expire_date = 0;
        int ability_no = 0;
        char ipg_agency_no[25];
        memset(ipg_agency_no, 0, sizeof(ipg_agency_no));
        int reg_date = 0;
        int ui_id = 0;
        int clear_avatar_id = 0;
        unsigned char lock = 0;
        unsigned int hidden_option = 0;
        unsigned int emblem_endurance = 0;
        short color1 = 0;
        short color2 = 0;
        char trade_restrict = 0;
        stAvatarEmblemInfo_t jewel_socket;
        jewel_socket.init();

        int column = 0;
        if (db_handle->get_int(column++, slot) != true) {
            return false;
        }
        if (slot < 0 || slot >= oldsrv::config::kAvatarDetailItemCount) {
            return false;
        }
        if (db_handle->get_int(column++, it_id) != true) {
            return false;
        }
        if (db_handle->get_uint(column++, expire_date) != true) {
            return false;
        }
        if (db_handle->get_int(column++, ability_no) != true) {
            return false;
        }
        if (db_handle->get_str(
                column++, ipg_agency_no, sizeof(ipg_agency_no) - 1) != true) {
            return false;
        }
        if (db_handle->get_int(column++, reg_date) != true) {
            return false;
        }
        if (db_handle->get_int(column++, ui_id) != true) {
            return false;
        }
        if (db_handle->get_int(column++, clear_avatar_id) != true) {
            return false;
        }
        if (db_handle->get_binary(
                column++, &jewel_socket, sizeof(jewel_socket)) != true) {
            return false;
        }
        if (db_handle->get_ubyte(column++, lock) != true) {
            return false;
        }
        if (db_handle->get_uint(column++, hidden_option) != true) {
            return false;
        }
        if (db_handle->get_uint(column++, emblem_endurance) != true) {
            return false;
        }
        if (db_handle->get_short(column++, color1) != true) {
            cMyTrace trace(
                "static bool DB_LoadInventory::_GetUserAvatar(unsigned int, AvatarDetailInfo*, Inven_Item*, Inven_Item*, int&)",
                10090,
                5);
            trace("DB_LoadInventory::GetUserAvatar, get_short(10) ERROR charac_no=%u",
                  charac_no);
            // IDA continues with color1=0 on this one field failure.
        }
        if (db_handle->get_short(column++, color2) != true) {
            return false;
        }
        avatar_detail[row].color1 = color1;
        avatar_detail[row].color2 = color2;
        if (db_handle->get_byte(column++, trade_restrict) != true) {
            return false;
        }

        AvatarDetailInfo& detail = avatar_detail[row];
        detail.expire_date = expire_date;
        detail.ui_id = static_cast<unsigned int>(ui_id);
        detail.registration_date = static_cast<unsigned int>(reg_date);
        strncpy(detail.ipg_agency_no, ipg_agency_no, 0x18u);
        detail.clear_avatar_id = static_cast<unsigned int>(clear_avatar_id);
        detail.jewel_socket = jewel_socket;

        Inven_Item* item = 0;
        if (slot < oldsrv::config::kAvatarEquipmentTypeCount) {
            item = &avatar_inven[slot];
        } else {
            item = &avatar_equip_slot[
                slot - oldsrv::config::kAvatarEquipmentTypeCount];
        }
        InventoryItemDbFields fields;
        memcpy(&fields, item, sizeof(fields));
        fields.item_type = 8;
        fields.endurance_or_ability = static_cast<uint16_t>(ability_no);
        fields.item_id = static_cast<uint32_t>(it_id);
        fields.ui_id = static_cast<uint32_t>(ui_id);
        fields.amplify_option.SetLock(lock);
        fields.hidden_option = static_cast<uint16_t>(hidden_option);
        fields.emblem_endurance = static_cast<uint16_t>(emblem_endurance);
        if (trade_restrict) {
            fields.upgrade_separate_flags |= 0x20u;
        } else {
            fields.upgrade_separate_flags &=
                static_cast<unsigned char>(~0x20u);
        }
        memcpy(item, &fields, sizeof(fields));
    }
    return true;
}

// IDA_EA: 0x840CA74
// IDA_NAME: _ZN16DB_LoadInventory15GetUserCreatureEP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY* payload) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect slot,it_id,name,stomach,exp,endurance,creature_type,ui_id,no_charge,stat,item_lock_key,unix_timestamp(expire_date) from creature_items where charac_no=%u and stat = 0 and expire_date>now()",
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                       10224,
                       5);
        trace("DB_LoadInventory::GetUserCreature, exec() ERROR charac_no=%u",
              payload->charac_no);
        return false;
    }
    const int rows = db_handle->get_n_rows();
    payload->creature_count = rows;
    if (rows > 141) {
        cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                       10235,
                       5);
        trace("DB_LoadInventory::GetUserCreature, creature_item_count overflow ERROR charac_no=%u, item_count=%d",
              payload->charac_no,
              rows);
        return false;
    }

    unsigned char slot_used[141];
    memset(slot_used, 0, sizeof(slot_used));
    std::vector<InventoryItemDbFields> pending_items;
    for (int row = 0; row < rows; ++row) {
        if (db_handle->fetch() != true) {
            cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                           10254,
                           5);
            trace("DB_LoadInventory::GetUserCreature, fetch() ERROR charac_no=%u,%d",
                  payload->charac_no,
                  row);
            // IDA breaks the row loop and still completes duplicate repair.
            break;
        }
        int slot = 0;
        if (db_handle->get_int(0, slot) != true) {
            return false;
        }

        const int item_slot =
            (slot ==
             oldsrv::config::kCreatureInventoryEquippedCreatureSlot)
                ? 140
                : slot;
        if (item_slot > 140) {
            cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                           10269,
                           5);
            trace("DB_LoadInventory::GetUserCreature, charac_no=%u, slot=%d overflow",
                  payload->charac_no,
                  slot);
            continue;
        }
        if (item_slot < 0) {
            return false;
        }

        const bool duplicate_slot = slot_used[item_slot] == 1;
        if (!duplicate_slot) {
            slot_used[item_slot] = 1;
        }

        unsigned long item_id = 0;
        char name[16];
        int stomach = 0;
        int experience = 0;
        unsigned short endurance = 0;
        int creature_type = 0;
        int ui_id = 0;
        int no_charge = 0;
        int state = 0;
        unsigned char lock = 0;
        int expiration_time = 0;
        memset(name, 0, sizeof(name));

        if (db_handle->get_ulong(1, item_id) != true) {
            return false;
        }
        if (db_handle->get_str(2, name, 13) != true) {
            return false;
        }
        if (db_handle->get_int(3, stomach) != true) {
            return false;
        }
        if (db_handle->get_int(4, experience) != true) {
            return false;
        }
        if (db_handle->get_ushort(5, endurance) != true) {
            return false;
        }
        if (db_handle->get_int(6, creature_type) != true) {
            return false;
        }
        if (db_handle->get_int(7, ui_id) != true) {
            return false;
        }
        if (db_handle->get_int(8, no_charge) != true) {
            return false;
        }
        if (db_handle->get_int(9, state) != true) {
            return false;
        }
        if (db_handle->get_ubyte(10, lock) != true) {
            return false;
        }
        if (db_handle->get_int(11, expiration_time) != true) {
            return false;
        }
        if (slot > oldsrv::config::kCreatureInventoryLastSlot) {
            cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                           10413,
                           5);
            trace("DB_LoadInventory::GetUserCreature, slot_no overflow ERROR charac_no=%u, slot_no=%d",
                  payload->charac_no,
                  slot);
            return false;
        }

        if (duplicate_slot) {
            InventoryItemDbFields fields;
            memset(&fields, 0, sizeof(fields));
            fields.item_type = 5;
            fields.endurance_or_ability = endurance;
            fields.item_id = static_cast<uint32_t>(item_id);
            fields.ui_id = static_cast<uint32_t>(ui_id);
            fields.amplify_option.SetLock(lock);
            pending_items.push_back(fields);
        } else {
            Inven_Item& item = payload->creature_inventory[slot];
            InventoryItemDbFields fields;
            memcpy(&fields, &item, sizeof(fields));
            fields.item_type = 5;
            fields.endurance_or_ability = endurance;
            fields.item_id = static_cast<uint32_t>(item_id);
            fields.ui_id = static_cast<uint32_t>(ui_id);
            fields.amplify_option.SetLock(lock);
            memcpy(&item, &fields, sizeof(fields));
        }

        LoadInventoryCreatureDetail& detail = payload->creature_detail[row];
        memset(&detail, 0, sizeof(detail));
        detail.slot = slot;
        detail.ui_id = ui_id;
        memcpy(detail.name, name, sizeof(detail.name));
        detail.stomach = stomach;
        detail.experience = experience;
        detail.creature_type = static_cast<unsigned char>(creature_type);
        detail.no_charge = static_cast<unsigned char>(no_charge);
        detail.state = static_cast<unsigned char>(state);
        detail.expiration_value = expiration_time;
    }
    int new_slot = 0;
    for (std::vector<InventoryItemDbFields>::iterator it =
             pending_items.begin();
         it != pending_items.end();
         ++it) {
        while (new_slot <= 140 && slot_used[new_slot]) {
            ++new_slot;
        }
        if (new_slot == 140) {
            new_slot =
                oldsrv::config::kCreatureInventoryEquippedCreatureSlot;
        }
        if (new_slot !=
                oldsrv::config::kCreatureInventoryEquippedCreatureSlot &&
            new_slot > 139) {
            cMyTrace trace("bool DB_LoadInventory::GetUserCreature(SIG_LOAD_INVENTORY*)",
                           10473,
                           5);
            trace("DB_LoadInventory::GetUserCreature, charac_no=%u, slot overflow",
                  payload->charac_no);
            break;
        }
        slot_used[new_slot ==
                          oldsrv::config::kCreatureInventoryEquippedCreatureSlot
                      ? 140
                      : new_slot] = 1;
        Inven_Item& destination = payload->creature_inventory[new_slot];
        InventoryItemDbFields destination_fields;
        memcpy(&destination_fields,
               &destination,
               sizeof(destination_fields));
        destination_fields.item_type = 5;
        destination_fields.endurance_or_ability = it->endurance_or_ability;
        destination_fields.item_id = it->item_id;
        destination_fields.ui_id = it->ui_id;
        destination_fields.amplify_option.SetLock(
            static_cast<unsigned char>(it->amplify_option.GetLock()));
        memcpy(&destination,
               &destination_fields,
               sizeof(destination_fields));
        DB_SwapCreatureItem::makeRequest(destination_fields.ui_id, new_slot);
    }
    return true;
}

// IDA_EA: 0x840D766
// IDA_NAME: _ZN16DB_LoadInventory11GetItemLockEjP14stItemLockInfo
// DEMANGLED: DB_LoadInventory::GetItemLock(unsigned int,stItemLockInfo *)
int DB_LoadInventory::GetItemLock(unsigned int charac_no,
                                  stItemLockInfo* item_lock) {
    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(3),static_cast<ENUM_SERVER_GROUP>( 0));
    db_handle->set_query(
        "seLect item_lock_info  from charac_item_lock_info where charac_no=%u",
        charac_no);
    if (db_handle->exec(true) != true) {
        return 10522;
    }
    if (!db_handle->get_n_rows()) {
        return 0;
    }
    if (db_handle->fetch() != true) {
        return 10532;
    }

    const unsigned int item_lock_capacity =
        100u * static_cast<unsigned int>(sizeof(stItemLockInfo));
    std::vector<char> lock_zip;
    unsigned int lock_raw_len = 0;
    if (!ReadInventoryCompressedBlob(db_handle,
                                     0,
                                     item_lock_capacity,
                                     &lock_zip,
                                     &lock_raw_len)) {
        return 10539;
    }
    if (lock_raw_len % sizeof(stItemLockInfo) != 0) {
        return 10545;
    }
    if (!DecodeCheckedInventoryBlob(
            lock_zip,
            lock_raw_len,
            reinterpret_cast<char*>(item_lock))) {
        return 10543;
    }
    return 0;
}

// IDA_EA: 0x840D98C
// IDA_NAME: _ZN16DB_LoadInventory19SendResult_forchinaEiiP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::SendResult_forchina(int,int,SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::SendResult_forchina(int signal,
                                           int user_idx,
                                           SIG_LOAD_INVENTORY* payload) {
    Stream* stream = GlobalData::big_stream_pool()->Acquire("DBThread.cpp",
                                                            10553);
    CStreamGuard guard(stream, true);
    *guard << signal;
    *guard << user_idx;
    guard->put_binary(payload, sizeof(*payload));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x840DA6E
// IDA_NAME: _ZN16DB_LoadInventory10SendResultEiiP18SIG_LOAD_INVENTORY
// DEMANGLED: DB_LoadInventory::SendResult(int,int,SIG_LOAD_INVENTORY *)
bool DB_LoadInventory::SendResult(int signal,
                                  int user_idx,
                                  SIG_LOAD_INVENTORY* payload) {
    return SendResult_forchina(signal, user_idx, payload);
}
