#ifndef OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SCRIPT_MANAGER_H
#define OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SCRIPT_MANAGER_H

#include <set>

#include "old_data/PVF/arad/arad_script_interface.h"
#include "old_data/PVF/arad/arad_singleton.h"

namespace ARAD {

class ScriptManager : public Singleton<ScriptManager> {
public:
    ScriptManager();
    ~ScriptManager();

    bool LoadScript();
    bool DestroyScript();
    bool RegistScript(ScriptInterface* script, int source_line);

    // IDA_EA: 0x81A13F6.  The original body registers all nine script
    // services atomically; keep it unresolved until every typed provider is
    // available rather than silently registering a partial subset.
    bool Init();

private:
    std::set<ScriptInterface*> scripts_;
};

template <>
Singleton<ScriptManager>::Singleton();

template <>
ScriptManager* Singleton<ScriptManager>::Get();

}  // namespace ARAD

#endif
