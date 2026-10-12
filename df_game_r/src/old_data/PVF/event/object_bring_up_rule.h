#ifndef OLDSRV_OLD_DATA_PVF_EVENT_OBJECT_BRING_UP_RULE_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_OBJECT_BRING_UP_RULE_H_

#include <stdint.h>

#include <string>
#include <utility>
#include <vector>

#include "old_data/PVF/event/action_send_mail.h"
#include "old_game/activity/schedule/event_value_types.h"


// IDA unit: 12_pvf_datamanager_scripts/ObjectBringUpScript and
// ObjectBringUpScript_ObjectBringUpRule.
class ObjectBringUpScript : public InGameEventScript {
public:
    // IDA_EA: 0x8985E0A
    ObjectBringUpScript();

    struct ObjectBringUpRule {
        // IDA_EA: 0x8985DD4
        ObjectBringUpRule();

        // IDA_EA: 0x8985DE8
        ObjectBringUpRule* reset();

        int32_t value_00;
        int32_t value_04;
        int32_t value_08;
    };

    std::string value_08;
    uint8_t flag_0c;
    std::vector<int> values_10;
    std::vector<std::pair<std::vector<unsigned int>, Action_SendMail> >
        mail_rules_1c;
    std::vector<std::pair<ObjectBringUpRule, Action_SendMail> >
        bring_up_rules_28;
};

// IDA_EA: 0x8983E48
int importObjectBringUpEventScript(ObjectBringUpScript* script,
                                   const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_OBJECT_BRING_UP_RULE_H_
