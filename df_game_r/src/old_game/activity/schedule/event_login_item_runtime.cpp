#include "old_game/activity/schedule/event_login_item_runtime.h"

#include "common/manager/db_common_public_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace {

struct PendingEventItem {
    unsigned int entry_id;
    unsigned int charac_no;
    unsigned int item_no;
    unsigned int stack_count;
};

MySQL* GetPrimaryDBHandle() {
    return GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_1,
                                              SERVER_GROUP_DEFAULT);
}

MySQL* GetCharacterDBHandle() {
    return GlobalData::s_db_mgr->GetDBHandle(DB_HANDLE_3,
                                              SERVER_GROUP_DEFAULT);
}

}  // namespace

namespace WongWork {

// IDA_EA: 0x86C1ABA
// IDA_NAME: _ZN8WongWork9CWebEvent11GetItemTypeEii
// DEMANGLED: WongWork::CWebEvent::GetItemType(int,int)
int CWebEvent::GetItemType(int event_kind, int item_no) {
    if (event_kind == 2) {
        switch (item_no) {
        case 999:
            return 4;
        case 26013:
            return 3;
        case 40656:
        case 44648:
        case 48652:
        case 52609:
            return 1;
        case 41056:
        case 45048:
        case 49052:
        case 53009:
            return 2;
        default:
            return 0;
        }
    }
    if (event_kind == 3) {
        if (item_no == 8) {
            return 2;
        }
        return item_no == 26014;
    }
    return 0;
}

// IDA_EA: 0x83F9B54
// IDA_NAME: _ZN8WongWork11EventCommon26InsertEventItem2EventTableEjjjj
// DEMANGLED: WongWork::EventCommon::InsertEventItem2EventTable(unsigned int,unsigned int,unsigned int,unsigned int)
unsigned int EventCommon::InsertEventItem2EventTable(unsigned int charac_no,
                                                      unsigned int item_no,
                                                      unsigned int stack_count,
                                                      unsigned int event_code) {
    MySQL* const db = GetCharacterDBHandle();
    db->set_query(
        "inSert into charac_event_items(charac_no,it_id,stack_count,event_code,reg_time) values(%d,%d,%d,%d,now())",
        charac_no,
        item_no,
        stack_count,
        event_code);
    if (db->exec(true)) {
        cMyTrace trace(
            "unsigned int WongWork::EventCommon::InsertEventItem2EventTable(unsigned int, unsigned int, unsigned int, unsigned int)",
            40,
            0);
        trace(
            "SUCCESS : insert into charac_event_items(charac_no,it_id,stack_count,event_code,reg_time) values(%d,%d,%d,%d,now())",
            charac_no,
            item_no,
            stack_count,
            event_code);
        return DBCommon::GetIdentity(db);
    }

    cMyTrace trace(
        "unsigned int WongWork::EventCommon::InsertEventItem2EventTable(unsigned int, unsigned int, unsigned int, unsigned int)",
        37,
        0);
    trace(
        "FAIL : insert into charac_event_items(charac_no,it_id,stack_count,event_code,reg_time) values(%d,%d,%d,%d,now())",
        charac_no,
        item_no,
        stack_count,
        event_code);
    return 0;
}

// IDA_EA: 0x83F9C7B
// IDA_NAME: _ZN8WongWork11EventCommon19LoadCharacEventItemEjjj
// DEMANGLED: WongWork::EventCommon::LoadCharacEventItem(unsigned int,unsigned int,unsigned int)
void EventCommon::LoadCharacEventItem(unsigned int account_id,
                                      unsigned int charac_no,
                                      unsigned int event_item_id) {
    MySQL* const db = GetCharacterDBHandle();
    if (event_item_id != 0) {
        db->set_query(
            "seLect id,it_id,stack_count,event_code from charac_event_items where id=%s and charac_no=%d",
            NumberToString(event_item_id, 0),
            charac_no);
    } else {
        db->set_query(
            "seLect id,it_id,stack_count,event_code from charac_event_items where charac_no=%d and delete_flag=0 limit %d",
            charac_no,
            5);
    }

    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventCommon::LoadCharacEventItem(memberIdentificationNumber_t, unsigned int, memberIdentificationNumber_t)",
            55,
            5);
        trace("DB_LoadInventory::GetUserInven, exec ERROR charac_no=%u",
              charac_no);
        return;
    }

    const int row_count = db->get_n_rows();
    if (row_count == 0) {
        return;
    }

    Stream* const stream = GlobalData::stream_pool()->Acquire("DBEventWork.inl",
                                                                62);
    CStreamGuard guard(stream, true);
    guard << 75;
    guard << -1;
    SIG_LOAD_EVENTITEM* const response = guard.GetInBuffer<SIG_LOAD_EVENTITEM>();
    response->member_id = account_id;
    response->charac_no = charac_no;
    for (int index = 0; index < row_count && db->fetch(); ++index) {
        db->get_uint(0, response->items[index].id);
        db->get_uint(1, response->items[index].item_id);
        db->get_uint(2, response->items[index].stack_count);
        db->get_uint(3, response->items[index].event_code);
    }
    response->item_count = static_cast<unsigned int>(row_count);
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x83F9F2E
// IDA_NAME: _ZN8WongWork8EventATI22CheckAlreadyGetATIItemEji
// DEMANGLED: WongWork::EventATI::CheckAlreadyGetATIItem(unsigned int,int)
bool EventATI::CheckAlreadyGetATIItem(unsigned int entry_id, int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    switch (CWebEvent::GetItemType(2, item_no)) {
    case 1:
        db->set_query("seLect avata_check1 from event_ati_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    case 2:
        db->set_query("seLect avata_check2 from event_ati_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    case 3:
        db->set_query("seLect item_check from event_ati_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    case 4:
        db->set_query("seLect cera_check from event_ati_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    }
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FA0AD
// IDA_NAME: _ZN8WongWork8EventATI18UpdateEventATIInfoEji
// DEMANGLED: WongWork::EventATI::UpdateEventATIInfo(unsigned int,int)
int EventATI::UpdateEventATIInfo(unsigned int entry_id, int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    switch (CWebEvent::GetItemType(2, item_no)) {
    case 1:
        db->set_query(
            "upDate event_ati_info set avata_check1=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    case 2:
        db->set_query(
            "upDate event_ati_info set avata_check2=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    case 3:
        db->set_query(
            "upDate event_ati_info set item_check=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    case 4:
        db->set_query(
            "upDate event_ati_info set cera_check=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    }
    return db->exec(true);
}

// IDA_EA: 0x83FA1D3
// IDA_NAME: _ZN8WongWork8EventATI19ArrangeATIEventItemEjj
// DEMANGLED: WongWork::EventATI::ArrangeATIEventItem(unsigned int,unsigned int)
unsigned char EventATI::ArrangeATIEventItem(unsigned int account_id,
                                             unsigned int event_id) {
    PendingEventItem pending[30];
    unsigned int pending_count = 0;
    MySQL* const db = GetPrimaryDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect id,cera,cera_check,item_no,item_server_id,item_charac_no,item_check,avata_no1,avata_no2,avata_server_id,avata_charac_no,avata_check1,avata_check2 from event_ati_info where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventATI::ArrangeATIEventItem(memberIdentificationNumber_t, unsigned int)",
            161,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventATI::ArrangeATIEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int entry_id = 0;
        unsigned int cera = 0;
        unsigned int cera_check = 0;
        unsigned int item_no = 0;
        unsigned int item_server_id = 0;
        unsigned int item_charac_no = 0;
        unsigned int item_check = 0;
        unsigned int avatar_no1 = 0;
        unsigned int avatar_no2 = 0;
        unsigned int avatar_server_id = 0;
        unsigned int avatar_charac_no = 0;
        unsigned int avatar_check1 = 0;
        unsigned int avatar_check2 = 0;
        db->get_int(0, entry_id);
        db->get_int(1, cera);
        db->get_int(2, cera_check);
        db->get_int(3, item_no);
        db->get_int(4, item_server_id);
        db->get_int(5, item_charac_no);
        db->get_int(6, item_check);
        db->get_int(7, avatar_no1);
        db->get_int(8, avatar_no2);
        db->get_int(9, avatar_server_id);
        db->get_int(10, avatar_charac_no);
        db->get_int(11, avatar_check1);
        db->get_int(12, avatar_check2);
        if (item_check == 0 && item_server_id == event_id) {
            pending[pending_count++] = {entry_id, item_charac_no, item_no, 1};
        }
        if (avatar_server_id == event_id && avatar_check1 == 0) {
            pending[pending_count++] =
                {entry_id, avatar_charac_no, avatar_no1, 1};
        }
        if (avatar_server_id == event_id && avatar_check2 == 0) {
            pending[pending_count++] =
                {entry_id, avatar_charac_no, avatar_no2, 1};
        }
    }
    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateEventATIInfo(pending[index].entry_id, pending[index].item_no);
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 2);
    }
    return pending_count != 0;
}

// IDA_EA: 0x83FA69C
// IDA_NAME: _ZN8WongWork11EventSigong25CheckAlreadyGetSigongItemEji
// DEMANGLED: WongWork::EventSigong::CheckAlreadyGetSigongItem(unsigned int,int)
bool EventSigong::CheckAlreadyGetSigongItem(unsigned int entry_id, int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    switch (CWebEvent::GetItemType(3, item_no)) {
    case 1:
        db->set_query("seLect item1_check from event_sigong_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    case 2:
        db->set_query("seLect item2_check from event_sigong_info where id=%s",
                      NumberToString(entry_id, 0));
        break;
    }
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FA7A5
// IDA_NAME: _ZN8WongWork11EventSigong21UpdateEventSigongInfoEji
// DEMANGLED: WongWork::EventSigong::UpdateEventSigongInfo(unsigned int,int)
int EventSigong::UpdateEventSigongInfo(unsigned int entry_id, int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    switch (CWebEvent::GetItemType(3, item_no)) {
    case 1:
        db->set_query(
            "upDate event_sigong_info set item1_check=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    case 2:
        db->set_query(
            "upDate event_sigong_info set item2_check=unix_timestamp(now()) where id=%s",
            NumberToString(entry_id, 0));
        break;
    }
    return db->exec(true);
}

// IDA_EA: 0x83FA855
// IDA_NAME: _ZN8WongWork11EventSigong22ArrangeSigongEventItemEjj
// DEMANGLED: WongWork::EventSigong::ArrangeSigongEventItem(unsigned int,unsigned int)
unsigned char EventSigong::ArrangeSigongEventItem(unsigned int account_id,
                                                   unsigned int event_id) {
    PendingEventItem pending[20];
    unsigned int pending_count = 0;
    MySQL* const db = GetPrimaryDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect id,item1_no,item1_server_id,item1_charac_no,item1_check,item2_no,item2_server_id,item2_charac_no,item2_check from event_sigong_info where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventSigong::ArrangeSigongEventItem(memberIdentificationNumber_t, unsigned int)",
            291,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventSigong::ArrangeSigongEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int entry_id = 0;
        unsigned int item1_no = 0;
        unsigned int item1_server_id = 0;
        unsigned int item1_charac_no = 0;
        unsigned int item1_check = 0;
        unsigned int item2_no = 0;
        unsigned int item2_server_id = 0;
        unsigned int item2_charac_no = 0;
        unsigned int item2_check = 0;
        db->get_int(0, entry_id);
        db->get_int(1, item1_no);
        db->get_int(2, item1_server_id);
        db->get_int(3, item1_charac_no);
        db->get_int(4, item1_check);
        db->get_int(5, item2_no);
        db->get_int(6, item2_server_id);
        db->get_int(7, item2_charac_no);
        db->get_int(8, item2_check);
        if (item1_check == 0 && item1_server_id == event_id) {
            pending[pending_count++] = {entry_id, item1_charac_no, item1_no, 1};
        }
        if (item2_check == 0 && item2_server_id == event_id) {
            pending[pending_count++] = {entry_id, item2_charac_no, item2_no, 200};
        }
    }
    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateEventSigongInfo(pending[index].entry_id, pending[index].item_no);
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 3);
    }
    return pending_count != 0;
}

// IDA_EA: 0x83FAC0D
// IDA_NAME: _ZN8WongWork13EventEverland27CheckAlreadyGetEverlandItemEji
// DEMANGLED: WongWork::EventEverland::CheckAlreadyGetEverlandItem(unsigned int,int)
bool EventEverland::CheckAlreadyGetEverlandItem(unsigned int entry_id,
                                                 int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    // Preserve the old kind-4 conditional literally.  `GetItemType` owns
    // the historical classification table; this event body must not invent a
    // substitute item-type mapping.
    if (CWebEvent::GetItemType(4, item_no) == 1) {
        db->set_query("seLect coin_check from event_everland_info where id=%s",
                      NumberToString(entry_id, 0));
    }
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FACE3
// IDA_NAME: _ZN8WongWork13EventEverland23UpdateEventEverlandInfoEji
// DEMANGLED: WongWork::EventEverland::UpdateEventEverlandInfo(unsigned int,int)
int EventEverland::UpdateEventEverlandInfo(unsigned int entry_id,
                                           int item_no) {
    MySQL* const db = GetPrimaryDBHandle();
    if (CWebEvent::GetItemType(4, item_no) == 1) {
        db->set_query(
            "upDate event_everland_info set coin_check=unix_timestamp(now()) "
            "where id=%s",
            NumberToString(entry_id, 0));
    }
    return db->exec(true);
}

// IDA_EA: 0x83FAD60
// IDA_NAME: _ZN8WongWork13EventEverland24ArrangeEverlandEventItemEjj
// DEMANGLED: WongWork::EventEverland::ArrangeEverlandEventItem(unsigned int,unsigned int)
unsigned char EventEverland::ArrangeEverlandEventItem(unsigned int account_id,
                                                       unsigned int event_id) {
    PendingEventItem pending[10];
    unsigned int pending_count = 0;
    MySQL* const db = GetPrimaryDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect id,server_id,charac_no,coin,coin_check from "
        "event_everland_info where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventEverland::ArrangeEverlandEventItem(memberIdentificationNumber_t, unsigned int)",
            386,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventEverland::ArrangeEverlandEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int entry_id = 0;
        unsigned int server_id = 0;
        unsigned int charac_no = 0;
        unsigned int coin = 0;
        unsigned int coin_check = 0;
        db->get_int(0, entry_id);
        db->get_int(1, server_id);
        db->get_int(2, charac_no);
        db->get_int(3, coin);
        db->get_int(4, coin_check);
        if (coin_check == 0 && server_id == event_id) {
            pending[pending_count++] = {entry_id, charac_no, 1, coin};
        }
    }

    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateEventEverlandInfo(pending[index].entry_id,
                                static_cast<int>(pending[index].item_no));
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 3);
    }
    return static_cast<unsigned char>(pending_count != 0);
}

// IDA_EA: 0x83FB047
// IDA_NAME: _ZN8WongWork8EventMuz22CheckAlreadyGetMuzItemEji
// DEMANGLED: WongWork::EventMuz::CheckAlreadyGetMuzItem(unsigned int,int)
bool EventMuz::CheckAlreadyGetMuzItem(unsigned int entry_id, int) {
    MySQL* const db = GetPrimaryDBHandle();
    db->set_query("seLect item_check from event_muz_entry where id=%s",
                  NumberToString(entry_id, 0));
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FB105
// IDA_NAME: _ZN8WongWork8EventMuz18UpdateEventMuzInfoEji
// DEMANGLED: WongWork::EventMuz::UpdateEventMuzInfo(unsigned int,int)
int EventMuz::UpdateEventMuzInfo(unsigned int entry_id, int) {
    MySQL* const db = GetPrimaryDBHandle();
    db->set_query(
        "upDate event_muz_entry set item_check=unix_timestamp(now()) where id=%s",
        NumberToString(entry_id, 0));
    return db->exec(true);
}

// IDA_EA: 0x83FB16A
// IDA_NAME: _ZN8WongWork8EventMuz19ArrangeMuzEventItemEjj
// DEMANGLED: WongWork::EventMuz::ArrangeMuzEventItem(unsigned int,unsigned int)
unsigned char EventMuz::ArrangeMuzEventItem(unsigned int account_id,
                                             unsigned int event_id) {
    PendingEventItem pending[10];
    unsigned int pending_count = 0;
    MySQL* const db = GetPrimaryDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect id,item_server_id,item_charac_no,item_no,item_check,item_cnt from event_muz_entry where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventMuz::ArrangeMuzEventItem(memberIdentificationNumber_t, unsigned int)",
            462,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventMuz::ArrangeMuzEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int entry_id = 0;
        unsigned int item_server_id = 0;
        unsigned int charac_no = 0;
        unsigned int item_no = 0;
        unsigned int item_check = 0;
        unsigned int item_count = 0;
        db->get_int(0, entry_id);
        db->get_int(1, item_server_id);
        db->get_int(2, charac_no);
        db->get_int(3, item_no);
        db->get_int(4, item_check);
        db->get_int(5, item_count);
        if (item_check == 0 && item_server_id == event_id) {
            pending[pending_count++] = {entry_id, charac_no, item_no, item_count};
        }
    }
    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateEventMuzInfo(pending[index].entry_id, 0);
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 5);
    }
    return pending_count != 0;
}

// IDA_EA: 0x83FB472
// IDA_NAME: _ZN8WongWork11EventAlttle25CheckAlreadyGetAlttleItemEji
// DEMANGLED: WongWork::EventAlttle::CheckAlreadyGetAlttleItem(unsigned int,int)
bool EventAlttle::CheckAlreadyGetAlttleItem(unsigned int entry_id, int) {
    MySQL* const db = GetPrimaryDBHandle();
    db->set_query("seLect item_check from event_alttle_info where m_id=%s",
                  NumberToString(entry_id, 0));
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FB530
// IDA_NAME: _ZN8WongWork11EventAlttle21UpdateEventAlttleInfoEji
// DEMANGLED: WongWork::EventAlttle::UpdateEventAlttleInfo(unsigned int,int)
int EventAlttle::UpdateEventAlttleInfo(unsigned int entry_id, int) {
    MySQL* const db = GetPrimaryDBHandle();
    db->set_query(
        "upDate event_alttle_info set item_check=unix_timestamp(now()) where m_id=%s",
        NumberToString(entry_id, 0));
    return db->exec(true);
}

// IDA_EA: 0x83FB595
// IDA_NAME: _ZN8WongWork11EventAlttle22ArrangeAlttleEventItemEjj
// DEMANGLED: WongWork::EventAlttle::ArrangeAlttleEventItem(unsigned int,unsigned int)
unsigned char EventAlttle::ArrangeAlttleEventItem(unsigned int account_id,
                                                   unsigned int event_id) {
    PendingEventItem pending[10];
    unsigned int pending_count = 0;
    MySQL* const db = GetPrimaryDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect item_server_id,item_charac_no,item_no,item_check,item_cnt from event_alttle_info where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventAlttle::ArrangeAlttleEventItem(memberIdentificationNumber_t, unsigned int)",
            539,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventAlttle::ArrangeAlttleEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int item_server_id = 0;
        unsigned int charac_no = 0;
        unsigned int item_no = 0;
        unsigned int item_check = 0;
        unsigned int item_count = 0;
        db->get_int(0, item_server_id);
        db->get_int(1, charac_no);
        db->get_int(2, item_no);
        db->get_int(3, item_check);
        db->get_int(4, item_count);
        if (item_check == 0 && item_server_id == event_id) {
            pending[pending_count++] = {account_id, charac_no, item_no, item_count};
        }
    }
    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateEventAlttleInfo(pending[index].entry_id, 0);
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 6);
    }
    return pending_count != 0;
}

// IDA_EA: 0x83FB883
// IDA_NAME: _ZN8WongWork12EventFCCombo28CheckAlreadyFCComboEventItemEji
// DEMANGLED: WongWork::EventFCCombo::CheckAlreadyFCComboEventItem(unsigned int,int)
bool EventFCCombo::CheckAlreadyFCComboEventItem(unsigned int entry_id, int) {
    MySQL* const db = GetDBHandle();
    db->set_query("seLect item_check from event_combo_entry where id=%s",
                  NumberToString(entry_id, 0));
    if (!db->exec(true)) {
        return false;
    }
    int check_time = 0;
    return db->get_int(0, check_time) && check_time != 0;
}

// IDA_EA: 0x83FB929
// IDA_NAME: _ZN8WongWork12EventFCCombo22UpdateFCComboEventInfoEji
// DEMANGLED: WongWork::EventFCCombo::UpdateFCComboEventInfo(unsigned int,int)
int EventFCCombo::UpdateFCComboEventInfo(unsigned int entry_id, int) {
    MySQL* const db = GetDBHandle();
    db->set_query(
        "upDate event_combo_entry set item_check=unix_timestamp(now()) where id=%s",
        NumberToString(entry_id, 0));
    return db->exec(true);
}

// IDA_EA: 0x83FB976
// IDA_NAME: _ZN8WongWork12EventFCCombo23ArrangeFCComboEventItemEjj
// DEMANGLED: WongWork::EventFCCombo::ArrangeFCComboEventItem(unsigned int,unsigned int)
unsigned char EventFCCombo::ArrangeFCComboEventItem(unsigned int account_id,
                                                     unsigned int event_id) {
    PendingEventItem pending[10];
    unsigned int pending_count = 0;
    MySQL* const db = GetDBHandle();
    const char* const account_text = NumberToString(account_id, 0);
    db->set_query(
        "seLect id,item_server_id,item_charac_no,item_no,item_check,item_cnt from event_combo_entry where m_id=%s and occ_check!=0 limit %d",
        account_text,
        10);
    if (!db->exec(true)) {
        cMyTrace trace(
            "void WongWork::EventFCCombo::ArrangeFCComboEventItem(memberIdentificationNumber_t, unsigned int)",
            634,
            5);
        return static_cast<unsigned char>(
            trace("%s, exec ERROR m_id=%s",
                  "void WongWork::EventFCCombo::ArrangeFCComboEventItem(memberIdentificationNumber_t, unsigned int)",
                  account_text));
    }

    const unsigned int row_count = static_cast<unsigned int>(db->get_n_rows());
    for (unsigned int index = 0; index < row_count && db->fetch(); ++index) {
        unsigned int entry_id = 0;
        unsigned int item_server_id = 0;
        unsigned int charac_no = 0;
        unsigned int item_no = 0;
        unsigned int item_check = 0;
        unsigned int item_count = 0;
        db->get_int(0, entry_id);
        db->get_int(1, item_server_id);
        db->get_int(2, charac_no);
        db->get_int(3, item_no);
        db->get_int(4, item_check);
        db->get_int(5, item_count);
        if (item_check == 0 && item_server_id == event_id) {
            pending[pending_count++] = {entry_id, charac_no, item_no, item_count};
        }
    }
    for (unsigned int index = 0; index < pending_count; ++index) {
        UpdateFCComboEventInfo(pending[index].entry_id, 0);
        EventCommon::InsertEventItem2EventTable(pending[index].charac_no,
                                                 pending[index].item_no,
                                                 pending[index].stack_count,
                                                 7);
    }
    return pending_count != 0;
}

// IDA_EA: 0x844DC7C
// IDA_NAME: _ZN8WongWork12EventFCCombo11GetDBHandleEv
// DEMANGLED: WongWork::EventFCCombo::GetDBHandle(void)
MySQL* EventFCCombo::GetDBHandle() {
    return GetPrimaryDBHandle();
}

}  // namespace WongWork
