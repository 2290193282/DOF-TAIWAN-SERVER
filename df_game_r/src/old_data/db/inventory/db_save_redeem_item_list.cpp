#include "old_data/db/inventory/db_save_redeem_item_list.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "old_data/db/character/load_etc_payload.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "old_data/db/core/db_mgr.h"
#pragma GCC diagnostic pop
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream.h"

#include <cstring>


namespace {

const unsigned int kRedeemInfoCompressedBytes = 1735u;

}  // namespace

// IDA_EA: 0x844F3B2
// IDA_NAME: _ZN21DB_SaveRedeemItemListC2Ev
// DEMANGLED: DB_SaveRedeemItemList::DB_SaveRedeemItemList(void)
DB_SaveRedeemItemList::DB_SaveRedeemItemList() : DBDispatcher() {}

// IDA_EA: 0x845FA4E
// IDA_NAME: _ZN21DB_SaveRedeemItemListD2Ev
// DEMANGLED: DB_SaveRedeemItemList::~DB_SaveRedeemItemList()
// IDA_EA: 0x845FA7E
// IDA_NAME: _ZN21DB_SaveRedeemItemListD0Ev
// DEMANGLED: DB_SaveRedeemItemList::~DB_SaveRedeemItemList()
DB_SaveRedeemItemList::~DB_SaveRedeemItemList() {}

// IDA_EA: 0x840E074
// IDA_NAME: _ZN21DB_SaveRedeemItemList8dispatchEiiP6Stream
// DEMANGLED: DB_SaveRedeemItemList::dispatch(int,int,Stream *)
bool DB_SaveRedeemItemList::dispatch(int signal,
                                     int user_idx,
                                     Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_REDEEMINFO_LIST_TO_DB* const payload =
        stream->GetOutBuffer<SIG_REDEEMINFO_LIST_TO_DB>();

    char compressed[kRedeemInfoCompressedBytes];
    std::memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_length = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_length,
                     reinterpret_cast<const char*>(payload),
                     sizeof(*payload)) != 1) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(3),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* redeem_blob = db_handle->blob_to_str(
        0, compressed, static_cast<int>(compressed_length));
    db_handle->set_query(
        "upDate charac_inven_expand set redeem_info='%s' where charac_no=%u",
        redeem_blob,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount() != 0) {
        return true;
    }

    redeem_blob = db_handle->blob_to_str(
        0, compressed, static_cast<int>(compressed_length));
    db_handle->set_query(
        "inSert into charac_inven_expand(charac_no, redeem_info) values(%u, '%s')",
        payload->charac_no,
        redeem_blob);
    return db_handle->exec(true) == true;
}
