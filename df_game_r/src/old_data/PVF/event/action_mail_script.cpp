#include "old_data/PVF/event/action_mail_script.h"

#include "old_data/PVF/common/datamanager/rdar_script.h"

#include <string>
#include <utility>

// IDA_EA: 0x897F147
// IDA_NAME: _Z22importActionMailScriptP15Action_SendMail
// DEMANGLED: importActionMailScript(Action_SendMail *)
int importActionMailScript(Action_SendMail* mail) {
    std::string tag;
    bool success = false;

    while (ScanType(tag, true) == 1) {
        if (tag == "[inven]") {
            mail->direct_inventory = static_cast<uint8_t>(ScanInt(&success));
        } else if (tag == "[job]") {
            while (ScanType(tag, true) == 1) {
                int job = -1;
                if (tag == "[swordman]") {
                    job = 0;
                } else if (tag == "[fighter]") {
                    job = 1;
                } else if (tag == "[gunner]") {
                    job = 2;
                } else if (tag == "[mage]") {
                    job = 3;
                } else if (tag == "[priest]") {
                    job = 4;
                } else if (tag == "[atgunner]") {
                    job = 5;
                } else if (tag == "[thief]") {
                    job = 6;
                } else if (tag == "[atfighter]") {
                    job = 7;
                } else if (tag == "[/job]") {
                    break;
                }

                if (job != -1) {
                    const int first = ScanInt(&success);
                    const int second = ScanInt(&success);
                    mail->job_item_counts.insert(
                        std::make_pair(job, std::make_pair(first, second)));
                }
            }
        } else if (tag == "[detail]") {
            mail->reserved_18 = 0;
            mail->item_id = ScanInt(&success);
            mail->item_count = ScanInt(&success);
            mail->money = ScanInt(&success);
            mail->occ_time = static_cast<signed char>(ScanInt(&success));
            if (static_cast<unsigned char>(mail->occ_time) >= 16u) {
                mail->occ_time = 0;
            }
            ScanStr(&mail->title_string_key);
            ScanStr(&mail->message_string_key);
        } else if (tag == "[/send mail]") {
            return 1;
        }
    }

    return 0;
}
