#ifndef OLDSRV_OLD_DATA_PVF_EVENT_EVENT_AVATAR_LIST_SCRIPT_H_
#define OLDSRV_OLD_DATA_PVF_EVENT_EVENT_AVATAR_LIST_SCRIPT_H_

#include <set>

// IDA unit: 11_event_runtime/EventAvatarListScript. This PVF record is
// embedded in CDataManager and canonical in B's PVF/event module.
struct EventAvatarListScript {
    EventAvatarListScript();
    ~EventAvatarListScript();

    int clear();
    int isfindEventAvatar(int avatar_index);

    std::set<int> avatar_indexes;
};

int importEventAvatarListScript(EventAvatarListScript* script,
                                const char* filename);

static_assert(sizeof(EventAvatarListScript) == 0x18u,
              "EventAvatarListScript IDA ABI size must stay 0x18");

#endif  // OLDSRV_OLD_DATA_PVF_EVENT_EVENT_AVATAR_LIST_SCRIPT_H_
