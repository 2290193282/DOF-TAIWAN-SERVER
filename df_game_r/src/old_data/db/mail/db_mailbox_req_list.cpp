#include "old_data/db/mail/db_mailbox_req_list.h"

#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_data/db/core/mysql.h"
#include "common/manager/db_mgr.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/equipment/item_option.h"
#include "old_data/db/inventory/db_item_growth_state.h"
#include "old_game/economy/mail/mailbox_public_abi.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <algorithm>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <vector>



namespace {

enum {
    kDbSignalMailboxList = 127,
    kSigMailboxListSize = 0x33DA,
    kMailboxListDbHandle = 3,
    kMailboxListDefaultServerGroup = 0,
    kMailboxListExpireDays = 15,
    kMailboxListPageLimit = 20,
};

typedef char SIG_MAILBOX_LIST_Size_Must_Be_0x33DA[
    (sizeof(SIG_MAILBOX_LIST) == kSigMailboxListSize) ? 1 : -1];

static_assert(sizeof(SIG_MAILBOX_RANDOM_OPTION) == sizeof(RandomOption),
              "mail random-option wire record must retain the IDA 14-byte layout");

}  // namespace

// IDA_EA: 0x844E2F6
// IDA_NAME: _ZN19DB_MailBox_Req_ListC2Ev
// DEMANGLED: DB_MailBox_Req_List::DB_MailBox_Req_List(void)
DB_MailBox_Req_List::DB_MailBox_Req_List() {}

// IDA_EA: 0x845F520
// IDA_NAME: _ZN19DB_MailBox_Req_ListD2Ev
// DEMANGLED: DB_MailBox_Req_List::~DB_MailBox_Req_List()
// IDA_EA: 0x845F550
// IDA_NAME: _ZN19DB_MailBox_Req_ListD0Ev
// DEMANGLED: DB_MailBox_Req_List::~DB_MailBox_Req_List() deleting destructor
DB_MailBox_Req_List::~DB_MailBox_Req_List() {}

