#include "old_game/dungeon/progress/dragon_timeattack_rank_runtime.h"

#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"
#include "old_data/db/event/db_dragon_timeattack_rank.h"
#include "old_game/player/core/cuser.h"
#include "old_game/social/party/party.h"

#include <cstring>
#include <stdint.h>

namespace {

const int kDragonTimeAttackDungeon = 3539;
const int kDragonTimeAttackDifficulty = 2;
const unsigned char kQueryAction = 1u;
const unsigned char kOperationFailed = 1u;

bool MakeIdentity(CUser* user,
                  dragon_timeattack_rank::Payload& payload) {
    using namespace dragon_timeattack_rank;
    ClearPayload(payload);
    const int charac_no = user ? user->get_charac_no(-1) : 0;
    if (!user || !user->getCurCharacR() || charac_no <= 0) {
        return false;
    }
    payload.request_charac_no = static_cast<uint32_t>(charac_no);
    payload.server_group = user->GetServerGroup();
    return true;
}

bool FillScore(CUser* user,
               uint32_t clear_time_ms,
               dragon_timeattack_rank::Payload& payload) {
    using namespace dragon_timeattack_rank;
    if (!MakeIdentity(user, payload) || clear_time_ms == 0u) {
        return false;
    }
    const char* const character_name = user->getCurCharacName();
    if (!character_name) {
        return false;
    }
    const size_t name_length = std::strlen(character_name);
    const int level = user->get_charac_level();
    const int job = user->get_charac_job();
    const int grow = user->getCurCharFirstGrowType();
    const int second = user->getCurCharSecondGrowType();
    if (name_length == 0u || name_length > kMaximumCharacterNameBytes ||
        level < 0 || level > 255 || job < 0 ||
        job > kMaximumJobType ||
        grow < 0 || grow > 255 || second < 0 || second > 255) {
        return false;
    }

    payload.operation = OPERATION_UPDATE_BEST_TIME;
    payload.score_charac_no = payload.request_charac_no;
    payload.score_clear_time_ms = clear_time_ms;
    payload.score_job_type = static_cast<uint8_t>(job);
    payload.score_grow_type = static_cast<uint8_t>(grow);
    payload.score_second_grow_type = static_cast<uint8_t>(second);
    payload.score_charac_level = static_cast<uint8_t>(level);
    std::memcpy(payload.score_character_name,
                character_name,
                name_length);
    payload.score_character_name[name_length] = 0;
    return true;
}

void SendError(CUser* user) {
    if (user) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_DRAGON_DUNGEON_TIMEATTACK_RANKER_BOARD,
            kOperationFailed);
    }
}

