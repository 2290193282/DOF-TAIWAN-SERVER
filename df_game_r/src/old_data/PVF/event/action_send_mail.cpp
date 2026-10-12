#include "old_data/PVF/event/action_send_mail.h"

namespace {
typedef char Action_SendMail_Size_Must_Be_0x44[
    (sizeof(Action_SendMail) == 0x44) ? 1 : -1];
}  // namespace

// IDA_EA: 0x88E40CE
// IDA_NAME: _ZN15Action_SendMailC2Ev
// DEMANGLED: Action_SendMail::Action_SendMail(void)
Action_SendMail::Action_SendMail() {}

// IDA_EA: 0x88E4160
// IDA_NAME: _ZN15Action_SendMailD2Ev
// DEMANGLED: Action_SendMail::~Action_SendMail()
Action_SendMail::~Action_SendMail() {}

// IDA_EA: 0x8987B48
// IDA_NAME: _ZN15Action_SendMailC2ERKS_
// DEMANGLED: Action_SendMail::Action_SendMail(Action_SendMail const&)
Action_SendMail::Action_SendMail(const Action_SendMail& rhs)
    : job_item_counts(rhs.job_item_counts),
      reserved_18(rhs.reserved_18),
      direct_inventory(rhs.direct_inventory),
      item_id(rhs.item_id),
      item_count(rhs.item_count),
      money(rhs.money),
      occ_time(rhs.occ_time),
      title_string_key(rhs.title_string_key),
      message_string_key(rhs.message_string_key),
      result_item_values(rhs.result_item_values) {}

// IDA_EA: 0x898A922
// IDA_NAME: _ZN15Action_SendMailaSERKS_
// DEMANGLED: Action_SendMail::operator=(Action_SendMail const&)
Action_SendMail& Action_SendMail::operator=(const Action_SendMail& rhs) {
    job_item_counts = rhs.job_item_counts;
    reserved_18 = rhs.reserved_18;
    direct_inventory = rhs.direct_inventory;
    item_id = rhs.item_id;
    item_count = rhs.item_count;
    money = rhs.money;
    occ_time = rhs.occ_time;
    title_string_key = rhs.title_string_key;
    message_string_key = rhs.message_string_key;
    result_item_values = rhs.result_item_values;
    return *this;
}

// IDA_EA: 0x80EC026
// IDA_NAME: _ZN15Action_SendMail5resetEv
// DEMANGLED: Action_SendMail::reset(void)
void Action_SendMail::reset() {
    job_item_counts.clear();
    reserved_18 = 0;
    direct_inventory = 0;
    item_count = 0;
    item_id = 0;
    money = 0;
    occ_time = 0;
    title_string_key.clear();
    message_string_key.clear();
}

// IDA_EA: 0x80EC08C
// IDA_NAME: _ZN15Action_SendMail8set_itemEii
// DEMANGLED: Action_SendMail::set_item(int,int)
Action_SendMail* Action_SendMail::set_item(int item_id_value,
                                           int item_count_value) {
    item_id = item_id_value;
    item_count = item_count_value;
    return this;
}
