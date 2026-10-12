#ifndef OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SCRIPT_INTERFACE_H
#define OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SCRIPT_INTERFACE_H

namespace ARAD {

// IDA type: ARAD::ScriptInterface.  The old vtable contains exactly these
// two entries: Load at +0 and Destroy at +4.  It has no virtual destructor.
struct ScriptInterface {
    ScriptInterface();

    virtual bool Load() = 0;
    virtual int Destroy() = 0;
};

}  // namespace ARAD

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(ARAD::ScriptInterface) == 0x04u,
              "ARAD::ScriptInterface IDA ABI size must stay 0x04");
#endif

#endif  // OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SCRIPT_INTERFACE_H
