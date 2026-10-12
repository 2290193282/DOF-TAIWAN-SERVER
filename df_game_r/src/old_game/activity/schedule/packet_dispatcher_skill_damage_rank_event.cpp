#include "old_game/activity/schedule/packet_dispatcher_skill_damage_rank_event.h"

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/event/db_skill_damage_rank_event.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>
#include <string>
#include <vector>

namespace {

const char kRankEventPath[] =
    "Event/Japan/SkillDamageRankEvent/SkillDamageRankEvent.evt";
const int kRankErrorOperationFailed = 1;

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct RankEventConfig {
    RankEventConfig()
        : loaded(false),
          job_type(0),
          second_grow_type(0),
          reward_term(0),
          reward_item_id(0),
          reward_item_count(0),
          grow_types() {}

    bool loaded;
    int job_type;
    int second_grow_type;
    int reward_term;
    int reward_item_id;
    int reward_item_count;
    std::vector<int> grow_types;
};

bool ParseRankEvent(RankEventConfig& config) {
    std::string current_tag;
    for (;;) {
        int integer_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&integer_value, &float_value, &text);
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_TYPE) {
            current_tag = text;
            continue;
        }
        if (token != DNF_TOK_DECIMAL && token != DNF_TOK_HEXADECIMAL) {
            continue;
        }

        if (current_tag == "[job type]") {
            if (config.job_type != 0 || integer_value <= 0 ||
                integer_value > 255) {
                return false;
            }
            config.job_type = integer_value;
        } else if (current_tag == "[grow type]") {
            if (integer_value <= 0 || integer_value > 255 ||
                config.grow_types.size() >=
                    skill_damage_rank_event::kMaximumGrowTypeCount ||
                std::find(config.grow_types.begin(),
                          config.grow_types.end(),
                          integer_value) != config.grow_types.end()) {
                return false;
            }
            config.grow_types.push_back(integer_value);
        } else if (current_tag == "[second grow type]") {
            if (config.second_grow_type != 0 || integer_value <= 0 ||
                integer_value > 255) {
                return false;
            }
            config.second_grow_type = integer_value;
        } else if (current_tag == "[reward term]") {
            if (config.reward_term != 0 || integer_value <= 0) {
                return false;
            }
            config.reward_term = integer_value;
            current_tag.clear();
        } else if (current_tag == "[reward item index]") {
            if (config.reward_item_id != 0 || integer_value <= 0) {
                return false;
            }
            config.reward_item_id = integer_value;
            current_tag.clear();
        } else if (current_tag == "[reward item count]") {
            if (config.reward_item_count != 0 || integer_value <= 0) {
                return false;
            }
            config.reward_item_count = integer_value;
            current_tag.clear();
        }
    }

    return config.job_type > 0 && config.second_grow_type > 0 &&
           config.reward_term > 0 && config.reward_item_id > 0 &&
           config.reward_item_count > 0 && !config.grow_types.empty();
}

const RankEventConfig* GetRankEventConfig() {
    static bool attempted = false;
    static RankEventConfig config;
    if (attempted) {
        return config.loaded ? &config : 0;
    }
    attempted = true;
    if (loadRDARScriptFile("", kRankEventPath) != 1) {
        return 0;
    }
    RdarUnloadGuard unload;
    config.loaded = ParseRankEvent(config);
    return config.loaded ? &config : 0;
}

bool IsEligibleCharacter(CUser* user, const RankEventConfig& config) {
    if (!user || !user->getCurCharacR() ||
        user->get_charac_job() != config.job_type ||
        static_cast<int>(user->getCurCharSecondGrowType()) !=
            config.second_grow_type) {
        return false;
    }
    return std::find(config.grow_types.begin(),
                     config.grow_types.end(),
                     user->getCurCharFirstGrowType()) !=
           config.grow_types.end();
}

bool MakePayload(CUser* user,
                 const RankEventConfig& config,
                 skill_damage_rank_event::Payload& payload) {
    using namespace skill_damage_rank_event;
    ClearPayload(payload);
    const int charac_no = user ? user->get_charac_no(-1) : 0;
    const char* const character_name =
        user && user->getCurCharacR() ? user->getCurCharacName() : 0;
    if (charac_no <= 0 || !character_name) {
        return false;
    }
    const size_t name_length = std::strlen(character_name);
    if (name_length == 0u || name_length > kMaximumCharacterNameBytes ||
        config.grow_types.size() > kMaximumGrowTypeCount) {
        return false;
    }

    payload.request_charac_no = static_cast<uint32_t>(charac_no);
    payload.server_group = user->GetServerGroup();
    payload.job_type = static_cast<uint8_t>(config.job_type);
    payload.grow_type =
        static_cast<uint8_t>(user->getCurCharFirstGrowType());
    payload.second_grow_type =
        static_cast<uint8_t>(config.second_grow_type);
    payload.grow_type_count =
        static_cast<uint8_t>(config.grow_types.size());
    for (size_t index = 0u; index < config.grow_types.size(); ++index) {
        payload.grow_types[index] =
            static_cast<uint8_t>(config.grow_types[index]);
    }
    std::memcpy(payload.character_name, character_name, name_length);
    payload.character_name[name_length] = 0;
    return true;
}

