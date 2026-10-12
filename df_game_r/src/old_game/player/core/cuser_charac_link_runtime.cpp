#include "old_game/player/core/character_view_data.h"

#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/user.h"
#include "old_data/db/character/db_charac_link.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <map>


namespace {

// CLIENT_PVF_EVIDENCE: Etc/characlinksystem.etc defines the first-link
// character level interval as 60..70 and the second-link interval as 40..59.
// CLIENT_IDA_EVIDENCE: sub_886B50 stores the selected vector entries and
// sub_1388340 writes their low bytes as CMD309 fields three/four;
// sub_B4AFB0(job, selector) indexes the job's grow-type script table.  The
// selectors are therefore validated against the actual character grow_type,
// rather than persisted as a new database field.
const int kFirstLinkMinimumLevel = 60;
const int kFirstLinkMaximumLevel = 70;
const int kSecondLinkMinimumLevel = 40;
const int kSecondLinkMaximumLevel = 59;

struct LinkSessionState {
    LinkSessionState()
        : user_idx(-1), account_id(0u), server_group(-1),
          session_token(0u), next_request_token(1u),
          pending_request_token(0u), pending_operation(0u) {}

    int32_t user_idx;
    uint32_t account_id;
    int32_t server_group;
    uint32_t session_token;
    uint32_t next_request_token;
    uint32_t pending_request_token;
    uint8_t pending_operation;
};

std::map<CUser*, LinkSessionState>& LinkSessions() {
    // CMD packet dispatch and DB replies are serialized by DISPATCH_Q.  The
    // native DB worker never touches this game-owned session map.
    static std::map<CUser*, LinkSessionState> sessions;
    return sessions;
}

uint32_t NextNonZero(uint32_t& value) {
    uint32_t result = value++;
    if (result == 0u) {
        result = value++;
    }
    if (value == 0u) {
        ++value;
    }
    return result;
}

uint32_t NextSessionToken() {
    static uint32_t next_session_token = 1u;
    return NextNonZero(next_session_token);
}

bool HasCurrentIdentity(CUser* user, const LinkSessionState& state) {
    return user && state.session_token != 0u &&
           state.user_idx == user->GetUID() &&
           state.account_id == static_cast<uint32_t>(user->get_acc_id()) &&
           state.server_group == user->GetServerGroup();
}

LinkSessionState* FindCurrentSession(CUser* user) {
    std::map<CUser*, LinkSessionState>& sessions = LinkSessions();
    std::map<CUser*, LinkSessionState>::iterator found = sessions.find(user);
    if (found == sessions.end() || !HasCurrentIdentity(user, found->second)) {
        return 0;
    }
    return &found->second;
}

void InitializeRequest(CUser* user,
                       LinkSessionState& state,
                       uint8_t operation,
                       charac_link_db::Message& request) {
    std::memset(&request, 0, sizeof(request));
    request.user_idx = user->GetUID();
    request.account_id = static_cast<uint32_t>(user->get_acc_id());
    request.server_group = user->GetServerGroup();
    request.session_token = state.session_token;
    request.request_token = NextNonZero(state.next_request_token);
    request.operation = operation;
    request.result =
        static_cast<uint8_t>(charac_link_db::kResultRequest);
}

bool QueueRequest(CUser* user,
                  LinkSessionState& state,
                  charac_link_db::Message& request) {
    if (state.pending_request_token != 0u ||
        !charac_link_db::DB_CharacLink::Queue(
            user->GetUID(), request)) {
        return false;
    }
    state.pending_request_token = request.request_token;
    state.pending_operation = request.operation;
    return true;
}

bool IsPendingReply(const LinkSessionState& state,
                    const charac_link_db::Message& response) {
    return state.pending_request_token != 0u &&
           state.pending_request_token == response.request_token &&
           state.pending_operation == response.operation;
}

void ClearPending(LinkSessionState& state) {
    state.pending_request_token = 0u;
    state.pending_operation = 0u;
}

void SendLinkSuccess(CUser* user, ENUM_CMDPACKET command) {
    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET, command);
    packet.put_byte(1u);
    packet.finalize(true);
    user->Send(packet);
}

bool IsLevelInRange(int level, int minimum, int maximum) {
    return level >= minimum && level <= maximum;
}

}  // namespace


// IDA_EA: 0x82300E8
// IDA_NAME: _ZN5CUser16get_charac_countEv
// DEMANGLED: CUser::get_charac_count(void)
int CUser::get_charac_count() {
    return static_cast<int>(characters_.size());
}

