#ifndef OLDSRV_DB_DB_REMOVE_FEATURED_GOODS_IDX_H_
#define OLDSRV_DB_DB_REMOVE_FEATURED_GOODS_IDX_H_

#include "old_data/db/core/db_dispatcher.h"


// IDA unit: 13_database_dispatch/DB_RemoveFeaturedGoodsIdx.
// IDA_EA: dispatch @0x841B41A; ctor/dtor
// @0x844E0FE/@0x845F964/@0x845F994. Registered at DBTask signal 83.
class DB_RemoveFeaturedGoodsIdx : public DBDispatcher {
public:
    DB_RemoveFeaturedGoodsIdx();
    virtual ~DB_RemoveFeaturedGoodsIdx();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_DB_DB_REMOVE_FEATURED_GOODS_IDX_H_
