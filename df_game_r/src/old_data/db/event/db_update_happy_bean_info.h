#ifndef DNF_DB_UPDATE_HAPPY_BEAN_INFO_H_
#define DNF_DB_UPDATE_HAPPY_BEAN_INFO_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_UpdateHappyBeanInfo.
class DB_UpdateHappyBeanInfo : public DBDispatcher {
public:
    DB_UpdateHappyBeanInfo();
    virtual ~DB_UpdateHappyBeanInfo();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int user_idx,
                            unsigned int member_id,
                            unsigned int event_value,
                            const char* character_name);
};

#endif  // DNF_DB_UPDATE_HAPPY_BEAN_INFO_H_
