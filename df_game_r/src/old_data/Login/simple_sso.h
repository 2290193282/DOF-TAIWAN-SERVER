#ifndef OLDSRV_OLD_DATA_LOGIN_SIMPLE_SSO_H_
#define OLDSRV_OLD_DATA_LOGIN_SIMPLE_SSO_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/core/mysql.h"
#include "infra/security/crypto/crypto_rijndael.h"

namespace WongWork {

class CSimpleSSO {
public:
    struct stOutputData_t {
        stOutputData_t();

        int last_play_time;
        int account_fail;
        int total_account_fail;
        int report_count;
        char reliable_flag;
        uint8_t reserved_11[3];
        uint32_t trade_gold_daily;
        char login_ip[20];
        char security_flag;
        uint8_t reserved_2d[3];
        uint32_t dungeon_gain_gold;
        int school_id;
        float rating;
        uint8_t tutorial_skipable;
        uint8_t event_charac_flag;
        uint8_t reserved_3e[2];
        uint32_t login_time;
        uint32_t expire_time;
    };

    struct stUpdateSSOExpireTime_t {
        int report_count;
        int trade_gold_daily;
        int dungeon_gain_gold;
        float rating;
        uint8_t tutorial_skipable;
        uint8_t reserved_11[3];
    };

    CSimpleSSO();
    ~CSimpleSSO();

    bool init(MySQL* mysql);
    CSimpleSSO* reset();
    int checkValidPassword(unsigned int account_id,
                           const char* password,
                           const char* token,
                           stOutputData_t& output);
    bool updateSSOExpireTime(unsigned int account_id,
                             const stUpdateSSOExpireTime_t& update);
    bool updateSSOExpireTime(unsigned int account_id,
                             unsigned int login_time);
    void expireLoginTime(unsigned int account_id);
    int _getGameStartTime(unsigned int account_id,
                          long& login_time,
                          long& expire_time,
                          stOutputData_t& output);
    int getGameStartTime(unsigned int account_id,
                         long& login_time,
                         long& expire_time,
                         stOutputData_t& output);
    bool updateHumanCertify(unsigned int account_id, int account_fail);

private:
    static const char* const strKeyTable_[31][24];

    CRijndael rijndael_[31][24];
    MySQL* mysql_;
};

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(CSimpleSSO::stOutputData_t) == 0x48u,
              "CSimpleSSO output ABI size must stay 0x48");
static_assert(offsetof(CSimpleSSO::stOutputData_t, reliable_flag) == 0x10u,
              "CSimpleSSO reliable flag offset must stay 0x10");
static_assert(offsetof(CSimpleSSO::stOutputData_t, login_ip) == 0x18u,
              "CSimpleSSO login IP offset must stay 0x18");
static_assert(offsetof(CSimpleSSO::stOutputData_t, login_time) == 0x40u,
              "CSimpleSSO login time offset must stay 0x40");
static_assert(sizeof(CSimpleSSO::stUpdateSSOExpireTime_t) == 0x14u,
              "CSimpleSSO update ABI size must stay 0x14");
static_assert(sizeof(CSimpleSSO) == 0xD0864u,
              "CSimpleSSO old object size must stay 0xD0864");
#endif

}  // namespace WongWork

#endif  // OLDSRV_OLD_DATA_LOGIN_SIMPLE_SSO_H_
