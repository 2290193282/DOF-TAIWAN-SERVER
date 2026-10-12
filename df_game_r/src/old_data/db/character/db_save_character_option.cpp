#include "old_data/db/character/db_save_character_option.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "infra/network/packet/stream.h"

#include <stdint.h>



namespace {

enum {
    kSigCharacterOptionSize = 516,
};

#define DB_CHARACTER_OPTION_U32(payload, offset) \
    (*reinterpret_cast<uint32_t*>((payload) + (offset)))

}  // namespace

// IDA_EA: 0x844EC42
// IDA_NAME: _ZN22DB_SaveCharacterOptionC2Ev
// DEMANGLED: DB_SaveCharacterOption::DB_SaveCharacterOption(void)
DB_SaveCharacterOption::DB_SaveCharacterOption() {}

// IDA_EA: 0x845DC72
// IDA_NAME: _ZN22DB_SaveCharacterOptionD2Ev
// DEMANGLED: DB_SaveCharacterOption::~DB_SaveCharacterOption()
// IDA_EA: 0x845DCA2
// IDA_NAME: _ZN22DB_SaveCharacterOptionD0Ev
// DEMANGLED: DB_SaveCharacterOption::~DB_SaveCharacterOption() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_SaveCharacterOption::~DB_SaveCharacterOption() {}

// IDA_EA: 0x8430144
// IDA_NAME: _ZN22DB_SaveCharacterOption8dispatchEiiP6Stream
// DEMANGLED: DB_SaveCharacterOption::dispatch(int,int,Stream *)
bool DB_SaveCharacterOption::dispatch(int signal,
                                      int user_idx,
                                      Stream* stream) {
    (void)signal;
    (void)user_idx;

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(
        DB_HANDLE_2, SERVER_GROUP_DEFAULT);
    if (!stream->enable_out(kSigCharacterOptionSize)) {
        return false;
    }
    unsigned char* payload =
        reinterpret_cast<unsigned char*>(stream->get_buffer());
    stream->out_ptr(kSigCharacterOptionSize);

    char* option_blob = db_handle->blob_to_str(0, payload + 4, 512);
    db_handle->set_query(
        "upDate charac_option set options = '%s' where charac_no= %u",
        option_blob,
        DB_CHARACTER_OPTION_U32(payload, 0));

    bool need_insert = true;
    if (db_handle->exec(true) == true) {
        if (db_handle->getAffectedRowCount()) {
            need_insert = false;
        }
    }
    if (need_insert) {
        db_handle->set_query(
            "inSert into charac_option(charac_no, options) values(%u,'%s')",
            DB_CHARACTER_OPTION_U32(payload, 0),
            option_blob);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }
    return true;
}

#undef DB_CHARACTER_OPTION_U32
