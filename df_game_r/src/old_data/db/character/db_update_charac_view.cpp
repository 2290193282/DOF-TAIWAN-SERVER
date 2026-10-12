#include "old_data/db/character/db_update_charac_view.h"

#include "old_data/db/core/mysql.h"
#include "common/types/compressed_blob.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/network/packet/stream.h"
#include "old_game/player/core/character_view_data.h"

#include <stdint.h>
#include <string.h>


namespace {

enum {
    kCharacViewCompressedBufferSize = 6405,
};

}  // namespace

// IDA_EA: 0x844D85C
// IDA_NAME: _ZN19DB_UpdateCharacViewC2Ev
// DEMANGLED: DB_UpdateCharacView::DB_UpdateCharacView(void)
DB_UpdateCharacView::DB_UpdateCharacView() {}

// IDA_EA: 0x844D878
// IDA_NAME: _ZN19DB_UpdateCharacViewD2Ev
// DEMANGLED: DB_UpdateCharacView::~DB_UpdateCharacView()
// IDA_EA: 0x844D8A8
// IDA_NAME: _ZN19DB_UpdateCharacViewD0Ev
// DEMANGLED: DB_UpdateCharacView::~DB_UpdateCharacView() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_UpdateCharacView::~DB_UpdateCharacView() {}

// IDA_EA: 0x8408848
// IDA_NAME: _ZN19DB_UpdateCharacView8dispatchEiiP6Stream
// DEMANGLED: DB_UpdateCharacView::dispatch(int,int,Stream *)
bool DB_UpdateCharacView::dispatch(int signal, int user_idx, Stream* stream) {
    (void)signal;
    (void)user_idx;

    SIG_UPDATE_CHARAC_VIEW* const payload =
        stream->GetOutBuffer<SIG_UPDATE_CHARAC_VIEW>();

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2,
        static_cast<ENUM_SERVER_GROUP>(payload->server_group));

    char compressed[kCharacViewCompressedBufferSize];
    memset(compressed, 0, sizeof(compressed));
    unsigned int compressed_len = sizeof(compressed);
    if (compress_zip(compressed,
                     compressed_len,
                     reinterpret_cast<const char*>(payload->characters),
                     sizeof(payload->characters)) != 1) {
        return false;
    }

    const char* const member_id = NumberToString(
        static_cast<uint32_t>(payload->account_id), 0);
    char* const info_blob =
        db_handle->blob_to_str(0, compressed, compressed_len);
    if (!member_id || !info_blob) {
        return false;
    }

    // MySQL reports zero affected rows when an UPDATE writes bytes identical
    // to the existing blob.  The legacy two-step code treated that as a
    // missing row and attempted a duplicate INSERT.  One MySQL-5.5-compatible
    // upsert handles new, changed, and byte-identical native 0x9C records
    // atomically without relying on affected-row semantics.
    if (!db_handle->set_query(
            "inSert into charac_view(m_id,info) values(%s,'%s') "
            "on duplicate key upDate info=values(info)",
            member_id,
            info_blob)) {
        return false;
    }
    return db_handle->exec(true) == true;
}
