#include "old_data/PVF/pvp_mission/pvp_reference_tables.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "common/manager/global_tools.h"
#include "old_game/pvp/battlefield/pvp_reference_public_abi.h"

#include <algorithm>
#include <string>
#include <vector>

// IDA_EA: 0x837F544
// IDA_NAME: _ZN17stSpPerLevelTableC2Ev
// DEMANGLED: stSpPerLevelTable::stSpPerLevelTable(void)
stSpPerLevelTable::stSpPerLevelTable() {}

// IDA_EA: 0x837F558
// IDA_NAME: _ZN17stSpPerLevelTableD2Ev
// DEMANGLED: stSpPerLevelTable::~stSpPerLevelTable()
stSpPerLevelTable::~stSpPerLevelTable() {}

namespace {

struct PvpReferenceToken {
    PvpReferenceToken(DNFLEX_TOKEN token_type,
                      int integer,
                      float real,
                      const std::string& token_text)
        : type(token_type),
          int_value(integer),
          float_value(real),
          text(token_text) {}

    DNFLEX_TOKEN type;
    int int_value;
    float float_value;
    std::string text;
};

enum PvpReferenceField {
    kSeenExperienceMaxGrade = 1u << 0,
    kSeenExperience = 1u << 1,
    kSeenExperienceLoss = 1u << 2,
    kSeenRankRateGrade = 1u << 3,
    kSeenRankTotalCount = 1u << 4,
    kSeenRankRate = 1u << 5,
    kSeenNormalBattleTime = 1u << 6,
    kSeenDeathmatchBattleTime = 1u << 7,
    kSeenRelayBattleTime = 1u << 8,
    kSeenNormalTournamentTime = 1u << 9,
    kSeenDeathmatchTournamentTime = 1u << 10,
    kSeenRelayTournamentTime = 1u << 11,
    kSeenDirectWait = 1u << 12,
    kSeenTimeout = 1u << 13,
    kSeenRematch = 1u << 14,
    kSeenRewardItemIndex = 1u << 15,
    kSeenRewardItemCount = 1u << 16,
    kSeenRewardItemLimitPerDay = 1u << 17,
    kSeenRecordOrder = 1u << 18,
    kSeenTeamHpCorrection = 1u << 19,
};

const unsigned int kRequiredPvpReferenceMask =
    kSeenExperienceMaxGrade |
    kSeenExperience |
    kSeenExperienceLoss |
    kSeenRankRateGrade |
    kSeenRankTotalCount |
    kSeenRankRate |
    kSeenNormalBattleTime |
    kSeenDeathmatchBattleTime |
    kSeenRelayBattleTime |
    kSeenNormalTournamentTime |
    kSeenDeathmatchTournamentTime |
    kSeenRelayTournamentTime |
    kSeenDirectWait |
    kSeenTimeout |
    kSeenRematch |
    kSeenRewardItemIndex |
    kSeenRewardItemCount |
    kSeenRewardItemLimitPerDay |
    kSeenRecordOrder;

enum PvpReferenceImportResult {
    kPvpReferenceLoadError = 6166,
    kPvpReferenceSyntaxError = 6175,
    kPvpReferenceDuplicateField = 6176,
    kPvpReferenceMissingField = 6177,
    kPvpReferenceInvalidValue = 6178,
    kPvpReferenceUnknownField = 6179,
};

bool IsPvpIntegerToken(DNFLEX_TOKEN token) {
    return token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL;
}

bool MarkPvpReferenceField(unsigned int flag, unsigned int* seen) {
    if (!seen || (*seen & flag) != 0u) {
        return false;
    }
    *seen |= flag;
    return true;
}

bool ReadSinglePositivePvpInt(const std::vector<PvpReferenceToken>& tokens,
                              size_t* cursor,
                              int* output) {
    if (!cursor || !output || *cursor >= tokens.size() ||
        !IsPvpIntegerToken(tokens[*cursor].type)) {
        return false;
    }
    const int value = tokens[*cursor].int_value;
    ++*cursor;
    if (value <= 0 ||
        (*cursor < tokens.size() &&
         tokens[*cursor].type != DNF_TOK_TYPE)) {
        return false;
    }
    *output = value;
    return true;
}

bool ReadSinglePositivePvpNumber(
    const std::vector<PvpReferenceToken>& tokens,
    size_t* cursor) {
    if (!cursor || *cursor >= tokens.size()) {
        return false;
    }
    const PvpReferenceToken& token = tokens[*cursor];
    float value = 0.0f;
    if (IsPvpIntegerToken(token.type)) {
        value = static_cast<float>(token.int_value);
    } else if (token.type == DNF_TOK_FLOAT) {
        value = token.float_value;
    } else {
        return false;
    }
    ++*cursor;
    return value > 0.0f &&
           (*cursor >= tokens.size() ||
            tokens[*cursor].type == DNF_TOK_TYPE);
}

bool ReadPvpCloseTag(const std::vector<PvpReferenceToken>& tokens,
                     size_t* cursor,
                     const char* expected) {
    if (!cursor || !expected || *cursor >= tokens.size() ||
        tokens[*cursor].type != DNF_TOK_TYPE ||
        tokens[*cursor].text != expected) {
        return false;
    }
    ++*cursor;
    return true;
}

}  // namespace

