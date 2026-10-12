#ifndef OLDSRV_DB_DB_CREATE_CHARAC_H_
#define OLDSRV_DB_DB_CREATE_CHARAC_H_

#include "old_data/db/character/create_charac_payload.h"
#include "old_data/db/core/db_dispatcher.h"

class MySQL;
class Stream;
// IDA unit: 13_database_dispatch/DB_CreateCharac.
// IDA_EA: MakeInsertCharacInfo @0x80EC2A0; ctor/dtor
// @0x8400CC2/@0x8400CDE/@0x8400D0E; dispatch @0x8400D2C; DB helpers
// @0x840103E/@0x8401094/@0x84010FC/@0x84011D4/@0x84013B2/@0x8401428;
// limit-create helpers @0x8401504/@0x84016AE/@0x8401864/@0x8401922/
// @0x8401B9A; create chain @0x8401DCC/@0x840215E/@0x8402230/
// @0x84023AC/@0x84024E6/@0x84026EC/@0x840275C/@0x84027B0.
// The old C++ ABI keeps DB_CreateCharac in the global namespace; DB_LoadEtc
// calls DB_CreateCharac::CreatePvP(unsigned int) through that symbol.
class DB_CreateCharac : public DBDispatcher {
public:
    DB_CreateCharac();
    ~DB_CreateCharac();

    bool dispatch(int signal, int user_idx, Stream* stream);

    int MakeInsertCharacInfo(MySQL* db_handle,
                             SIG_CREATE_CHARAC* payload,
                             const char* escaped_name);
    bool _createCharacNPCRelationship(unsigned int charac_no);
    bool _createCharacQuest(unsigned int charac_no);
    bool _createCharacDungeon(unsigned int account_id);
    bool _checkCharacName(MySQL* db_handle,
                          const char* database_name,
                          SIG_CREATE_CHARAC* payload);
    bool _checkCreateEventCharacter(SIG_CREATE_CHARAC* payload);
    bool _updateCreateEventCharacter(SIG_CREATE_CHARAC* payload);
    bool CheckLimitCreateNewCharac(SIG_CREATE_CHARAC* payload);
    bool InsertLimitCreateNewCharac(SIG_CREATE_CHARAC* payload);
    void makeRequestResetLimitCreateMid();
    bool CheckLimitCreateNewCharac(SIG_CREATE_CHARAC* payload,
                                   bool use_mid_limit);
    bool InsertLimitCreateNewCharac(SIG_CREATE_CHARAC* payload,
                                    bool use_mid_limit);
    bool CreateNewCharac(SIG_CREATE_CHARAC* payload);
    bool _CreateNewCharacStat(SIG_CREATE_CHARAC* payload);
    bool CreateInventory(unsigned int charac_no, Inven_Item* inventory);
    bool CreateEquip(unsigned int charac_no, Inven_Item* equip);
    bool CreateSkill(SIG_CREATE_CHARAC* payload);
    bool CreatePvP(unsigned int charac_no);
    bool GetCharacNo(SIG_CREATE_CHARAC* payload);
    bool SendResult(int signal, int user_idx, SIG_CREATE_CHARAC* payload);
};

using ::DB_CreateCharac;
#endif  // OLDSRV_DB_DB_CREATE_CHARAC_H_
