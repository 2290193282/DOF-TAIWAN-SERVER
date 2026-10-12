#ifndef OLDSRV_OLD_DATA_DB_ACCOUNT_ARAD_LOGIN_QUERY_H_
#define OLDSRV_OLD_DATA_DB_ACCOUNT_ARAD_LOGIN_QUERY_H_

#include "old_data/db/account/db_login.h"
#include "old_data/db/account/login_payloads.h"

#include <vector>


namespace ARAD {
namespace DATABASE {
namespace LOGIN_QUERY {

int arad_sp_select_account_info(SIG_LOGIN_DATA* payload, char* password);
int arad_sp_select_punish_second_user(SIG_LOGIN_DATA* payload);
int arad_sp_select_punish_user_info(SIG_LOGIN_DATA* payload);
int arad_sp_convert_pass_pad_info(SIG_LOGIN_DATA* payload);
int arad_sp_check_goblin_reward(SIG_LOGIN_DATA* payload, char* timestamp);
int arad_sp_delete_member_mouspass(SIG_LOGIN_DATA* payload);
int arad_sp_update_fail_record(SIG_LOGIN_DATA* payload);
int arad_sp_update_history(SIG_LOGIN_DATA* payload, int modify_type);
int arad_sp_load_event_entry(
    SIG_LOGIN_DATA* payload,
    std::vector<DB_Login::tagDNFEventEntry>& event_entries);
int arad_sp_select_member_brodcast(
    SIG_LOGIN_DATA* payload,
    DB_Login::tagDNFEventEntry& event_entry,
    int user_idx);
int arad_sp_select_member_premium(SIG_LOGIN_DATA* payload,
                                  DB_Login::tagDNFEventEntry& event_entry,
                                  std::vector<int>& event_ids,
                                  std::vector<int> remove_event_ids);

}  // namespace LOGIN_QUERY
}  // namespace DATABASE
}  // namespace ARAD


#endif  // OLDSRV_OLD_DATA_DB_ACCOUNT_ARAD_LOGIN_QUERY_H_
