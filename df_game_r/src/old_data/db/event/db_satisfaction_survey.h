#ifndef OLDSRV_OLD_DATA_DB_EVENT_DB_SATISFACTION_SURVEY_H_
#define OLDSRV_OLD_DATA_DB_EVENT_DB_SATISFACTION_SURVEY_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stdint.h>

namespace satisfaction_survey {

enum {
    // RECONSTRUCTED_0828: slot 843 is unused in both restored 852-entry
    // DB routing tables. It is internal only and is not a game packet ID.
    kDbSignal = 843,
    kMaximumQuestionCount = 16,
    // CLIENT_IDA_EVIDENCE: sub_14327C0 emits one, two or three UTF-8 bytes
    // for each UTF-16 code unit, so a 120-character PVF limit needs 360.
    kMaximumCommentBytes = 360,
};

#pragma pack(push, 1)
struct Request {
    uint32_t account_id;
    uint32_t charac_no;
    uint32_t survey_id;
    int32_t server_group;
    uint32_t answers[kMaximumQuestionCount];
    uint16_t comment_size;
    uint8_t question_count;
    char comment[kMaximumCommentBytes + 1];
};
#pragma pack(pop)

static_assert(sizeof(Request) == 444u,
              "CMD563 survey DB request must remain a fixed 444-byte ABI");

class DB_SaveSatisfactionSurvey : public DBDispatcher {
public:
    DB_SaveSatisfactionSurvey();
    virtual ~DB_SaveSatisfactionSurvey();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

bool Queue(int user_idx, const Request& request);

}  // namespace satisfaction_survey_0828

#endif  // OLDSRV_OLD_DATA_DB_EVENT_DB_SATISFACTION_SURVEY_0828_H_
