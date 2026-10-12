#ifndef OLDSRV_OLD_DATA_PVF_EVENT_ACTION_SEND_MAIL_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_ACTION_SEND_MAIL_H_

#include <stdint.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

// IDA unit: 10_mail_trade_auction_cash/Action_SendMail.
// The type is owned by the PVF/RDAR [send mail] action script.  Its i386
// layout is fixed at 0x44 bytes by the old df_game_r constructors, parser,
// and EventClassify consumers.
struct Action_SendMail {
    Action_SendMail();
    Action_SendMail(const Action_SendMail& rhs);
    ~Action_SendMail();

    Action_SendMail& operator=(const Action_SendMail& rhs);
    void reset();
    Action_SendMail* set_item(int item_id, int item_count);

    std::map<int, std::pair<int, int> > job_item_counts;  // +0x00
    int reserved_18;                                     // +0x18
    uint8_t direct_inventory;                            // +0x1c
    int item_id;                                         // +0x20
    int item_count;                                      // +0x24
    int money;                                           // +0x28
    signed char occ_time;                                // +0x2c
    std::string title_string_key;                        // +0x30
    std::string message_string_key;                      // +0x34
    std::vector<int> result_item_values;                 // +0x38
};

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_ACTION_SEND_MAIL_H_