// IDA_EA: 0x84ECD52
// IDA_NAME: _ZN5CUser24isCharacLinkBonusSettingEc
// DEMANGLED: CUser::isCharacLinkBonusSetting(char)
int CUser::isCharacLinkBonusSetting(char slot) {
    const int slot_index = static_cast<signed char>(slot);
    if (slot_index >= get_charac_count() || slot_index < 0) {
        return 0;
    }

    return characters_[static_cast<std::size_t>(slot_index)]
        .fields_.link_state_124f_;
}

// IDA_EA: 0x84ECDB6
// IDA_NAME: _ZN5CUser25setCharacLinkBonusSettingEib
// DEMANGLED: CUser::setCharacLinkBonusSetting(int,bool)
// IDA return type is CCharacterView*; the invalid branch returns eax=1.
CCharacterView* CUser::setCharacLinkBonusSetting(int slot, bool enabled) {
    const int invalid = (get_charac_count() <= slot || slot < 0);
    if (!static_cast<unsigned char>(invalid)) {
        characters_[static_cast<std::size_t>(slot)]
            .fields_.link_state_124f_ = enabled ? 1u : 0u;
        return character_view_.enableSaveCharacView();
    }
    return reinterpret_cast<CCharacterView*>(invalid);
}

// IDA_EA: 0x8652B06
// IDA_NAME: _ZN5CUser27decideLinkCharacSlotIndexesEci
// DEMANGLED: CUser::decideLinkCharacSlotIndexes(char,int)
int CUser::decideLinkCharacSlotIndexes(char slot, int charac_count) {
    const int link_type =
        static_cast<signed char>(getCharacLinkType());
    const int slot_index = static_cast<signed char>(slot);
    if (slot_index < charac_count && slot_index >= 0) {
        if (link_type == 2) {
            return setLinkCharacSlotIndex(static_cast<char>(slot_index + 1));
        }
        if (link_type == 3) {
            return setLinkCharacSlotIndex(static_cast<char>(slot_index - 1));
        }
        return setLinkCharacSlotIndex(-1);
    }
    return slot_index;
}

// IDA_EA: 0x8652B90
// IDA_NAME: _ZN5CUser19checkCharacLinkTypeEci
// DEMANGLED: CUser::checkCharacLinkType(char,int)
int CUser::checkCharacLinkType(char slot, int charac_count) {
    const int link_type =
        static_cast<signed char>(getCharacLinkType());
    const int slot_index = static_cast<signed char>(slot);
    if (slot_index >= charac_count || slot_index < 0) {
        return 0;
    }

    if (link_type == 2) {
        if (slot_index + 1 >= charac_count) {
            return 0;
        }
        if ((slot_index & 1) != 0) {
            return 0;
        }
        if (static_cast<unsigned char>(
                characters_[static_cast<std::size_t>(slot_index + 1)]
                    .fields_.link_type_) !=
            3u) {
            return 0;
        }
    } else if (link_type == 3) {
        if (slot_index - 1 < 0) {
            return 0;
        }
        if ((slot_index & 1) == 0) {
            return 0;
        }
        if (static_cast<unsigned char>(
                characters_[static_cast<std::size_t>(slot_index - 1)]
                    .fields_.link_type_) !=
            2u) {
            return 0;
        }
    }
    return 1;
}

bool CUser::ConnectLinkCharacters(uint8_t first_slot,
                                  uint8_t second_slot,
                                  uint8_t first_grow_type,
                                  uint8_t second_grow_type) {
    const std::size_t first_index =
        static_cast<std::size_t>(first_slot);
    const std::size_t second_index =
        static_cast<std::size_t>(second_slot);
    if (first_index >= characters_.size() ||
        second_index >= characters_.size() ||
        first_index == second_index) {
        return false;
    }

    _Charac_info& first = characters_[first_index];
    _Charac_info& second = characters_[second_index];
    if (!IsLevelInRange(first.fields_.level_,
                        kFirstLinkMinimumLevel,
                        kFirstLinkMaximumLevel) ||
        !IsLevelInRange(second.fields_.level_,
                        kSecondLinkMinimumLevel,
                        kSecondLinkMaximumLevel) ||
        static_cast<uint8_t>(first.fields_.grow_type_.growth_type) !=
            first_grow_type ||
        static_cast<uint8_t>(second.fields_.grow_type_.growth_type) !=
            second_grow_type) {
        return false;
    }

    const uint32_t first_charac_no =
        static_cast<uint32_t>(first.fields_.character_no_);
    const uint32_t second_charac_no =
        static_cast<uint32_t>(second.fields_.character_no_);
    if (!first_charac_no || !second_charac_no) {
        return false;
    }

    LinkSessionState* const state = FindCurrentSession(this);
    if (!state || state->pending_request_token != 0u) {
        return false;
    }

    charac_link_db::Message request;
    InitializeRequest(this,
                      *state,
                      charac_link_db::kOperationConnect,
                      request);
    request.first_charac_no = first_charac_no;
    request.second_charac_no = second_charac_no;
    request.first_slot = first_slot;
    request.second_slot = second_slot;
    request.first_grow_type = first_grow_type;
    request.second_grow_type = second_grow_type;
    return QueueRequest(this, *state, request);
}

