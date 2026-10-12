#ifndef OLDSRV_DB_DB_MERCENARY_LOG_INFO_H_
#define OLDSRV_DB_DB_MERCENARY_LOG_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA_EA: 0x844FAEA/0x845B39E/0x845B3CE/0x8446196/0x8446340.
// IDA unit: 02_login_account/DB_MercenaryLogInfo.
class DB_MercenaryLogInfo : public DBDispatcher {
public:
    DB_MercenaryLogInfo();
    virtual ~DB_MercenaryLogInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int m_id, unsigned int charac_no,
                            unsigned int start_time, unsigned int finish_time,
                            char type, char period, char area, int gold,
                            int item_id, int rarity);
};

#endif  // OLDSRV_DB_DB_MERCENARY_LOG_INFO_H_
