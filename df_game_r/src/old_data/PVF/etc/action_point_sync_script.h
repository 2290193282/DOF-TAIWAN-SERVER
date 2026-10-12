#ifndef OLDSRV_APSYSTEM_ACTION_POINT_SYNC_SCRIPT_H_
#define OLDSRV_APSYSTEM_ACTION_POINT_SYNC_SCRIPT_H_

// IDA unit: 12_pvf_datamanager_scripts/APSystem::CSyncScript.
// IDA_EA: TruncateDescTable @0x812262A, InsertDescTable @0x812268C.
namespace APSystem {

class CSyncScript {
public:
    static bool TruncateDescTable();
    static bool InsertDescTable();
};

}  // namespace APSystem

#endif  // OLDSRV_APSYSTEM_ACTION_POINT_SYNC_SCRIPT_H_
