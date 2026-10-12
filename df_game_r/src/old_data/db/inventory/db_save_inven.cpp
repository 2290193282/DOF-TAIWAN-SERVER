#include "old_data/db/inventory/db_save_inven.h"
#include "old_data/db/inventory/db_item_growth_state.h"

#include "common/config.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>
#include <string.h>

extern int compress_zip(char* dest, unsigned int& dest_len,
                        const char* source, unsigned int source_len);


// IDA_EA: 0x8416A42
// IDA_NAME: _ZN12DB_SaveInvenC2Ev
// DEMANGLED: DB_SaveInven::DB_SaveInven(void)
DB_SaveInven::DB_SaveInven() {}

// IDA_EA: 0x8416A5E
// IDA_NAME: _ZN12DB_SaveInvenD2Ev
// DEMANGLED: DB_SaveInven::~DB_SaveInven()
// IDA_EA: 0x8416A8E
// IDA_NAME: _ZN12DB_SaveInvenD0Ev
// DEMANGLED: DB_SaveInven::~DB_SaveInven() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveInven::~DB_SaveInven() {}

// IDA_EA: 0x8416AAC
// IDA_NAME: _ZN12DB_SaveInven8dispatchEiiP6Stream
// DEMANGLED: DB_SaveInven::dispatch(int,int,Stream *)
bool DB_SaveInven::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    unsigned char payload_bytes[sizeof(SIG_SAVE_INVENTORY)];
    if (stream->get_binary(payload_bytes, sizeof(payload_bytes)) != 1) {
        return false;
    }

    SIG_SAVE_INVENTORY* const request =
        reinterpret_cast<SIG_SAVE_INVENTORY*>(payload_bytes);
    item_growth_db::LegacySaveProofTrailer proof;
    memset(&proof, 0, sizeof(proof));
    bool has_proof = false;
    if (stream->length() != 0) {
        if (stream->length() != static_cast<int>(sizeof(proof)) ||
            stream->get_binary(&proof, sizeof(proof)) != 1 ||
            stream->length() != 0 ||
            proof.magic != item_growth_db::kLegacyProofMagic ||
            proof.version != item_growth_db::kMessageVersion ||
            proof.proof_kind != item_growth_db::kSagaProofInventory ||
            proof.reserved != 0u || proof.server_group < 0 ||
            proof.server_group >= 0x12 ||
            proof.account_id != request->account_id ||
            proof.charac_no != request->character_no) {
            return false;
        }
        has_proof = true;
    }
    if (SaveInven(request) != true) {
        return false;
    }
    const unsigned char mode = request->save_mode;
    if ((!mode || mode == 2) && _saveCharacInvenExpand(request) != true) {
        return false;
    }
    return !has_proof || item_growth_db::QueuePersistedProof(
                             user_idx,
                             proof.server_group,
                             proof.account_id,
                             proof.charac_no,
                             proof.proof_kind);
}

