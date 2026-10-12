#ifndef OLDSRV_DB_DB_SAVE_FEATURED_GOODS_IDX_H_
#define OLDSRV_DB_DB_SAVE_FEATURED_GOODS_IDX_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_SaveFeaturedGoodsIdx.
// IDA_EA: dispatch @0x841B210; makeRequest @0x841B2DA; ctor/dtor
// @0x844E0E2/@0x845F9B2/@0x845F9E2. makeRequest emits signal 82.
class DB_SaveFeaturedGoodsIdx : public DBDispatcher {
public:
    DB_SaveFeaturedGoodsIdx();
    virtual ~DB_SaveFeaturedGoodsIdx();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    static void makeRequest(unsigned int account_id,
                            int charac_no,
                            int event_id,
                            int obtain_date,
                            int server_id);
};

#endif  // OLDSRV_DB_DB_SAVE_FEATURED_GOODS_IDX_H_
