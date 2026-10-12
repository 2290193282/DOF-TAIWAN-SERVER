#ifndef OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_UPGRADE_H_
#define OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_UPGRADE_H_

#include "old_data/db/core/db_dispatcher.h"

// IDA unit: 13_database_dispatch/DB_ReportMannerlessUser_Upgrade.
// The object is a vptr-only DBDispatcher derivative.  It is deliberately
// standalone here: DBTask registration and the Monitor/Inter return route are
// separate lifecycle work.
class DB_ReportMannerlessUser_Upgrade : public DBDispatcher {
public:
    // IDA_EA: 0x844E692
    // IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeC2Ev
    // DEMANGLED: DB_ReportMannerlessUser_Upgrade::DB_ReportMannerlessUser_Upgrade(void)
    DB_ReportMannerlessUser_Upgrade();

    // IDA_EA: 0x845EA28
    // IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeD2Ev
    // DEMANGLED: DB_ReportMannerlessUser_Upgrade::~DB_ReportMannerlessUser_Upgrade()
    // IDA_EA: 0x845EA58
    // IDA_NAME: _ZN31DB_ReportMannerlessUser_UpgradeD0Ev
    // DEMANGLED: DB_ReportMannerlessUser_Upgrade::~DB_ReportMannerlessUser_Upgrade()
    // ROLE: deleting destructor (D0)
    virtual ~DB_ReportMannerlessUser_Upgrade();

    // IDA_EA: 0x84276B0
    // IDA_NAME: _ZN31DB_ReportMannerlessUser_Upgrade8dispatchEiiP6Stream
    // DEMANGLED: DB_ReportMannerlessUser_Upgrade::dispatch(int,int,Stream *)
    virtual bool dispatch(int signal, int user_idx, Stream* stream);
};

#endif  // OLDSRV_OLD_DATA_DB_ANTI_CHEAT_DB_REPORT_MANNERLESS_USER_UPGRADE_H_
