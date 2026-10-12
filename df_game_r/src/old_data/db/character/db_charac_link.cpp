#include "old_data/db/character/db_charac_link.h"

#include "common/manager/global_data.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "old_data/db/core/db_mgr.h"
#include "old_data/db/core/mysql.h"

#include <cstring>


namespace charac_link_db {
namespace {

bool IsValidServerGroup(int server_group) {
    return (server_group >= 0 && server_group < 0x12) ||
           server_group == 99 || server_group == 100;
}

bool HasEmptyRecords(const Message& message) {
    for (int index = 0; index < kMaximumLinkRecords; ++index) {
        if (message.records[index].charac_no != 0u ||
            message.records[index].link_charac_no != 0u) {
            return false;
        }
    }
    return true;
}

bool IsValidRequest(const Message& message, int user_idx) {
    if (user_idx == -1 || message.user_idx != user_idx ||
        message.account_id == 0u ||
        !IsValidServerGroup(message.server_group) ||
        message.session_token == 0u || message.request_token == 0u ||
        message.result != static_cast<uint8_t>(kResultRequest) ||
        message.record_count != 0u || message.reserved != 0u ||
        !HasEmptyRecords(message)) {
        return false;
    }

    if (message.operation == kOperationLoad) {
        return message.first_charac_no == 0u &&
               message.second_charac_no == 0u &&
               message.first_slot == 0u && message.second_slot == 0u &&
               message.first_grow_type == 0u &&
               message.second_grow_type == 0u;
    }
    if (message.operation != kOperationConnect &&
        message.operation != kOperationDisconnect) {
        return false;
    }
    if (message.first_charac_no == 0u ||
        message.second_charac_no == 0u ||
        message.first_charac_no == message.second_charac_no ||
        message.first_slot == message.second_slot) {
        return false;
    }
    if (message.operation == kOperationDisconnect) {
        return message.first_grow_type == 0u &&
               message.second_grow_type == 0u;
    }
    return true;
}

bool SendResult(int user_idx, const Message& response) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const output = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!output) {
        return false;
    }
    *output = response;
    return queues->put(MsgQueueMgr::DISPATCH_Q, guard) == 1;
}

bool AreCharactersMutuallyLinked(MySQL* db,
                                 uint32_t account_id,
                                 uint32_t first_charac_no,
                                 uint32_t second_charac_no) {
    return db &&
           db->set_query(
               "seLect charac_no from charac_info "
               "where m_id=%u and delete_flag=0 and "
               "((charac_no=%u and link_charac_no=%u) or "
               "(charac_no=%u and link_charac_no=%u))",
               static_cast<unsigned int>(account_id),
               static_cast<unsigned int>(first_charac_no),
               static_cast<unsigned int>(second_charac_no),
               static_cast<unsigned int>(second_charac_no),
               static_cast<unsigned int>(first_charac_no)) &&
           db->exec(true) && db->get_n_rows() == 2;
}

bool AreCharactersUnlinked(MySQL* db,
                           uint32_t account_id,
                           uint32_t first_charac_no,
                           uint32_t second_charac_no) {
    return db &&
           db->set_query(
               "seLect charac_no from charac_info "
               "where m_id=%u and delete_flag=0 and link_charac_no=0 "
               "and charac_no in (%u,%u)",
               static_cast<unsigned int>(account_id),
               static_cast<unsigned int>(first_charac_no),
               static_cast<unsigned int>(second_charac_no)) &&
           db->exec(true) && db->get_n_rows() == 2;
}

bool LoadLinks(MySQL* db, Message& response) {
    if (!db ||
        !db->set_query(
            "seLect charac_no,link_charac_no from charac_info "
            "where m_id=%u and delete_flag=0 and link_charac_no<>0 "
            "order by charac_no limit %u",
            static_cast<unsigned int>(response.account_id),
            static_cast<unsigned int>(kMaximumLinkRecords + 1)) ||
        !db->exec(true)) {
        return false;
    }

    const int row_count = db->get_n_rows();
    if (row_count < 0 || row_count > kMaximumLinkRecords) {
        return false;
    }
    for (int row = 0; row < row_count; ++row) {
        unsigned int charac_no = 0u;
        unsigned int link_charac_no = 0u;
        if (!db->fetch() || !db->get_uint(0, charac_no) ||
            !db->get_uint(1, link_charac_no) || charac_no == 0u ||
            link_charac_no == 0u || charac_no == link_charac_no) {
            return false;
        }
        response.records[row].charac_no =
            static_cast<uint32_t>(charac_no);
        response.records[row].link_charac_no =
            static_cast<uint32_t>(link_charac_no);
    }
    response.record_count = static_cast<uint8_t>(row_count);
    return true;
}

