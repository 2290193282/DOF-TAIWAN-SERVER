#include "old_data/db/character/db_load_charac_hotkey_option.h"

#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <string.h>

extern int uncompress_zip(char* dest, unsigned int& dest_len,
                          const char* source, unsigned int source_len);



namespace {

enum {
    kInterLoadCharacHotkeyOptionSignal = 800,
    kDBLoadCharacHotkeyOptionSignal = 800,
    kHotkeyOptionBytes = CHotkeyOption::kCapacity,
    kLegacyHotkeyOptionBytes = 188,
    kPreviousHotkeyOptionBytes = 612,
    // The old server's conservative compression contract is raw + raw/5 +
    // 12.  Applying it to 1280 bytes gives exactly 1548 bytes.
    kCompressedHotkeyOptionBytes = 1548,
    kLineLoadCharacHotkeyOptionOut = 29806,
    kLineLoadCharacHotkeyOptionFetch = 29823,
    kLineLoadCharacHotkeyOptionBinary = 29833,
    kLineLoadCharacHotkeyOptionRequest = 29850,
};

}  // namespace

// IDA_EA: 0x844FEDA
// IDA_NAME: _ZN25DB_LoadCharacHotKeyOptionC2Ev
// DEMANGLED: DB_LoadCharacHotKeyOption::DB_LoadCharacHotKeyOption(void)
DB_LoadCharacHotKeyOption::DB_LoadCharacHotKeyOption() {}

// IDA_EA: 0x845DDAA
// IDA_NAME: _ZN25DB_LoadCharacHotKeyOptionD2Ev
// DEMANGLED: DB_LoadCharacHotKeyOption::~DB_LoadCharacHotKeyOption()
// IDA_EA: 0x845DDDA
// IDA_NAME: _ZN25DB_LoadCharacHotKeyOptionD0Ev
// DEMANGLED: DB_LoadCharacHotKeyOption::~DB_LoadCharacHotKeyOption() deleting destructor
// NOTE: The D0 wrapper calls this D2 body and then operator delete.
DB_LoadCharacHotKeyOption::~DB_LoadCharacHotKeyOption() {}