bool CUser::DisconnectLinkCharacters(uint8_t first_slot,
                                     uint8_t second_slot) {
    const std::size_t first_index =
        static_cast<std::size_t>(first_slot);
    const std::size_t second_index =
        static_cast<std::size_t>(second_slot);
    if (first_index >= characters_.size() ||
        second_index >= characters_.size() ||
        first_index == second_index) {
        return false;
    }

    _Charac_info& first = characters_[first_index];
    _Charac_info& second = characters_[second_index];
    const uint32_t first_charac_no =
        static_cast<uint32_t>(first.fields_.character_no_);
    const uint32_t second_charac_no =
        static_cast<uint32_t>(second.fields_.character_no_);
    if (!first_charac_no || !second_charac_no) {
        return false;
    }

    LinkSessionState* const state = FindCurrentSession(this);
    if (!state || state->pending_request_token != 0u) {
        return false;
    }

    charac_link_db::Message request;
    InitializeRequest(this,
                      *state,
                      charac_link_db::kOperationDisconnect,
                      request);
    request.first_charac_no = first_charac_no;
    request.second_charac_no = second_charac_no;
    request.first_slot = first_slot;
    request.second_slot = second_slot;
    return QueueRequest(this, *state, request);
}

void CUser::RestoreLinkCharactersFromDatabase() {
    for (std::vector<_Charac_info>::iterator character = characters_.begin();
         character != characters_.end();
         ++character) {
        character->fields_.link_charac_slot_index_ = 0xFFu;
        character->fields_.link_type_ = 0u;
    }

    if (GetUID() == -1 || get_acc_id() <= 0) {
        return;
    }

    LinkSessionState state;
    state.user_idx = GetUID();
    state.account_id = static_cast<uint32_t>(get_acc_id());
    state.server_group = GetServerGroup();
    state.session_token = NextSessionToken();
    LinkSessions()[this] = state;

    LinkSessionState* const current = FindCurrentSession(this);
    if (!current) {
        return;
    }
    charac_link_db::Message request;
    InitializeRequest(this,
                      *current,
                      charac_link_db::kOperationLoad,
                      request);
    QueueRequest(this, *current, request);
}

