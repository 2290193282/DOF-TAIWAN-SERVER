#ifndef OLDSRV_OLD_DATA_PVF_DATA_SCRIPT_HASH_PARAM_H_
#define OLDSRV_OLD_DATA_PVF_DATA_SCRIPT_HASH_PARAM_H_

#include "infra/network/packet/message_base.h"


// IDA unit: 12_pvf_datamanager_scripts/ScriptHashParam.
class ScriptHashParam : public ParamBase {
public:
    ScriptHashParam();
    virtual ~ScriptHashParam();

    unsigned char script_type;
    unsigned char script_index;
    int script_hash;
};

#endif  // OLDSRV_OLD_DATA_PVF_DATA_SCRIPT_HASH_PARAM_H_