void SendRankingError(CUser* user) {
    if (user) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_BEST_DAMAGE_RANK,
            kRankErrorOperationFailed);
    }
}

void SendPersonalBest(CUser* user, uint32_t best_damage) {
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_SKILL_DAMAGE_RANK_EVENT);
    packet.put_int(static_cast<int>(best_damage));
    packet.finalize(true);
    user->Send(packet);
}

void SendRanking(CUser* user,
                 const skill_damage_rank_event::Payload& result) {
    using namespace skill_damage_rank_event;
    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_BEST_DAMAGE_RANK);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(result.record_count));
    for (unsigned int index = 0u; index < result.record_count; ++index) {
        const RankRecord& record = result.records[index];
        const size_t name_length = std::strlen(record.character_name);
        packet.put_byte(record.grow_type);
        packet.put_int(static_cast<int>(name_length));
        packet.put_str(record.character_name,
                       static_cast<int>(name_length));
        packet.put_int(static_cast<int>(record.best_damage));
    }
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace

Dispatcher_UpdateBestDamageRank::Dispatcher_UpdateBestDamageRank()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_UpdateBestDamageRank::~Dispatcher_UpdateBestDamageRank() {}

int Dispatcher_UpdateBestDamageRank::dispatch_sig(CUser* user,
                                                    PacketBuf& packet) {
    uint32_t best_damage = 0u;
    if (!user || user->get_state() != 5 ||
        packet.get_int(best_damage) != 1 || best_damage == 0u) {
        return 0;
    }
    const RankEventConfig* const config = GetRankEventConfig();
    if (!config || !IsEligibleCharacter(user, *config)) {
        return 0;
    }

    skill_damage_rank_event::Payload payload;
    if (!MakePayload(user, *config, payload)) {
        return 0;
    }
    payload.operation =
        skill_damage_rank_event::OPERATION_UPDATE_BEST_DAMAGE;
    payload.score_charac_no = payload.request_charac_no;
    payload.best_damage = best_damage;

    // CLIENT_IDA_EVIDENCE: sub_41C1B0 sends only a u32 when the current
    // dungeon's protected damage value exceeds the locally cached best. The
    // client has no CMD732 response handler, so this is a one-way MAX update.
    (void)skill_damage_rank_event::Queue(user->GetUID(), payload);
    return 0;
}

int Dispatcher_UpdateBestDamageRank::read(PacketBuf& packet,
                                           MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_UpdateBestDamageRank::process(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Dispatcher_RequestBestDamageRank::Dispatcher_RequestBestDamageRank()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_RequestBestDamageRank::~Dispatcher_RequestBestDamageRank() {}

int Dispatcher_RequestBestDamageRank::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) {
        return 0;
    }
    const RankEventConfig* const config = GetRankEventConfig();
    if (!config || !IsEligibleCharacter(user, *config)) {
        SendRankingError(user);
        return 0;
    }

    skill_damage_rank_event::Payload payload;
    if (!MakePayload(user, *config, payload)) {
        SendRankingError(user);
        return 0;
    }
    payload.operation =
        skill_damage_rank_event::OPERATION_QUERY_RANKING;
    if (!skill_damage_rank_event::Queue(user->GetUID(), payload)) {
        SendRankingError(user);
    }
    return 0;
}

int Dispatcher_RequestBestDamageRank::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_RequestBestDamageRank::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

Inter_SkillDamageRankEvent::Inter_SkillDamageRankEvent()
    : ISigDispatcher() {}

Inter_SkillDamageRankEvent::~Inter_SkillDamageRankEvent() {}

int Inter_SkillDamageRankEvent::dispatch_sig(CUser* user,
                                                  char* payload,
                                                  int payload_size) {
    using namespace skill_damage_rank_event;
    if (!user || user->get_state() <= 2 || !user->getCurCharacR() ||
        !payload || payload_size != static_cast<int>(sizeof(Payload))) {
        return 0;
    }
    const Payload* const result = reinterpret_cast<const Payload*>(payload);
    const int charac_no = user->get_charac_no(-1);
    if (result->operation != OPERATION_QUERY_RANKING || charac_no <= 0 ||
        result->request_charac_no != static_cast<uint32_t>(charac_no)) {
        return 0;
    }
    if (result->result != 0u ||
        result->record_count > kMaximumRecordCount) {
        SendRankingError(user);
        return 0;
    }
    for (unsigned int index = 0u; index < result->record_count; ++index) {
        const RankRecord& record = result->records[index];
        if (record.grow_type == 0u || record.best_damage == 0u ||
            record.character_name[0] == 0 ||
            record.character_name[kMaximumCharacterNameBytes] != 0) {
            SendRankingError(user);
            return 0;
        }
    }

    // CLIENT_IDA_EVIDENCE: NOTI708 sub_41CE60 consumes one u32 into the
    // protected personal-best field. CMD733 sub_41CF00 then consumes a u32
    // count and repeated {u8 grow_type,u32 byte_len,bytes,u32 damage}; the UI
    // renders exactly ten rows for each configured grow type.
    SendPersonalBest(user, result->best_damage);
    SendRanking(user, *result);
    return 0;
}