// IDA_EA: 0x841D52C
// IDA_NAME: _ZN19DB_MailBox_Req_List12SelectLetterEP5MySQLiijP16SIG_MAILBOX_LIST
// DEMANGLED: DB_MailBox_Req_List::SelectLetter(MySQL *,int,int,unsigned int,SIG_MAILBOX_LIST *)
bool DB_MailBox_Req_List::SelectLetter(MySQL* db_handle,
                                       int user_idx,
                                       int charac_no,
                                       unsigned int last_letter_id,
                                       SIG_MAILBOX_LIST* list) {
    (void)user_idx;

    memset(list->response.letters, 0, sizeof(list->response.letters));
    if (last_letter_id) {
        db_handle->set_query(
            "seLect letter_id,send_charac_no,send_charac_name,letter_text,unix_timestamp(reg_date),stat from letter where (charac_no=%d and stat = 3) or (charac_no=%d and letter_id > %d and reg_date>subdate(now(),interval %d day)) order by letter_id",
            charac_no,
            charac_no,
            last_letter_id,
            kMailboxListExpireDays);
    } else {
        db_handle->set_query(
            "deLete from letter where charac_no=%d and reg_date<subdate(now(),interval %d day) and stat=%d",
            charac_no,
            kMailboxListExpireDays,
            2);
        if (db_handle->exec(true) != true) {
            return false;
        }
        db_handle->set_query(
            "deLete from letter where charac_no=%d and reg_date<subdate(now(),interval %d day) and stat=%d",
            charac_no,
            kMailboxListExpireDays,
            1);
        if (db_handle->exec(true) != true) {
            return false;
        }
        db_handle->set_query(
            "seLect letter_id,send_charac_no,send_charac_name,letter_text,unix_timestamp(reg_date),stat from letter where (charac_no=%d and stat = 3) or (charac_no=%d and reg_date>subdate(now(),interval %d day)) order by letter_id",
            charac_no,
            charac_no,
            kMailboxListExpireDays);
    }

    if (db_handle->exec(true) != true) {
        return false;
    }

    const int mail_count_before = list->response.mail_count;
    const int requested_count = list->response.letter_count;
    list->response.letter_count = db_handle->get_n_rows();

    std::vector<SIG_LETTER_INFO> contained_letters;
    std::vector<SIG_LETTER_INFO> not_contained_letters;
    std::vector<SIG_LETTER_INFO> stat3_letters;

    for (int index = 0; list->response.letter_count > index;
         ++index) {
        if (last_letter_id) {
            if (mail_count_before >= requested_count) {
                if (list->response.not_loaded_mail_count &&
                    static_cast<int>(contained_letters.size()) >=
                        requested_count) {
                    break;
                }
            }
        }

        if (db_handle->fetch() != true) {
            return false;
        }

        SIG_LETTER_INFO letter;
        memset(&letter, 0, sizeof(letter));
        unsigned int letter_id = 0;
        int send_charac_no = 0;
        int reg_time = 0;
        short stat = 0;
        if (db_handle->get_uint(0, letter_id) != true ||
            db_handle->get_int(1, send_charac_no) != true ||
            db_handle->get_str(2,
                               letter.send_charac_name,
                               sizeof(letter.send_charac_name)) != true ||
            db_handle->get_str(3,
                               letter.letter_text,
                               sizeof(letter.letter_text)) != true ||
            db_handle->get_int(4, reg_time) != true ||
            db_handle->get_short(5, stat) != true) {
            return false;
        }
        letter.letter_id = letter_id;
        letter.send_charac_no = send_charac_no;
        letter.reg_time = reg_time;
        letter.stat = stat;

        if (stat >= 1) {
            if (stat <= 2) {
                if (IsContainedLetter(list, static_cast<int>(letter.letter_id))) {
                    contained_letters.push_back(letter);
                } else {
                    not_contained_letters.push_back(letter);
                }
            } else if (stat == 3) {
                stat3_letters.push_back(letter);
            }
        }
    }

    const int excess =
        static_cast<int>(not_contained_letters.size()) +
        static_cast<int>(contained_letters.size()) -
        list->response.requested_mail_count;
    if (excess > 0) {
        ArrangeMailAndLetterIndex(list,
                                  not_contained_letters,
                                  excess,
                                  contained_letters);
        if (list->response.mail_count) {
            list->response.last_postal_id = list->response.mails[
                list->response.mail_count - 1].postal_id;
        }
    }

    int write_counts[3] = {0, 0, 0};
    int removed_new_letters = 0;
    if (list->response.letter_count) {
        write_counts[0] = static_cast<int>(contained_letters.size());
        write_counts[1] = static_cast<int>(not_contained_letters.size());
        write_counts[2] = static_cast<int>(stat3_letters.size());
        if (write_counts[0] > list->response.requested_mail_count) {
            write_counts[0] = list->response.requested_mail_count;
        }
        if (write_counts[1] + list->response.mail_count >
            list->response.requested_mail_count) {
            removed_new_letters =
                write_counts[1] + list->response.mail_count -
                list->response.requested_mail_count;
            write_counts[1] -= removed_new_letters;
        }
        if (write_counts[2] > 10) {
            write_counts[2] = 10;
        }
    }

    std::vector<SIG_LETTER_INFO>* groups[3] = {
        &contained_letters,
        &not_contained_letters,
        &stat3_letters,
    };
    int write_count = 0;
    for (int group = 0; group <= 2; ++group) {
        for (int index = 0; write_counts[group] > index; ++index) {
            list->response.letters[write_count] = (*groups[group])[index];
            ++write_count;
        }
    }

    list->response.loaded_letter_adjusted = write_counts[1];
    const int carryover_delta =
        removed_new_letters -
        list->response.letter_carryover_count;
    list->response.not_loaded_mail_count +=
        std::max(carryover_delta, 0);
    list->response.letter_count = write_count;
    return true;
}

