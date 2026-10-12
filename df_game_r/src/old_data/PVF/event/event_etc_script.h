#ifndef OLDSRV_OLD_DATA_PVF_EVENT_EVENT_ETC_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_EVENT_ETC_SCRIPT_H_

#include <map>
#include <vector>

// IDA unit: 11_event_runtime/EventEtcScript.
// CDataManager embeds this 0x24-byte PVF value at +0xB520.
struct EventEtcScript {
    EventEtcScript();
    ~EventEtcScript();

    void clear();

    std::vector<int> event_ids;
    std::map<unsigned int, unsigned int> values;
};

static_assert(sizeof(EventEtcScript) == 0x24u,
              "EventEtcScript IDA ABI size must stay 0x24");

// IDA_EA: 0x895E30F
int importEventEtcScript(EventEtcScript* script,
                         const char* filename);

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_EVENT_ETC_SCRIPT_H_
