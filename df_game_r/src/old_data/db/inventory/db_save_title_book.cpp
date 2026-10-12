#include "old_data/db/inventory/db_save_title_book.h"

#include "common/types/compressed_blob.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "old_data/db/inventory/title_book_payload.h"
#include "infra/network/packet/stream.h"

#include <cstring>

namespace {

const unsigned int kGeneralCompressedBytes = 5136u;
const unsigned int kSpecificCompressedBytes = 14652u;
const unsigned int kDespairCompressedBytes = 8796u;
const unsigned int kEventCompressedBytes = 4404u;

}  // namespace

// IDA_EA: 0x844F82E
// IDA_NAME: _ZN16DB_SaveTitleBookC2Ev
// DEMANGLED: DB_SaveTitleBook::DB_SaveTitleBook(void)
DB_SaveTitleBook::DB_SaveTitleBook() : DBDispatcher() {}

// IDA_EA: 0x845FAEA
// IDA_NAME: _ZN16DB_SaveTitleBookD2Ev
// DEMANGLED: DB_SaveTitleBook::~DB_SaveTitleBook()
// IDA_EA: 0x845FB1A
// IDA_NAME: _ZN16DB_SaveTitleBookD0Ev
// DEMANGLED: DB_SaveTitleBook::~DB_SaveTitleBook()
DB_SaveTitleBook::~DB_SaveTitleBook() {}

// IDA_EA: 0x840DB00
// IDA_NAME: _ZN16DB_SaveTitleBook8dispatchEiiP6Stream
// DEMANGLED: DB_SaveTitleBook::dispatch(int,int,Stream *)
bool DB_SaveTitleBook::dispatch(int signal,
                                int user_idx,
                                Stream* stream) {
    (void)signal;
    (void)user_idx;

    stTitleBook_Save* const payload =
        stream->GetOutBuffer<stTitleBook_Save>();

    char general_blob[kGeneralCompressedBytes];
    std::memset(general_blob, 0, sizeof(general_blob));
    unsigned int general_len = sizeof(general_blob);
    if (compress_zip(general_blob,
                     general_len,
                     reinterpret_cast<const char*>(payload->general_section),
                     sizeof(payload->general_section)) != 1) {
        return false;
    }

    char specific_blob[kSpecificCompressedBytes];
    std::memset(specific_blob, 0, sizeof(specific_blob));
    unsigned int specific_len = sizeof(specific_blob);
    if (compress_zip(specific_blob,
                     specific_len,
                     reinterpret_cast<const char*>(payload->specific_section),
                     sizeof(payload->specific_section)) != 1) {
        return false;
    }

    char despair_blob[kDespairCompressedBytes];
    std::memset(despair_blob, 0, sizeof(despair_blob));
    unsigned int despair_len = sizeof(despair_blob);
    if (compress_zip(despair_blob,
                     despair_len,
                     reinterpret_cast<const char*>(payload->despair),
                     sizeof(payload->despair)) != 1) {
        return false;
    }

    char event_blob[kEventCompressedBytes];
    std::memset(event_blob, 0, sizeof(event_blob));
    unsigned int event_len = sizeof(event_blob);
    if (compress_zip(event_blob,
                     event_len,
                     reinterpret_cast<const char*>(payload->event),
                     sizeof(payload->event)) != 1) {
        return false;
    }

    MySQL* const db_handle = GlobalData::s_db_mgr->GetDBHandle(
        static_cast<ENUM_DB_HANDLE_IDX>(2),
        static_cast<ENUM_SERVER_GROUP>(0));
    char* event = db_handle->blob_to_str(3, event_blob, event_len);
    char* despair = db_handle->blob_to_str(2, despair_blob, despair_len);
    char* specific = db_handle->blob_to_str(1, specific_blob, specific_len);
    char* general = db_handle->blob_to_str(0, general_blob, general_len);
    db_handle->set_query(
        "upDate charac_titlebook set general_section='%s', specific_section='%s', despair='%s', event='%s' where charac_no=%u",
        general,
        specific,
        despair,
        event,
        payload->charac_no);
    if (db_handle->exec(true) != true) {
        return false;
    }
    if (db_handle->getAffectedRowCount()) {
        return true;
    }

    event = db_handle->blob_to_str(3, event_blob, event_len);
    despair = db_handle->blob_to_str(2, despair_blob, despair_len);
    specific = db_handle->blob_to_str(1, specific_blob, specific_len);
    general = db_handle->blob_to_str(0, general_blob, general_len);
    db_handle->set_query(
        "inSert into charac_titlebook(charac_no, general_section, specific_section, despair, event) values(%u, '%s','%s','%s','%s')",
        payload->charac_no,
        general,
        specific,
        despair,
        event);
    return db_handle->exec(true) == true;
}