bool ConnectLinks(MySQL* db, const Message& request) {
    if (!db ||
        !db->set_query(
            "seLect charac_no,link_charac_no from charac_info "
            "where m_id=%u and delete_flag=0 and charac_no in (%u,%u) "
            "order by charac_no",
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.first_charac_no),
            static_cast<unsigned int>(request.second_charac_no)) ||
        !db->exec(true) || db->get_n_rows() != 2) {
        return false;
    }

    uint32_t charac_nos[2] = {0u, 0u};
    uint32_t links[2] = {0u, 0u};
    for (int row = 0; row < 2; ++row) {
        unsigned int charac_no = 0u;
        unsigned int link_charac_no = 0u;
        if (!db->fetch() || !db->get_uint(0, charac_no) ||
            !db->get_uint(1, link_charac_no)) {
            return false;
        }
        charac_nos[row] = static_cast<uint32_t>(charac_no);
        links[row] = static_cast<uint32_t>(link_charac_no);
    }

    const bool already_linked =
        (charac_nos[0] == request.first_charac_no &&
         links[0] == request.second_charac_no &&
         charac_nos[1] == request.second_charac_no &&
         links[1] == request.first_charac_no) ||
        (charac_nos[1] == request.first_charac_no &&
         links[1] == request.second_charac_no &&
         charac_nos[0] == request.second_charac_no &&
         links[0] == request.first_charac_no);
    if (already_linked) {
        return true;
    }
    if (links[0] != 0u || links[1] != 0u) {
        return false;
    }

    if (!db->set_query(
            "upDate charac_info as first_charac "
            "join charac_info as second_charac "
            "on second_charac.m_id=first_charac.m_id "
            "and second_charac.charac_no=%u "
            "set first_charac.link_charac_no=second_charac.charac_no,"
            "second_charac.link_charac_no=first_charac.charac_no "
            "where first_charac.m_id=%u "
            "and first_charac.charac_no=%u "
            "and first_charac.delete_flag=0 "
            "and second_charac.delete_flag=0 "
            "and first_charac.link_charac_no=0 "
            "and second_charac.link_charac_no=0",
            static_cast<unsigned int>(request.second_charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.first_charac_no)) ||
        !db->exec(true)) {
        return false;
    }
    return db->getAffectedRowCount() == 2u ||
           AreCharactersMutuallyLinked(db,
                                       request.account_id,
                                       request.first_charac_no,
                                       request.second_charac_no);
}

bool DisconnectLinks(MySQL* db, const Message& request) {
    if (!db ||
        !db->set_query(
            "upDate charac_info as first_charac "
            "join charac_info as second_charac "
            "on second_charac.m_id=first_charac.m_id "
            "and second_charac.charac_no=%u "
            "set first_charac.link_charac_no=0,"
            "second_charac.link_charac_no=0 "
            "where first_charac.m_id=%u "
            "and first_charac.charac_no=%u "
            "and first_charac.delete_flag=0 "
            "and second_charac.delete_flag=0 "
            "and (first_charac.link_charac_no=0 or "
            "first_charac.link_charac_no=second_charac.charac_no) "
            "and (second_charac.link_charac_no=0 or "
            "second_charac.link_charac_no=first_charac.charac_no) "
            "and (first_charac.link_charac_no=second_charac.charac_no "
            "or second_charac.link_charac_no=first_charac.charac_no)",
            static_cast<unsigned int>(request.second_charac_no),
            static_cast<unsigned int>(request.account_id),
            static_cast<unsigned int>(request.first_charac_no)) ||
        !db->exec(true)) {
        return false;
    }
    const unsigned long long affected_rows = db->getAffectedRowCount();
    return affected_rows > 0u && affected_rows <= 2u &&
           AreCharactersUnlinked(db,
                                 request.account_id,
                                 request.first_charac_no,
                                 request.second_charac_no);
}

}  // namespace


DB_CharacLink::DB_CharacLink() : DBDispatcher() {}

DB_CharacLink::~DB_CharacLink() {}

bool DB_CharacLink::Queue(int user_idx, const Message& request) {
    StreamPool* const pool = GlobalData::stream_pool();
    MsgQueueMgr* const queues = GlobalData::msgq_mgr();
    if (!IsValidRequest(request, user_idx) || !pool || !queues) {
        return false;
    }

    Stream* const stream = pool->Acquire(__FILE__, __LINE__);
    if (!stream) {
        return false;
    }
    CStreamGuard guard(stream, true);
    *guard << static_cast<int>(kDbSignal);
    *guard << user_idx;
    Message* const queued = reinterpret_cast<Message*>(
        guard.GetInBufferUntyped(static_cast<int>(sizeof(Message))));
    if (!queued) {
        return false;
    }
    *queued = request;
    return queues->put(MsgQueueMgr::FEATURE_DB_Q, guard) == 1;
}

bool DB_CharacLink::dispatch(int signal,
                                 int user_idx,
                                 Stream* stream) {
    if (signal != kDbSignal || !stream ||
        !DBDispatcher::dispatch(signal, user_idx, stream) ||
        stream->length() != static_cast<int>(sizeof(Message))) {
        return false;
    }

    const Message* const request =
        reinterpret_cast<const Message*>(stream->get_buffer());
    if (!request || !IsValidRequest(*request, user_idx)) {
        return false;
    }

    Message response = *request;
    response.result = static_cast<uint8_t>(kResultFailed);
    response.record_count = 0u;
    response.reserved = 0u;
    std::memset(response.records, 0, sizeof(response.records));

    // Handle 18 is a dedicated MYSQL object/socket cloned from logical DB2.
    // Explicit server-group lookup avoids DBMgr::user_idx_ and cannot fall
    // back to any legacy/log connection.
    MySQL* const db = GlobalData::s_db_mgr
                          ? GlobalData::s_db_mgr->GetDBHandleExact(
                                DB_HANDLE_FEATURE_DB2,
                                static_cast<ENUM_SERVER_GROUP>(
                                    response.server_group))
                          : 0;
    bool success = false;
    if (response.operation == kOperationLoad) {
        success = LoadLinks(db, response);
    } else if (response.operation == kOperationConnect) {
        success = ConnectLinks(db, response);
    } else if (response.operation == kOperationDisconnect) {
        success = DisconnectLinks(db, response);
    }
    if (success) {
        response.result = static_cast<uint8_t>(kResultOk);
    } else {
        response.record_count = 0u;
        std::memset(response.records, 0, sizeof(response.records));
    }
    return SendResult(user_idx, response);
}

}  // namespace charac_link_db_0828
