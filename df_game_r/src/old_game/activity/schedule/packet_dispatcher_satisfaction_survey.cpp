#include "old_game/activity/schedule/packet_dispatcher_satisfaction_survey.h"

#include "old_data/PVF/common/datamanager/dnf_lex.h"
#include "old_data/PVF/common/datamanager/rdar_script.h"
#include "old_data/db/event/db_satisfaction_survey.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

namespace {

const char kSurveyListPath[] =
    "ClientOnly/SatisfactionSurvey/SatisfactionSurvey.lst";

class RdarUnloadGuard {
public:
    RdarUnloadGuard() {}
    ~RdarUnloadGuard() { unloadRDARScriptFile(); }

private:
    RdarUnloadGuard(const RdarUnloadGuard&);
    RdarUnloadGuard& operator=(const RdarUnloadGuard&);
};

struct SurveyDefinition {
    SurveyDefinition() : comment_character_limit(0), answer_limits() {}

    int comment_character_limit;
    std::vector<int> answer_limits;
};

typedef std::vector<std::pair<uint32_t, std::string> > SurveyFileList;
typedef std::map<uint32_t, SurveyDefinition> SurveyDefinitionMap;

bool ParseSurveyFileList(SurveyFileList& files) {
    uint32_t pending_id = 0u;
    bool need_filename = false;
    for (;;) {
        int integer_value = 0;
        float float_value = 0.0f;
        std::string text;
        const DNFLEX_TOKEN token =
            ScanAll(&integer_value, &float_value, &text);
        if (token == DNF_TOK_EOF) {
            break;
        }
        if (token == DNF_TOK_DECIMAL || token == DNF_TOK_HEXADECIMAL) {
            if (need_filename || integer_value <= 0) {
                return false;
            }
            pending_id = static_cast<uint32_t>(integer_value);
            need_filename = true;
            continue;
        }
        if (token == DNF_TOK_STRING) {
            if (!need_filename || text.empty()) {
                return false;
            }
            files.push_back(std::make_pair(pending_id, text));
            need_filename = false;
            continue;
        }
        return false;
    }
    return !need_filename && !files.empty();
}

bool ParseSurveyDefinition(SurveyDefinition& definition) {
    int declared_question_count = 0;
    int comment_limit = 0;
    int current_question = -1;
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
            if (text == "[question]") {
                if (definition.answer_limits.size() >=
                    satisfaction_survey::kMaximumQuestionCount) {
                    return false;
                }
                definition.answer_limits.push_back(0);
                current_question = static_cast<int>(
                    definition.answer_limits.size() - 1u);
            } else if (text == "[/question]") {
                current_question = -1;
            }
            continue;
        }
        if (token != DNF_TOK_DECIMAL && token != DNF_TOK_HEXADECIMAL) {
            continue;
        }

        if (current_tag == "[question count]") {
            if (declared_question_count != 0 || integer_value <= 0 ||
                integer_value >
                    satisfaction_survey::kMaximumQuestionCount) {
                return false;
            }
            declared_question_count = integer_value;
            current_tag.clear();
        } else if (current_tag == "[example number]") {
            if (current_question < 0 || integer_value <= 0 ||
                integer_value > 255 ||
                definition.answer_limits[
                    static_cast<size_t>(current_question)] != 0) {
                return false;
            }
            definition.answer_limits[static_cast<size_t>(current_question)] =
                integer_value;
            current_tag.clear();
        } else if (current_tag == "[comment limit length]") {
            if (comment_limit != 0 || integer_value <= 0 ||
                integer_value > 120) {
                return false;
            }
            comment_limit = integer_value;
            current_tag.clear();
        }
    }

    if (declared_question_count <= 0 || comment_limit <= 0 ||
        definition.answer_limits.size() !=
            static_cast<size_t>(declared_question_count)) {
        return false;
    }
    for (std::vector<int>::const_iterator limit =
             definition.answer_limits.begin();
         limit != definition.answer_limits.end();
         ++limit) {
        if (*limit <= 0) {
            return false;
        }
    }
    definition.comment_character_limit = comment_limit;
    return true;
}

class SurveyDefinitionStore {
public:
    SurveyDefinitionStore() : attempted_(false), definitions_() {}

    const SurveyDefinition* Find(uint32_t survey_id) {
        if (!Load()) {
            return 0;
        }
        const SurveyDefinitionMap::const_iterator found =
            definitions_.find(survey_id);
        return found == definitions_.end() ? 0 : &found->second;
    }

private:
    bool Load() {
        if (attempted_) {
            return !definitions_.empty();
        }
        attempted_ = true;

        SurveyFileList files;
        if (loadRDARScriptFile("", kSurveyListPath) != 1) {
            return false;
        }
        {
            RdarUnloadGuard unload;
            if (!ParseSurveyFileList(files)) {
                return false;
            }
        }

        SurveyDefinitionMap parsed;
        for (SurveyFileList::const_iterator file = files.begin();
             file != files.end();
             ++file) {
            if (parsed.find(file->first) != parsed.end() ||
                loadRDARScriptFile("", file->second.c_str()) != 1) {
                return false;
            }
            SurveyDefinition definition;
            {
                RdarUnloadGuard unload;
                if (!ParseSurveyDefinition(definition)) {
                    return false;
                }
            }
            parsed.insert(std::make_pair(file->first, definition));
        }
        definitions_.swap(parsed);
        return !definitions_.empty();
    }

