#ifndef OLDSRV_DB_DB_ACTION_POINT_REWARD_MEDAL_H_
#define OLDSRV_DB_DB_ACTION_POINT_REWARD_MEDAL_H_

#include "old_data/db/core/db_dispatcher.h"

namespace APSystem {

#pragma pack(push, 1)
struct _SIG_LOAD_REWARD_MEDAL {
    unsigned int charac_no;
    unsigned int ap_sum;
    char occ_date[11];
};

struct _SIG_UPDATE_REWARD_MEDAL {
    unsigned int charac_no;
    char occ_date[11];
};
#pragma pack(pop)

static_assert(sizeof(_SIG_LOAD_REWARD_MEDAL) == 0x13u,
              "_SIG_LOAD_REWARD_MEDAL old ABI size must stay 0x13");
static_assert(sizeof(_SIG_UPDATE_REWARD_MEDAL) == 0x0Fu,
              "_SIG_UPDATE_REWARD_MEDAL old ABI size must stay 0x0F");

// IDA_EA: 0x812487A/0x8124882/0x8124910/0x8124B8E/0x844DDB2.
// IDA unit: 11_event_runtime/APSystem/DB_LoadRewardMedal, registered from
// 99_misc_global/DBTask as DB signal 20.
class DB_LoadRewardMedal : public DBDispatcher {
public:
    DB_LoadRewardMedal();
    virtual ~DB_LoadRewardMedal();

    static void makeRequest(int user_idx, unsigned int charac_no);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool Select(_SIG_LOAD_REWARD_MEDAL& payload);
    bool SendResult(int signal,
                    int user_idx,
                    _SIG_LOAD_REWARD_MEDAL& payload);
};

// IDA_EA: 0x8124C70/0x8124C78/0x8124CB2/0x844DDCE.
// IDA unit: 11_event_runtime/APSystem/DB_UpdateRewardMedal, registered from
// 99_misc_global/DBTask as DB signal 21.
class DB_UpdateRewardMedal : public DBDispatcher {
public:
    DB_UpdateRewardMedal();
    virtual ~DB_UpdateRewardMedal();

    static void makeRequest(int user_idx,
                            const _SIG_LOAD_REWARD_MEDAL& payload);
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool Update(_SIG_UPDATE_REWARD_MEDAL& payload);
};

}  // namespace APSystem
#endif  // OLDSRV_DB_DB_ACTION_POINT_REWARD_MEDAL_H_