// IDA_EA: 0x841DC84
// IDA_NAME: _ZN19DB_MailBox_Req_List15setCreatureDataEP5MySQLjPjP16SIG_MAILBOX_LIST
// DEMANGLED: DB_MailBox_Req_List::setCreatureData(MySQL *,unsigned int,unsigned int *,SIG_MAILBOX_LIST *)
bool DB_MailBox_Req_List::setCreatureData(MySQL* db_handle,
                                          unsigned int charac_no,
                                          unsigned int* ui_id_list,
                                          SIG_MAILBOX_LIST* list) {
    (void)charac_no;

    char in_query[4096];
    memset(in_query, 0, sizeof(in_query));
    getList2inQuery(ui_id_list, in_query);
    db_handle->set_query(
        "seLect ui_id,unix_timestamp(expire_date),unix_timestamp(reg_date) from creature_items where ui_id %s and stat = 1",
        in_query);
    db_handle->exec(true);
    list->response.creature_count = db_handle->get_n_rows();

    for (unsigned int row = 0;
         static_cast<unsigned int>(db_handle->get_n_rows()) > row;
         ++row) {
        int ui_id = 0;
        int expire_time = 0;
        int reg_time = 0;
        if (db_handle->fetch() != true ||
            db_handle->get_int(0, ui_id) != true ||
            db_handle->get_int(1, expire_time) != true ||
            db_handle->get_int(2, reg_time) != true) {
            return false;
        }
        SIG_MAILBOX_CREATURE_ENTRY& creature =
            list->response.creatures[row];
        creature.ui_id = ui_id;
        creature.expire_time = expire_time;
        creature.reg_time = reg_time;
    }
    return true;
}