void CUser::ApplyCharacLinkDatabaseResult(
    const charac_link_db::Message& response) {
    LinkSessionState* const state = FindCurrentSession(this);
    if (!state || response.user_idx != GetUID() ||
        response.account_id != static_cast<uint32_t>(get_acc_id()) ||
        response.server_group != GetServerGroup() ||
        response.session_token != state->session_token ||
        !IsPendingReply(*state, response) || response.reserved != 0u ||
        (response.result != charac_link_db::kResultOk &&
         response.result != charac_link_db::kResultFailed)) {
        return;
    }

    const uint8_t operation = response.operation;
    const int user_state = get_state();
    if ((operation == charac_link_db::kOperationLoad &&
         user_state != 2 && user_state != 3) ||
        (operation != charac_link_db::kOperationLoad &&
         user_state != 2)) {
        return;
    }
    ClearPending(*state);

    if (operation == charac_link_db::kOperationLoad) {
        if (response.result != charac_link_db::kResultOk ||
            response.record_count >
                charac_link_db::kMaximumLinkRecords) {
            return;
        }

        std::map<uint32_t, uint32_t> links;
        for (unsigned int index = 0; index < response.record_count; ++index) {
            const charac_link_db::LinkRecord& record =
                response.records[index];
            if (record.charac_no == 0u || record.link_charac_no == 0u ||
                record.charac_no == record.link_charac_no ||
                links.find(record.charac_no) != links.end()) {
                return;
            }
            links.insert(std::make_pair(record.charac_no,
                                        record.link_charac_no));
        }

        // CMD309's two CMD292 moves persist the pair as adjacent even/odd
        // rows.  Restore only a mutual pair; corrupt/half-written MyISAM rows
        // remain disabled rather than manufacturing a partner.
        for (std::size_t first_index = 0;
             first_index + 1u < characters_.size();
             first_index += 2u) {
            const std::size_t second_index = first_index + 1u;
            const uint32_t first_charac_no = static_cast<uint32_t>(
                characters_[first_index].fields_.character_no_);
            const uint32_t second_charac_no = static_cast<uint32_t>(
                characters_[second_index].fields_.character_no_);
            std::map<uint32_t, uint32_t>::const_iterator first_link =
                links.find(first_charac_no);
            std::map<uint32_t, uint32_t>::const_iterator second_link =
                links.find(second_charac_no);
            if (first_link == links.end() || second_link == links.end() ||
                first_link->second != second_charac_no ||
                second_link->second != first_charac_no) {
                continue;
            }
            characters_[first_index].fields_.link_charac_slot_index_ =
                static_cast<uint8_t>(second_index);
            characters_[first_index].fields_.link_type_ = 2u;
            characters_[second_index].fields_.link_charac_slot_index_ =
                static_cast<uint8_t>(first_index);
            characters_[second_index].fields_.link_type_ = 3u;
        }
        return;
    }

    const ENUM_CMDPACKET command =
        operation == charac_link_db::kOperationConnect
            ? ENUM_CMDPACKET_CONNECT_LINK_CHARAC
            : ENUM_CMDPACKET_DISCONNECT_LINK_CHARAC;
    const std::size_t first_index = response.first_slot;
    const std::size_t second_index = response.second_slot;
    const bool current_slots =
        first_index < characters_.size() &&
        second_index < characters_.size() && first_index != second_index &&
        static_cast<uint32_t>(
            characters_[first_index].fields_.character_no_) ==
            response.first_charac_no &&
        static_cast<uint32_t>(
            characters_[second_index].fields_.character_no_) ==
            response.second_charac_no;
    if (response.result != charac_link_db::kResultOk ||
        !current_slots ||
        (operation != charac_link_db::kOperationConnect &&
         operation != charac_link_db::kOperationDisconnect)) {
        SendCmdErrorPacket(command, 1u);
        return;
    }

    _Charac_info& first = characters_[first_index];
    _Charac_info& second = characters_[second_index];
    if (operation == charac_link_db::kOperationConnect) {
        if (static_cast<uint8_t>(first.fields_.grow_type_.growth_type) !=
                response.first_grow_type ||
            static_cast<uint8_t>(second.fields_.grow_type_.growth_type) !=
                response.second_grow_type ||
            !IsLevelInRange(first.fields_.level_,
                            kFirstLinkMinimumLevel,
                            kFirstLinkMaximumLevel) ||
            !IsLevelInRange(second.fields_.level_,
                            kSecondLinkMinimumLevel,
                            kSecondLinkMaximumLevel)) {
            SendCmdErrorPacket(command, 1u);
            return;
        }

        // CLIENT_IDA_EVIDENCE: CMD309 success sub_138D070 marks the first
        // selected character as {partner=second,type=2} and the second as
        // {partner=first,type=3}; the client then emits the two CMD292 moves.
        first.fields_.link_charac_slot_index_ = response.second_slot;
        first.fields_.link_type_ = 2u;
        second.fields_.link_charac_slot_index_ = response.first_slot;
        second.fields_.link_type_ = 3u;
    } else {
        first.fields_.link_charac_slot_index_ = 0xFFu;
        first.fields_.link_type_ = 0u;
        second.fields_.link_charac_slot_index_ = 0xFFu;
        second.fields_.link_type_ = 0u;
    }
    SendLinkSuccess(this, command);
}

// IDA_EA: 0x86528B4
// IDA_NAME: _ZN5CUser25saveCharacLinkMessageFlagEb
// DEMANGLED: CUser::saveCharacLinkMessageFlag(bool)
void CUser::saveCharacLinkMessageFlag(bool enabled) {
    setCharacLinkMessageFlag(enabled);

    CStreamGuard guard(
        GlobalData::s_stream_pool->Acquire("user.cpp", 7270), true);
    *guard << 484;
    *guard << -1;
    *guard << get_acc_id();
    *guard << enabled;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

// IDA_EA: 0x8652A58
// IDA_NAME: _ZN5CUser23decideCharacLinkMessageEv
// DEMANGLED: CUser::decideCharacLinkMessage(void)
void CUser::decideCharacLinkMessage() {
    if (isCharacLinkMessageFlag()) {
        return;
    }

    for (std::vector<_Charac_info>::const_iterator character =
             characters_.begin();
         character != characters_.end();
         ++character) {
        if (character->fields_.level_ > 59 &&
            !isCharacLinkMessageFlag()) {
            sendCharacLinkMessage();
            saveCharacLinkMessageFlag(true);
            return;
        }
    }
}