    bool attempted_;
    SurveyDefinitionMap definitions_;
};

SurveyDefinitionStore& SurveyDefinitions() {
    static SurveyDefinitionStore definitions;
    return definitions;
}

bool HasEmbeddedNul(const char* text, unsigned int size) {
    return size != 0u && std::memchr(text, 0, size) != 0;
}

bool IsValidUtf8WithinCharacterLimit(const char* text,
                                     unsigned int size,
                                     unsigned int character_limit) {
    if ((!text && size != 0u) || character_limit == 0u) {
        return false;
    }

    unsigned int offset = 0u;
    unsigned int characters = 0u;
    while (offset < size) {
        const unsigned int first =
            static_cast<unsigned char>(text[offset]);
        unsigned int sequence_size = 0u;
        unsigned int code_point = 0u;
        if (first <= 0x7Fu) {
            sequence_size = 1u;
            code_point = first;
        } else if (first >= 0xC2u && first <= 0xDFu) {
            sequence_size = 2u;
            code_point = first & 0x1Fu;
        } else if (first >= 0xE0u && first <= 0xEFu) {
            sequence_size = 3u;
            code_point = first & 0x0Fu;
        } else {
            // MySQL 5.5's utf8 column accepts at most three-byte UTF-8.
            return false;
        }
        if (offset + sequence_size > size) {
            return false;
        }
        for (unsigned int index = 1u; index < sequence_size; ++index) {
            const unsigned int continuation =
                static_cast<unsigned char>(text[offset + index]);
            if ((continuation & 0xC0u) != 0x80u) {
                return false;
            }
            code_point = (code_point << 6u) | (continuation & 0x3Fu);
        }
        if ((sequence_size == 2u && code_point < 0x80u) ||
            (sequence_size == 3u && code_point < 0x800u) ||
            (code_point >= 0xD800u && code_point <= 0xDFFFu)) {
            return false;
        }
        ++characters;
        if (characters > character_limit) {
            return false;
        }
        offset += sequence_size;
    }
    return true;
}

}  // namespace

Dispatcher_SatisfactionSurvey::Dispatcher_SatisfactionSurvey()
    : IPacketDispatcher<MSG_BASE,
                        ParamBase,
                        static_cast<ch_state>(0)>() {}

Dispatcher_SatisfactionSurvey::~Dispatcher_SatisfactionSurvey() {}

int Dispatcher_SatisfactionSurvey::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (!user || user->get_state() <= 1 || user->get_acc_id() <= 0) {
        return 0;
    }

    uint32_t survey_id = 0u;
    uint32_t question_count = 0u;
    if (packet.get_int(survey_id) != 1 ||
        packet.get_int(question_count) != 1) {
        return 0;
    }
    const SurveyDefinition* const definition =
        SurveyDefinitions().Find(survey_id);
    if (!definition || question_count == 0u ||
        question_count != definition->answer_limits.size() ||
        question_count >
            satisfaction_survey::kMaximumQuestionCount) {
        return 0;
    }

    satisfaction_survey::Request request;
    std::memset(&request, 0, sizeof(request));
    request.account_id = static_cast<uint32_t>(user->get_acc_id());
    request.charac_no = user->getCurCharacR()
                            ? static_cast<uint32_t>(user->getCurCharacNo())
                            : 0u;
    request.survey_id = survey_id;
    request.server_group = user->GetServerGroup();
    request.question_count = static_cast<uint8_t>(question_count);

    for (uint32_t index = 0u; index < question_count; ++index) {
        uint32_t answer = 0u;
        if (packet.get_int(answer) != 1 || answer == 0u ||
            answer > static_cast<uint32_t>(
                         definition->answer_limits[static_cast<size_t>(
                             index)])) {
            return 0;
        }
        request.answers[index] = answer;
    }

    uint32_t comment_size = 0u;
    const uint32_t maximum_comment_bytes = static_cast<uint32_t>(
        std::min<int>(definition->comment_character_limit * 3,
                      satisfaction_survey::kMaximumCommentBytes));
    if (packet.get_int(comment_size) != 1 ||
        comment_size > maximum_comment_bytes) {
        return 0;
    }
    if (comment_size != 0u &&
        packet.get_binary(request.comment,
                          static_cast<int>(comment_size)) != 1) {
        return 0;
    }
    if (HasEmbeddedNul(request.comment, comment_size) ||
        !IsValidUtf8WithinCharacterLimit(
            request.comment,
            comment_size,
            static_cast<unsigned int>(
                definition->comment_character_limit))) {
        return 0;
    }
    request.comment_size = static_cast<uint16_t>(comment_size);
    request.comment[comment_size] = 0;

    // CLIENT_IDA_EVIDENCE: sub_663AD0 begins CMD563 at 0x663B0C and writes
    // u32 survey_id, u32 count, count*u32 one-based answers, then one
    // u32-length-prefixed UTF-8 comment. The client registers no CMD success
    // handler, so native handling is persistence-only and sends no reply.
    (void)satisfaction_survey::Queue(user->GetUID(), request);
    return 0;
}

int Dispatcher_SatisfactionSurvey::read(PacketBuf& packet,
                                            MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

int Dispatcher_SatisfactionSurvey::process(CUser* user,
                                               MSG_BASE& message,
                                               ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