// IDA_EA: 0x841DE4A
// IDA_NAME: _ZN19DB_MailBox_Req_List8dispatchEiiP6Stream
// DEMANGLED: DB_MailBox_Req_List::dispatch(int,int,Stream *)
bool DB_MailBox_Req_List::dispatch(int signal,
                                   int user_idx,
                                   Stream* stream) {
    if (DBDispatcher::dispatch(signal, user_idx, stream) != true) {
        return false;
    }

    uint16_t growth_route_magic = 0u;
    uint8_t growth_server_group = 0xffu;
    if (stream->length() != kSigMailboxListSize + 3 ||
        (*stream >> growth_route_magic) != 1 ||
        (*stream >> growth_server_group) != 1 ||
        growth_route_magic != 0x8247u || growth_server_group >= 0x12u) {
        return false;
    }
    SIG_MAILBOX_LIST* request = stream->GetOutBuffer<SIG_MAILBOX_LIST>();
    if (!request) {
        return false;
    }

    SIG_MAILBOX_LIST result;
    memset(&result, 0, sizeof(result));

    MySQL* db_handle = GlobalData::s_db_mgr->GetDBHandle(static_cast<ENUM_DB_HANDLE_IDX>(
        kMailboxListDbHandle),static_cast<ENUM_SERVER_GROUP>(
        kMailboxListDefaultServerGroup));

    const int charac_no = request->request.charac_no;
    if (!request->request.last_postal_id) {
        db_handle->set_query(
            "upDate postal set delete_flag=%d where receive_charac_no=%d and unlimit_flag=0 and occ_time<subdate(now(),interval %d day) and delete_flag=0",
            2,
            charac_no,
            kMailboxListExpireDays);
        if (db_handle->exec(true) != true) {
            return false;
        }
    }

    char postal_where[128];
    memset(postal_where, 0, sizeof(postal_where));
    if (request->request.last_postal_id) {
        sprintf(postal_where,
                " and postal_id > %d",
                request->request.last_postal_id);
    }

    db_handle->set_query(
        "seLect postal_id,unix_timestamp(occ_time),send_charac_no,seal_flag,item_id,add_info,endurance,upgrade,gold,send_charac_name,unlimit_flag,avata_flag,creature_flag,letter_id,extend_info,item_guid,amplify_option,amplify_value,random_option,chronicle_value,seperate_upgrade,type from postal where receive_charac_no=%d and delete_flag=0 %s and (unlimit_flag=1 or occ_time>subdate(now(),interval 15 day)) order by postal_id limit %d",
        charac_no,
        postal_where,
        kMailboxListPageLimit);
    if (db_handle->exec(true) != true) {
        return false;
    }

    std::map<unsigned int, int> package_load_lack;
    for (unsigned int index = 0; index <= 19; ++index) {
        const unsigned int ui_id = request->request.package_load_lack[index];
        if (ui_id) {
            package_load_lack.insert(std::make_pair(ui_id, 0));
        }
    }

    result.response.requested_mail_count = request->request.remain_count;
    result.response.last_postal_id = 0;
    const int row_count = db_handle->get_n_rows();
    const int request_count = request->request.remain_count;
    result.response.mail_count = std::min(request_count, row_count);
    result.response.not_loaded_mail_count = 0;
    if (result.response.mail_count >= request_count) {
        result.response.not_loaded_mail_count =
            row_count - result.response.mail_count;
    }

    unsigned int avatar_ui_ids[21];
    unsigned int creature_ui_ids[21];
    memset(avatar_ui_ids, 0, sizeof(avatar_ui_ids));
    memset(creature_ui_ids, 0, sizeof(creature_ui_ids));

    int written_mail_count = 0;
    for (int row = 0;
         result.response.mail_count > row && row <= 19;
         ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }

        int postal_id = 0;
        int occ_time = 0;
        int send_charac_no = 0;
        short seal_flag = 0;
        unsigned long item_id = 0;
        int add_info = 0;
        short endurance = 0;
        char upgrade = 0;
        int gold = 0;
        char send_charac_name[29];
        short unlimit_flag = 0;
        short avatar_flag = 0;
        short creature_flag = 0;
        unsigned int letter_id = 0;
        unsigned long extend_info = 0;
        unsigned char item_guid[10];
        char amplify_option = 0;
        short amplify_value = 0;
        RandomOption random_option;
        unsigned int chronicle_value = 0;
        char separate_upgrade = 0;
        char type = 0;
        memset(send_charac_name, 0, sizeof(send_charac_name));
        memset(item_guid, 0, sizeof(item_guid));
        random_option.reset();

        if (db_handle->get_int(0, postal_id) != true ||
            db_handle->get_int(1, occ_time) != true ||
            db_handle->get_int(2, send_charac_no) != true ||
            db_handle->get_short(3, seal_flag) != true ||
            db_handle->get_ulong(4, item_id) != true ||
            db_handle->get_int(5, add_info) != true ||
            db_handle->get_short(6, endurance) != true ||
            db_handle->get_byte(7, upgrade) != true ||
            db_handle->get_int(8, gold) != true ||
            db_handle->get_str(9,
                               send_charac_name,
                               sizeof(send_charac_name)) != true ||
            db_handle->get_short(10, unlimit_flag) != true ||
            db_handle->get_short(11, avatar_flag) != true ||
            db_handle->get_short(12, creature_flag) != true ||
            (avatar_flag == 1 && creature_flag == 1)) {
            continue;
        }

        int column = 14;
        if (db_handle->get_uint(13, letter_id) == true &&
            (item_id || gold)) {
            if (db_handle->get_ulong(column++, extend_info) != true ||
                db_handle->get_binary(column++, item_guid, sizeof(item_guid)) !=
                    true ||
                db_handle->get_byte(column++, amplify_option) != true ||
                db_handle->get_short(column++, amplify_value) != true ||
                db_handle->get_binary(column++,
                                      &random_option,
                                      sizeof(random_option)) != true ||
                db_handle->get_uint(column++, chronicle_value) != true ||
                db_handle->get_byte(column++, separate_upgrade) != true ||
                db_handle->get_byte(column++, type) != true) {
                continue;
            }

            if (letter_id) {
                package_load_lack.insert(std::make_pair(letter_id, 0));
            }

            SIG_MAILBOX_MAIL_ENTRY& mail =
                result.response.mails[written_mail_count];
            mail.postal_id = static_cast<uint32_t>(postal_id);
            mail.expire_time = occ_time + 1296000;
            mail.send_charac_no = send_charac_no;
            mail.seal_flag = seal_flag == 1;
            mail.item_id = static_cast<uint32_t>(item_id);
            mail.add_info = add_info;
            mail.endurance = static_cast<uint16_t>(endurance);
            mail.upgrade = static_cast<uint8_t>(upgrade);
            mail.extend_info = static_cast<uint32_t>(extend_info);
            memcpy(mail.item_guid, item_guid, sizeof(mail.item_guid));
            mail.gold = gold;
            mail.unlimit_flag = unlimit_flag == 1;
            if (avatar_flag == 1) {
                mail.avatar_flag = 1;
                avatar_ui_ids[++avatar_ui_ids[0]] =
                    static_cast<unsigned int>(add_info);
            } else {
                mail.avatar_flag = 0;
            }
            if (creature_flag == 1) {
                mail.creature_flag = 1;
                creature_ui_ids[++creature_ui_ids[0]] =
                    static_cast<unsigned int>(add_info);
            } else {
                mail.creature_flag = 0;
            }
            mail.letter_id = letter_id;
            mail.amplify_option = static_cast<uint8_t>(amplify_option);
            mail.amplify_value = static_cast<uint16_t>(amplify_value);
            mail.type = static_cast<uint8_t>(type);
            for (int option_index = 0; option_index < 3; ++option_index) {
                const RandomOptionField& option =
                    random_option.option_fields_[option_index];
                SIG_MAILBOX_RANDOM_OPTION_FIELD& serialized =
                    mail.random_option.option_fields[option_index];
                serialized.option_index = option.option_index_;
                serialized.first_value = option.first_value_;
                serialized.second_value = option.second_value_;
            }
            mail.random_option.initial_seed = random_option.initial_seed_.seed_;
            mail.random_option.changed_option.option_index =
                random_option.changed_option_.option_index_;
            mail.random_option.changed_option.first_value =
                random_option.changed_option_.first_value_;
            mail.random_option.changed_option.second_value =
                random_option.changed_option_.second_value_;
            mail.random_option.changed_option_seed =
                random_option.changed_option_seed_.seed_;
            mail.separate_upgrade = static_cast<uint8_t>(separate_upgrade);
            result.response.chronicle_values[written_mail_count] =
                chronicle_value;
            strncpy(mail.send_charac_name, send_charac_name,
                    sizeof(mail.send_charac_name));

            result.response.last_postal_id =
                std::max(result.response.last_postal_id,
                         static_cast<uint32_t>(postal_id));
            ++written_mail_count;
        }
    }

    result.response.mail_count = written_mail_count;
    result.response.letter_carryover_count = 0;
    for (int row = 0; result.response.not_loaded_mail_count > row; ++row) {
        if (db_handle->fetch() != true) {
            return false;
        }
        unsigned int letter_id = 0;
        if (db_handle->get_uint(13, letter_id) == true && letter_id) {
            std::map<unsigned int, int>::iterator it =
                package_load_lack.find(letter_id);
            if (it != package_load_lack.end()) {
                ++it->second;
            }
            ++result.response.letter_carryover_count;
        }
    }

    if (!package_load_lack.empty()) {
        unsigned int changed_count = 0;
        for (std::map<unsigned int, int>::iterator it =
                 package_load_lack.begin();
             it != package_load_lack.end();
            ++it) {
            if (it->second > 0 && changed_count <= 19) {
                result.response.changed_package_ids[changed_count] = it->first;
                ++changed_count;
            }
        }
    }

    const int mail_count_before_select = result.response.mail_count;
    result.response.letter_count = request->request.remain_count;
    if (SelectLetter(db_handle,
                     user_idx,
                     charac_no,
                     static_cast<unsigned int>(request->request.last_letter_id),
                     &result) != true) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_MailBox_Req_List::dispatch(int, int, Stream*)",
            0x5438,
            "SelectLetter() error");
        result.response.letter_count = 0;
    }

    if (result.response.mail_count == mail_count_before_select) {
        if (!result.response.mail_count) {
            result.response.last_postal_id =
                static_cast<uint32_t>(request->request.last_postal_id);
        }
    } else {
        DoChangeMailList(&result,
                         avatar_ui_ids,
                         request->request.last_postal_id);
    }

    result.response.avatar_count = 0;
    if (avatar_ui_ids[0]) {
        char in_query[4096];
        char agency_no[25];
        memset(in_query, 0, sizeof(in_query));
        memset(agency_no, 0, sizeof(agency_no));
        getList2inQuery(avatar_ui_ids, in_query);
        db_handle->set_query(
            "seLect ui_id,unix_timestamp(reg_date),unix_timestamp(expire_date),ipg_agency_no,jewel_socket from user_items where charac_no=%d and ui_id %s",
            charac_no,
            in_query);
        db_handle->exec(true);
        result.response.avatar_count = db_handle->get_n_rows();
        for (unsigned int row = 0;
             static_cast<unsigned int>(db_handle->get_n_rows()) > row;
             ++row) {
            int ui_id = 0;
            unsigned int reg_time = 0;
            unsigned int expire_time = 0;
            stAvatarEmblemInfo_t emblem_info;
            memset(agency_no, 0, sizeof(agency_no));
            emblem_info.init();
            if (db_handle->fetch() != true ||
                db_handle->get_int(0, ui_id) != true ||
                db_handle->get_uint(1, reg_time) != true ||
                db_handle->get_uint(2, expire_time) != true ||
                db_handle->get_str(3, agency_no, 24) != true ||
                db_handle->get_binary(4,
                                      emblem_info.sockets,
                                      sizeof(emblem_info.sockets)) != true) {
                return false;
            }
            SIG_MAILBOX_AVATAR_ENTRY& avatar = result.response.avatars[row];
            avatar.expire_time = static_cast<int32_t>(expire_time);
            avatar.ui_id = ui_id;
            avatar.reg_time = reg_time;
            strncpy(avatar.ipg_agency_no, agency_no,
                    sizeof(avatar.ipg_agency_no) - 1u);
            for (size_t socket = 0; socket != 5u; ++socket) {
                avatar.emblem_info.sockets[socket].socket_type =
                    emblem_info.sockets[socket].socket_type;
                avatar.emblem_info.sockets[socket].emblem_item_index =
                    emblem_info.sockets[socket].emblem_item_index;
            }
        }
    }

    result.response.creature_count = 0;
    if (creature_ui_ids[0] &&
        setCreatureData(db_handle, charac_no, creature_ui_ids, &result) !=
            true) {
        return false;
    }

    result.response.charac_no = charac_no;
    result.response.mail_load_count = request->request.mail_load_count;

    // Chronicle values remain in their official parallel response array.
    // SQL006 is loaded only by the native worker, which deep-copies this
    // complete response and forwards signal 127 after the GUID fan-in.
    if (!item_growth_db::QueueMailboxPreload(
            user_idx,
            static_cast<int>(growth_server_group),
            static_cast<uint32_t>(charac_no),
            result)) {
        LogManager::logFormat(
            1,
            "DBThread.cpp",
            "virtual bool DB_MailBox_Req_List::dispatch(int, int, Stream*)",
            21649,
            "equipment growth continuation queue failed charac_no=%d",
            charac_no);
        return false;
    }
    return true;
}
