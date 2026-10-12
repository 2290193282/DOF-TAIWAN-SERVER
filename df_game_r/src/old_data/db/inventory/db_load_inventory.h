#ifndef OLDSRV_DB_DB_LOAD_INVENTORY_H_
#define OLDSRV_DB_DB_LOAD_INVENTORY_H_

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/inventory/load_inventory_payload.h"

// IDA_EA: 0x840A604/0x840A620/0x840A650/0x840A66E/0x840AAC8/0x840ABA2/0x840AD2A/0x840B0D8/0x840BD3A/0x840BE8A/0x840CA74/0x840D766/0x840D98C/0x840DA6E.
// IDA unit: 13_database_dispatch/DB_LoadInventory.
class DB_LoadInventory : public DBDispatcher {
public:
    DB_LoadInventory();
    virtual ~DB_LoadInventory();

    virtual bool dispatch(int signal, int user_idx, Stream* stream);
    bool _GetEquipMaxItemLevel(SIG_LOAD_INVENTORY* payload);
    static bool _GetTagCharacInven(unsigned int charac_no,
                                   TAG_CHARAC_INVEN_DATA* tag_payload);
    int _getCharacInvenExpand(SIG_LOAD_INVENTORY* payload) const;
    bool GetUserInven(SIG_LOAD_INVENTORY* payload);
    bool GetUserAvatar(SIG_LOAD_INVENTORY* payload);
    static bool _GetUserAvatar(unsigned int charac_no,
                               AvatarDetailInfo* avatar_detail,
                               Inven_Item* avatar_equip_slot,
                               Inven_Item* avatar_inven,
                               int& avatar_count);
    bool GetUserCreature(SIG_LOAD_INVENTORY* payload);
    static int GetItemLock(unsigned int charac_no,
                           stItemLockInfo* item_lock);
    bool SendResult_forchina(int signal,
                             int user_idx,
                             SIG_LOAD_INVENTORY* payload);
    bool SendResult(int signal,
                    int user_idx,
                    SIG_LOAD_INVENTORY* payload);
};

#endif  // OLDSRV_DB_DB_LOAD_INVENTORY_H_