// IDA_EA: 0x8416B38
// IDA_NAME: _ZNK12DB_SaveInven22_saveCharacInvenExpandEP18SIG_SAVE_INVENTORY
// DEMANGLED: DB_SaveInven::_saveCharacInvenExpand(SIG_SAVE_INVENTORY *)const
bool DB_SaveInven::_saveCharacInvenExpand(SIG_SAVE_INVENTORY* payload_data) const {
    char cargo_zip[11138];
    memset(cargo_zip, 0, sizeof(cargo_zip));
    unsigned int cargo_zip_len = sizeof(cargo_zip);
    if (compress_zip(cargo_zip,
                     cargo_zip_len,
                     reinterpret_cast<const char*>(payload_data->cargo_items),
                     sizeof(payload_data->cargo_items)) != 1) {
        return false;
    }

    char jewel_zip[4623];
    memset(jewel_zip, 0, sizeof(jewel_zip));
    unsigned int jewel_zip_len = sizeof(jewel_zip);
    if (compress_zip(jewel_zip,
                     jewel_zip_len,
                     reinterpret_cast<const char*>(
                         payload_data->stackable_items +
                         oldsrv::config::kMainInventoryPrimaryItemCount),
                     sizeof(Inven_Item) *
                         oldsrv::config::kMainInventoryExpandedItemCount) != 1) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    char* cargo_blob = db_handle->blob_to_str(0, cargo_zip, cargo_zip_len);
    char* jewel_blob = db_handle->blob_to_str(1, jewel_zip, jewel_zip_len);
    db_handle->set_query(
        "upDate charac_inven_expand set cargo_capacity=%d,cargo='%s',jewel='%s' where charac_no=%u",
        payload_data->cargo_capacity,
        cargo_blob,
        jewel_blob,
        payload_data->character_no);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8416CFA
// IDA_NAME: _ZN12DB_SaveInven9SaveInvenEP18SIG_SAVE_INVENTORY
// DEMANGLED: DB_SaveInven::SaveInven(SIG_SAVE_INVENTORY *)
bool DB_SaveInven::SaveInven(SIG_SAVE_INVENTORY* payload_data) {
    char equip_zip[890];
    char inven_zip[18238];
    char creature_zip[7478];
    memset(equip_zip, 0, sizeof(equip_zip));
    memset(inven_zip, 0, sizeof(inven_zip));
    memset(creature_zip, 0, sizeof(creature_zip));

    unsigned int equip_zip_len = sizeof(equip_zip);
    unsigned int inven_zip_len = sizeof(inven_zip);
    unsigned int creature_zip_len = sizeof(creature_zip);
    const unsigned char mode = payload_data->save_mode;
    if (!mode || mode == 2) {
        if (compress_zip(equip_zip,
                         equip_zip_len,
                         reinterpret_cast<const char*>(
                             payload_data->equipment_items +
                             oldsrv::config::kOrdinaryEquipmentTypeFirst),
                         sizeof(Inven_Item) *
                             oldsrv::config::kOrdinaryEquipmentTypeCount) !=
            1) {
            return false;
        }
        if (compress_zip(inven_zip,
                         inven_zip_len,
                         reinterpret_cast<const char*>(
                             payload_data->stackable_items),
                         sizeof(Inven_Item) *
                             oldsrv::config::kMainInventoryPrimaryItemCount) !=
            1) {
            return false;
        }
        if (compress_zip(creature_zip,
                         creature_zip_len,
                         reinterpret_cast<const char*>(
                             payload_data->creature_region.creature_items +
                             140),
                         sizeof(Inven_Item) * 102u) != 1) {
            return false;
        }
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_3, SERVER_GROUP_DEFAULT);
    if (mode) {
        if (mode == 1) {
            db_handle->set_query(
                "upDate inventory set money=%u,coin=%d,pay_coin=%d,event_coin=%d  where charac_no=%u",
                payload_data->money,
                payload_data->coin,
                payload_data->pay_coin,
                payload_data->event_coin,
                payload_data->character_no);
        } else {
            char* equip_blob = db_handle->blob_to_str(
                0, equip_zip, equip_zip_len);
            char* inven_blob = db_handle->blob_to_str(
                1, inven_zip, inven_zip_len);
            char* creature_blob = db_handle->blob_to_str(
                2, creature_zip, creature_zip_len);
            db_handle->set_query(
                "upDate inventory set money=%u,equipslot='%s',inventory='%s',creature='%s' where charac_no=%u",
                payload_data->money,
                equip_blob,
                inven_blob,
                creature_blob,
                payload_data->character_no);
        }
    } else {
        char* equip_blob = db_handle->blob_to_str(0, equip_zip, equip_zip_len);
        char* inven_blob = db_handle->blob_to_str(1, inven_zip, inven_zip_len);
        char* creature_blob = db_handle->blob_to_str(
            2, creature_zip, creature_zip_len);
        db_handle->set_query(
            "upDate inventory set money=%u,coin=%d,pay_coin=%d,event_coin=%d,equipslot='%s',inventory='%s',creature='%s' where charac_no=%u",
            payload_data->money,
            payload_data->coin,
            payload_data->pay_coin,
            payload_data->event_coin,
            equip_blob,
            inven_blob,
            creature_blob,
            payload_data->character_no);
    }
    if (db_handle->exec(true) != true) {
        return false;
    }

    const unsigned int avatar_coin = payload_data->avatar_coin;
    const char* m_id_text = NumberToString(payload_data->account_id, 0);
    db_handle->set_query(
        "inSert into member_avatar_coin (m_id , avatar_coin ) values (%s , %u) on duplicate key upDate avatar_coin = %u ",
        m_id_text,
        avatar_coin,
        avatar_coin);
    return db_handle->exec(true) == true;
}

// IDA_EA: 0x8417204
// IDA_NAME: _ZN12DB_SaveInven10SendResultEiiP18SIG_SAVE_INVENTORY
// DEMANGLED: DB_SaveInven::SendResult(int,int,SIG_SAVE_INVENTORY *)
bool DB_SaveInven::SendResult(int signal,
                              int user_idx,
                              SIG_SAVE_INVENTORY* payload) {
    (void)signal;
    (void)user_idx;
    (void)payload;
    return true;
}
