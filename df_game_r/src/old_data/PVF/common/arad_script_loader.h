#ifndef OLDSRV_DATAMANAGER_ARAD_SCRIPT_LOADER_H_
#define OLDSRV_DATAMANAGER_ARAD_SCRIPT_LOADER_H_

#include "old_data/PVF/common/rdar_script.h"

// IDA unit: 12_pvf_datamanager_scripts/Arad_ScriptLoader.
class Arad_ScriptLoader {
public:
    Arad_ScriptLoader();

    virtual int open(const char* filename);
    virtual int parsing(char* buffer);
};

#endif  // OLDSRV_DATAMANAGER_ARAD_SCRIPT_LOADER_H_