// IDA_EA: 0x842F3A0
// IDA_NAME: _ZN25DB_LoadCharacHotKeyOption8dispatchEiiP6Stream
// DEMANGLED: DB_LoadCharacHotKeyOption::dispatch(int,int,Stream *)
bool DB_LoadCharacHotKeyOption::dispatch(int signal,
                                         int user_idx,
                                         Stream* stream) {
    (void)signal;

    SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* request =
        stream->GetOutBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>();
    if (!request || request->key_type > 1u) {
        return false;
    }

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(11),static_cast<ENUM_SERVER_GROUP>( 0));
    const char* account_id = NumberToString(request->m_id, 0);
    db_handle->set_query(
        "seLect key_option from member_key_option where m_id = %s and key_type = %d",
        account_id,
        request->key_type);
    if (db_handle->exec(true) != true) {
        return false;
    }

    Stream* out_stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        kLineLoadCharacHotkeyOptionOut);
    CStreamGuard guard(out_stream, true);
    *guard << kInterLoadCharacHotkeyOptionSignal;
    *guard << user_idx;
    SIG_LOAD_CHARAC_HOTKEY_OPTION* payload =
        guard->GetInBuffer<SIG_LOAD_CHARAC_HOTKEY_OPTION>();
    if (!payload) {
        return false;
    }
    memset(payload, 0, sizeof(SIG_LOAD_CHARAC_HOTKEY_OPTION));
    payload->m_id = request->m_id;
    payload->key_type = static_cast<unsigned char>(request->key_type);

    if (db_handle->get_n_rows()) {
        if (db_handle->fetch() != true) {
            account_id = NumberToString(request->m_id, 0);
            cMyTrace trace(
                "virtual bool DB_LoadCharacHotKeyOption::dispatch(int, int, Stream*)",
                kLineLoadCharacHotkeyOptionFetch,
                5);
            trace("DB_LoadCharacHotKeyOption::dispatch, fetch ERROR m_id=%s",
                  account_id);
            return false;
        }

        const unsigned int compressed_len =
            static_cast<unsigned int>(db_handle->get_binary_length(0));
        if (compressed_len < sizeof(uint32_t) ||
            compressed_len > kCompressedHotkeyOptionBytes) {
            // Do not pass a truncated or oversized blob to get_binary():
            // MySQL::get_binary can truncate while still reporting success.
            return false;
        } else {
            char compressed[kCompressedHotkeyOptionBytes];
            memset(compressed, 0, sizeof(compressed));
            if (db_handle->get_binary(0, compressed, compressed_len) != true) {
                account_id = NumberToString(request->m_id, 0);
                cMyTrace trace(
                    "virtual bool DB_LoadCharacHotKeyOption::dispatch(int, int, Stream*)",
                    kLineLoadCharacHotkeyOptionBinary,
                    5);
                trace("DB_LoadCharacHotKeyOption, get_binary(0) ERROR m_id=%s",
                      account_id);
                return false;
            }

            uint32_t raw_header = 0;
            memcpy(&raw_header, compressed, sizeof(raw_header));
            const unsigned int stored_raw_len =
                raw_header & 0x3fffffffu;
            if (stored_raw_len == kLegacyHotkeyOptionBytes) {
                // An official pre-140828 row cannot represent the new text
                // contract.  Treat it as a one-time empty reset, without
                // exposing the legacy bytes to the 140828 client.
                account_id = NumberToString(request->m_id, 0);
                cMyTrace trace(
                    "virtual bool DB_LoadCharacHotKeyOption::dispatch(int, int, Stream*)",
                    kLineLoadCharacHotkeyOptionBinary,
                    5);
                trace("DB_LoadCharacHotKeyOption, legacy 188-byte row reset m_id=%s",
                      account_id);
                payload->hotkey_option.clear();
            } else {
                unsigned int accepted_raw_len = 0;
                if (stored_raw_len == kPreviousHotkeyOptionBytes) {
                    // Rows written by the previous 612-byte restoration are
                    // losslessly promoted into the new zero-filled object.
                    accepted_raw_len = kPreviousHotkeyOptionBytes;
                } else if (stored_raw_len == kHotkeyOptionBytes) {
                    accepted_raw_len = kHotkeyOptionBytes;
                } else {
                    // Validate the embedded raw-size header before
                    // uncompress_zip can use it as a destination length.
                    return false;
                }

                char raw[kHotkeyOptionBytes];
                memset(raw, 0, sizeof(raw));
                unsigned int raw_len = accepted_raw_len;
                if (uncompress_zip(raw,
                                   raw_len,
                                   compressed,
                                   compressed_len) == 1 &&
                    raw_len == accepted_raw_len) {
                    payload->hotkey_option.clear();
                    memcpy(payload->hotkey_option.bytes_, raw, raw_len);
                    const unsigned int text_length =
                        payload->hotkey_option.getTextLength();
                    if ((text_length == 0 &&
                         payload->hotkey_option.isEmpty()) ||
                        payload->hotkey_option.isValidText(text_length)) {
                        payload->option_length = text_length;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }
    }

    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
    return true;
}

// IDA_EA: 0x842F710
// IDA_NAME: _ZN25DB_LoadCharacHotKeyOption11makeRequestEiji
// DEMANGLED: DB_LoadCharacHotKeyOption::makeRequest(int,unsigned int,int)
void DB_LoadCharacHotKeyOption::makeRequest(int user_idx,
                                            unsigned int account_id,
                                            int key_type) {
    Stream* stream = GlobalData::stream_pool()->Acquire(
        "DBThread.cpp",
        kLineLoadCharacHotkeyOptionRequest);
    CStreamGuard guard(stream, true);
    *guard << kDBLoadCharacHotkeyOptionSignal;
    *guard << user_idx;
    SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION* request =
        guard->GetInBuffer<SIG_REQ_LOAD_CHARAC_HOTKEY_OPTION>();
    if (request) {
        request->m_id = account_id;
        request->key_type = static_cast<unsigned int>(key_type);
    }
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
}
