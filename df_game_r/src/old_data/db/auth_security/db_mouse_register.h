#ifndef OLDSRV_DB_DB_MOUSE_REGISTER_H_
#define OLDSRV_DB_DB_MOUSE_REGISTER_H_

#include "old_data/db/core/db_dispatcher.h"

#include <stddef.h>
#include <stdint.h>


#pragma pack(push, 1)
// The C sender @0x82637FE and Inter consumer @0x84D07D2 establish every
// field boundary used by DB_MouseRegister @0x8422964.
struct SIG_MOUSE_REGISTER {
    uint32_t account_id;                 // +0x00
    uint32_t request_kind;               // +0x04
    char previous_password[7];           // +0x08
    char requested_password[7];          // +0x0F
    int32_t result;                       // +0x16
    uint8_t activated;                   // +0x1A
    int32_t cancel_count;                 // +0x1B
    int32_t fail_count;                   // +0x1F
    char encrypted_password[37];          // +0x23; 36 copied + NUL
    char ip[16];                          // +0x48
    uint8_t advice_change_password;       // +0x58
};

struct SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD {
    uint32_t advice_change_password;
};
#pragma pack(pop)

static_assert(sizeof(SIG_MOUSE_REGISTER) == 0x59u,
              "SIG_MOUSE_REGISTER IDA ABI size must stay 0x59");
static_assert(offsetof(SIG_MOUSE_REGISTER, result) == 0x16u,
              "SIG_MOUSE_REGISTER result offset must stay 0x16");
static_assert(offsetof(SIG_MOUSE_REGISTER, cancel_count) == 0x1Bu,
              "SIG_MOUSE_REGISTER cancel count offset must stay 0x1B");
static_assert(offsetof(SIG_MOUSE_REGISTER, fail_count) == 0x1Fu,
              "SIG_MOUSE_REGISTER fail count offset must stay 0x1F");
static_assert(offsetof(SIG_MOUSE_REGISTER, encrypted_password) == 0x23u,
              "SIG_MOUSE_REGISTER password offset must stay 0x23");
static_assert(offsetof(SIG_MOUSE_REGISTER, ip) == 0x48u,
              "SIG_MOUSE_REGISTER IP offset must stay 0x48");
static_assert(offsetof(SIG_MOUSE_REGISTER, advice_change_password) == 0x58u,
              "SIG_MOUSE_REGISTER advice offset must stay 0x58");
static_assert(sizeof(SIG_GOBLIN_PAD_ADVICE_CHANGE_PWD) == 4u,
              "goblin-pad advice payload must stay four bytes");


class MySQL;
class Stream;

// IDA_EA: 0x8422938/0x8422964/0x8422C8A/0x8422E6A/0x84231B6/
// 0x84232E0/0x8423528/0x8423794/0x8423914/0x8423ABE/
// 0x844E4B6/0x845EFF2/0x845F022.
// IDA unit: 13_database_dispatch/DB_MouseRegister. Registered by
// DBTask::RegistDispatcher @0x83FBFEE at old storage slot +164, active signal
// 163.
class DB_MouseRegister : public DBDispatcher {
public:
    DB_MouseRegister();
    virtual ~DB_MouseRegister();

    static MySQL* _getDBHandleForDevideAccountDB();
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static int checkPassword(unsigned int account_id,
                             const char* password,
                             bool fail_lock_check,
                             int& fail_count,
                             int& cancel_count,
                             bool& password_found,
                             char* mousepass,
                             char* encrypted_password);
    static int convertPassPadInfo(unsigned int account_id,
                                  char* password,
                                  int& fail_count,
                                  char& passpad_result,
                                  int& cancel_count,
                                  bool& passpad_enabled,
                                  char* mousepass,
                                  bool& validity_flag,
                                  char* ip,
                                  long& validity_time,
                                  bool& month_old_flag);
    static int _updateFailCnt(unsigned int account_id,
                              int fail_count,
                              char* ip);
    static int _registNewPassword(unsigned int account_id,
                                  const char* encrypted_password,
                                  char* ip,
                                  int user_idx);
    static int _modifyPassword(unsigned int account_id,
                               const char* encrypted_password,
                               char* ip,
                               int user_idx,
                               bool advice_change_password);
    static int _unregistPassword(unsigned int account_id, char* ip);
    static bool _updateHistory(unsigned int account_id,
                               int modify_type,
                               char* ip);
    static void _sendResult(unsigned int account_id,
                            int result,
                            int request_kind,
                            int user_idx,
                            SIG_MOUSE_REGISTER* payload);
};

#endif  // OLDSRV_DB_DB_MOUSE_REGISTER_H_