void SendRanking(
    CUser* user,
    const dragon_timeattack_rank::Payload& result) {
    using namespace dragon_timeattack_rank;
    PacketGuard packet;
    packet.put_header(
        ENUM_PACKETCLASS_CMDPACKET,
        ENUM_CMDPACKET_DRAGON_DUNGEON_TIMEATTACK_RANKER_BOARD);
    // The CMD framework consumes this success byte and passes it to
    // sub_57A310 as a2; the handler tail starts at record_count only after
    // a2 is non-zero.
    packet.put_byte(1);
    packet.put_byte(result.record_count);
    for (unsigned int index = 0u; index < result.record_count; ++index) {
        const RankRecord& record = result.records[index];
        const size_t name_length = std::strlen(record.character_name);
        packet.put_int(static_cast<int>(name_length));
        packet.put_str(record.character_name,
                       static_cast<int>(name_length));
        packet.put_byte(record.job_type);
        packet.put_byte(record.grow_type);
        packet.put_byte(record.second_grow_type);
        packet.put_byte(record.charac_level);
        packet.put_int(static_cast<int>(record.clear_time_ms));
    }
    packet.put_int(
        static_cast<int>(result.personal_best_clear_time_ms));
    packet.put_int(static_cast<int>(result.personal_rank));
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

namespace dragon_timeattack_rank {

void RecordPartyClear(CParty* party,
                      int dungeon_index,
                      int dungeon_difficulty,
                      int clear_time_ms) {
    if (!party || dungeon_index != kDragonTimeAttackDungeon ||
        dungeon_difficulty != kDragonTimeAttackDifficulty ||
        clear_time_ms <= 0) {
        return;
    }

    for (int seat = 0; seat < 4; ++seat) {
        if (static_cast<unsigned char>(party->checkValidUser(seat)) != 1u) {
            continue;
        }
        CUser* const user = party->get_user(seat);
        Payload payload;
        if (FillScore(user,
                      static_cast<uint32_t>(clear_time_ms),
                      payload)) {
            (void)Queue(user->GetUID(), payload);
        }
    }
}

}  // namespace dragon_timeattack_rank_0828

Dispatcher_DragonDungeonTimeAttackRankerBoard::
    Dispatcher_DragonDungeonTimeAttackRankerBoard()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_DragonDungeonTimeAttackRankerBoard::~
    Dispatcher_DragonDungeonTimeAttackRankerBoard() {}

int Dispatcher_DragonDungeonTimeAttackRankerBoard::dispatch_sig(
    CUser* user,
    PacketBuf& packet) {
    unsigned char action = 0u;
    unsigned char job_type = 0u;
    if (!user || user->get_state() <= 2 || !user->getCurCharacR() ||
        packet.get_byte(action) != 1 ||
        packet.get_byte(job_type) != 1 || action != kQueryAction) {
        SendError(user);
        return 0;
    }

    if (job_type > dragon_timeattack_rank::kMaximumJobType) {
        SendError(user);
        return 0;
    }

    dragon_timeattack_rank::Payload payload;
    if (!MakeIdentity(user, payload)) {
        SendError(user);
        return 0;
    }
    payload.operation =
        dragon_timeattack_rank::OPERATION_QUERY_RANKING;
    payload.requested_job_type = job_type;
    if (!dragon_timeattack_rank::Queue(user->GetUID(), payload)) {
        SendError(user);
    }
    return 0;
}

int Dispatcher_DragonDungeonTimeAttackRankerBoard::read(
    PacketBuf& packet,
    MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_DragonDungeonTimeAttackRankerBoard::process(
    CUser* user,
    MSG_BASE& message,
    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_DragonTimeAttackRank::Inter_DragonTimeAttackRank()
    : ISigDispatcher() {}

Inter_DragonTimeAttackRank::~Inter_DragonTimeAttackRank() {}

int Inter_DragonTimeAttackRank::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    using namespace dragon_timeattack_rank;
    if (!user || user->get_state() <= 2 || !user->getCurCharacR() ||
        !payload || payload_size != static_cast<int>(sizeof(Payload))) {
        return 0;
    }
    const Payload* const result =
        reinterpret_cast<const Payload*>(payload);
    const int charac_no = user->get_charac_no(-1);
    if (charac_no <= 0 ||
        result->request_charac_no != static_cast<uint32_t>(charac_no) ||
        result->server_group != user->GetServerGroup() ||
        result->operation != OPERATION_QUERY_RANKING ||
        result->requested_job_type > kMaximumJobType ||
        result->result != 0u ||
        result->record_count > kMaximumRecordCount) {
        SendError(user);
        return 0;
    }
    for (unsigned int index = 0u; index < result->record_count; ++index) {
        const RankRecord& record = result->records[index];
        if (record.clear_time_ms == 0u ||
            record.job_type > kMaximumJobType ||
            record.job_type != result->requested_job_type ||
            record.character_name[0] == 0 ||
            record.character_name[kMaximumCharacterNameBytes] != 0) {
            SendError(user);
            return 0;
        }
    }

    // CLIENT_IDA_EVIDENCE: CMD696 sub_57A310 consumes u8 count, repeated
    // {u32 byte_len,bytes,u8 job,u8 grow,u8 second,u8 level,u32 time}, then
    // the requesting character's {u32 best_time,u32 rank}.
    SendRanking(user, *result);
    return 0;
}