// IDA_EA: 0x890857B
// IDA_NAME: _Z23importPvPReferenceTablePKcR15stPvpRefTable_t
// DEMANGLED: importPvPReferenceTable(char const*,stPvpRefTable_t &)
int importPvPReferenceTable(const char* filename, stPvpRefTable_t& table) {
    if (loadRDARScriptFile("", filename) != 1) {
        return kPvpReferenceLoadError;
    }

    std::vector<PvpReferenceToken> tokens;
    while (1) {
        int int_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&int_value, &float_value, &text);
        if (token == DNF_TOK_EOF) {
            break;
        }
        tokens.push_back(
            PvpReferenceToken(token, int_value, float_value, text));
    }

    stPvpRefTable_t staged;
    int staged_normal_battle_time = 0;
    int staged_deathmatch_battle_time = 0;
    int staged_relay_battle_time = 0;
    int staged_normal_tournament_time = 0;
    int staged_deathmatch_tournament_time = 0;
    int staged_relay_tournament_time = 0;
    unsigned int seen = 0u;
    size_t cursor = 0u;

    while (cursor < tokens.size()) {
        if (tokens[cursor].type != DNF_TOK_TYPE) {
            return kPvpReferenceSyntaxError;
        }
        const std::string tag = tokens[cursor].text;
        ++cursor;

        if (tag == "[pvp experience max grade]") {
            if (!MarkPvpReferenceField(kSeenExperienceMaxGrade, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            int max_grade = 0;
            if (!ReadSinglePositivePvpInt(tokens, &cursor, &max_grade) ||
                max_grade > 37) {
                return kPvpReferenceInvalidValue;
            }
            staged.max_grade_plus_one = max_grade + 1;
            staged.grade_exp_table.reserve(staged.max_grade_plus_one);
            staged.grade_exp_table.resize(
                staged.max_grade_plus_one,
                stPvpRefTable_t::stPvPGradeExp_t());
        } else if (tag == "[pvp experience]") {
            if (!MarkPvpReferenceField(kSeenExperience, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if ((seen & kSeenExperienceMaxGrade) == 0u) {
                return kPvpReferenceInvalidValue;
            }
            std::vector<char> grade_seen(staged.max_grade_plus_one, 0);
            int grade_count = 0;
            while (cursor < tokens.size() &&
                   tokens[cursor].type != DNF_TOK_TYPE) {
                if (cursor + 1u >= tokens.size() ||
                    !IsPvpIntegerToken(tokens[cursor].type) ||
                    !IsPvpIntegerToken(tokens[cursor + 1u].type)) {
                    return kPvpReferenceSyntaxError;
                }
                const int grade = tokens[cursor].int_value;
                const int exp = tokens[cursor + 1u].int_value;
                cursor += 2u;
                if (grade <= 0 || grade >= staged.max_grade_plus_one ||
                    exp <= 0 || grade_seen[grade]) {
                    return kPvpReferenceInvalidValue;
                }
                grade_seen[grade] = 1;
                ++grade_count;
                staged.grade_exp_table[grade] =
                    stPvpRefTable_t::stPvPGradeExp_t(
                        grade, static_cast<unsigned int>(exp));
            }
            if (!ReadPvpCloseTag(tokens, &cursor, "[/pvp experience]") ||
                grade_count != staged.max_grade_plus_one - 1) {
                return kPvpReferenceMissingField;
            }
        } else if (tag == "[pvp experience loss]") {
            if (!MarkPvpReferenceField(kSeenExperienceLoss, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (cursor + 1u >= tokens.size() ||
                !IsPvpIntegerToken(tokens[cursor].type) ||
                !IsPvpIntegerToken(tokens[cursor + 1u].type)) {
                return kPvpReferenceSyntaxError;
            }
            staged.pvp_exp_loss_grade = tokens[cursor].int_value;
            staged.pvp_exp_loss_exp = tokens[cursor + 1u].int_value;
            cursor += 2u;
            if (staged.pvp_exp_loss_grade <= 0 ||
                staged.pvp_exp_loss_exp <= 0 ||
                (cursor < tokens.size() &&
                 tokens[cursor].type != DNF_TOK_TYPE)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp rank rate grade]") {
            if (!MarkPvpReferenceField(kSeenRankRateGrade, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_rank_rate_grade)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp rank total cnt]") {
            if (!MarkPvpReferenceField(kSeenRankTotalCount, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_rank_total_count)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp rank rate]") {
            if (!MarkPvpReferenceField(kSeenRankRate, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            while (cursor < tokens.size() &&
                   tokens[cursor].type != DNF_TOK_TYPE) {
                if (!IsPvpIntegerToken(tokens[cursor].type) ||
                    tokens[cursor].int_value <= 0) {
                    return kPvpReferenceInvalidValue;
                }
                staged.pvp_rank_rate.push_back(tokens[cursor].int_value);
                ++cursor;
            }
            if (!ReadPvpCloseTag(tokens, &cursor, "[/pvp rank rate]") ||
                staged.pvp_rank_rate.size() != 5u) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp normal battle time]") {
            if (!MarkPvpReferenceField(kSeenNormalBattleTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_normal_battle_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp deathmatch battle time]") {
            if (!MarkPvpReferenceField(kSeenDeathmatchBattleTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_deathmatch_battle_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp relay battle time]") {
            if (!MarkPvpReferenceField(kSeenRelayBattleTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_relay_battle_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp normal battle time for tournament]") {
            if (!MarkPvpReferenceField(kSeenNormalTournamentTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_normal_tournament_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp deathmatch battle time for tournament]") {
            if (!MarkPvpReferenceField(
                    kSeenDeathmatchTournamentTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_deathmatch_tournament_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp relay battle time for tournament]") {
            if (!MarkPvpReferenceField(kSeenRelayTournamentTime, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged_relay_tournament_time)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp direct wait]") {
            if (!MarkPvpReferenceField(kSeenDirectWait, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_direct_wait)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp timeout]") {
            if (!MarkPvpReferenceField(kSeenTimeout, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_timeout)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp rematch]") {
            if (!MarkPvpReferenceField(kSeenRematch, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_rematch)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp reward item index]") {
            if (!MarkPvpReferenceField(kSeenRewardItemIndex, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_reward_item_index)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp reward item count]") {
            if (!MarkPvpReferenceField(kSeenRewardItemCount, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor, &staged.pvp_reward_item_count)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp reward item limit per day]") {
            if (!MarkPvpReferenceField(
                    kSeenRewardItemLimitPerDay, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpInt(
                    tokens, &cursor,
                    &staged.pvp_reward_item_limit_per_day)) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp record order]") {
            if (!MarkPvpReferenceField(kSeenRecordOrder, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            while (cursor < tokens.size() &&
                   tokens[cursor].type != DNF_TOK_TYPE) {
                if (tokens[cursor].type != DNF_TOK_STRING) {
                    return kPvpReferenceSyntaxError;
                }
                const int job =
                    ::getCharacterJobToEnumValue(tokens[cursor].text);
                if (job < 0 ||
                    std::find(staged.raw_28_values.begin(),
                              staged.raw_28_values.end(), job) !=
                        staged.raw_28_values.end()) {
                    return kPvpReferenceInvalidValue;
                }
                staged.raw_28_values.push_back(job);
                ++cursor;
            }
            if (!ReadPvpCloseTag(tokens, &cursor, "[/pvp record order]") ||
                staged.raw_28_values.empty()) {
                return kPvpReferenceInvalidValue;
            }
        } else if (tag == "[pvp team hp correction]") {
            if (!MarkPvpReferenceField(kSeenTeamHpCorrection, &seen)) {
                return kPvpReferenceDuplicateField;
            }
            if (!ReadSinglePositivePvpNumber(tokens, &cursor)) {
                return kPvpReferenceInvalidValue;
            }
        } else {
            return kPvpReferenceUnknownField;
        }
    }

    if ((seen & kRequiredPvpReferenceMask) !=
        kRequiredPvpReferenceMask) {
        return kPvpReferenceMissingField;
    }
    if (staged.pvp_exp_loss_grade >= staged.max_grade_plus_one ||
        staged.pvp_rank_rate_grade >= staged.max_grade_plus_one) {
        return kPvpReferenceInvalidValue;
    }
    unsigned int previous_exp = 0u;
    for (int grade = 1; grade < staged.max_grade_plus_one; ++grade) {
        if (staged.grade_exp_table[grade].grade != grade ||
            staged.grade_exp_table[grade].exp <= previous_exp) {
            return kPvpReferenceInvalidValue;
        }
        previous_exp = staged.grade_exp_table[grade].exp;
    }
    int rank_rate_total = 0;
    for (size_t index = 0u; index < staged.pvp_rank_rate.size(); ++index) {
        rank_rate_total += staged.pvp_rank_rate[index];
    }
    if (rank_rate_total > 1000) {
        return kPvpReferenceInvalidValue;
    }

    // Publish only after the whole official schema has been validated.  A
    // rejected reload therefore leaves both the typed owner and battle-time
    // globals untouched.
    table.swap(staged);
    PVP_NORMAL_BATTLE_TIME[0] = staged_normal_battle_time;
    PVP_DM_BATTLE_TIME[0] = staged_deathmatch_battle_time;
    PVP_RELAY_BATTLE_TIME = staged_relay_battle_time;
    dword_93F4EE4 = staged_normal_tournament_time;
    dword_93F4EEC = staged_deathmatch_tournament_time;
    dword_93F4EF4 = staged_relay_tournament_time;
    return 0;
}

// IDA_EA: 0x8910505
// IDA_NAME: _Z30importSpPerLevelReferenceTablePKcR17stSpPerLevelTable
// DEMANGLED: importSpPerLevelReferenceTable(char const*,stSpPerLevelTable &)
int importSpPerLevelReferenceTable(const char* filename,
                                   stSpPerLevelTable& table) {
    bool success = false;
    std::string tag;
    if (loadRDARScriptFile("", filename) != 1) {
        return 10568;
    }

    while (ScanType(tag, true) == 1) {
        if (tag == "[sp table]") {
            table.values.reserve(200);
            table.values.push_back(0);
            while (1) {
                ScanInt(&success);
                if (!success) {
                    break;
                }
                const unsigned int value =
                    static_cast<unsigned int>(ScanInt(&success));
                if (!success) {
                    break;
                }
                table.values.push_back(value);
            }
        }
    }
    return 0;
}
