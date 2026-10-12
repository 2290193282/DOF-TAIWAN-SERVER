#ifndef OLDSRV_OLD_GAME_EVENT_LEVELUP_SUPPORT_SCRIPT_SERVICES_H_
#define OLDSRV_OLD_GAME_EVENT_LEVELUP_SUPPORT_SCRIPT_SERVICES_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "old_data/PVF/arad/arad_script_interface.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_data/PVF/data/levelup_support_item_info.h"

class CUser;

// IDA unit: 99_misc_global/LevelupSupportSciprt.  This is distinct from the
// later LevelupSupport2ndScript and owns tw_LevelupSupport.evt.
class LevelupSupportSciprt {
public:
    struct ItemInfo {
        ItemInfo(int32_t item_index_value, int item_count_value)
            : item_index(item_index_value),
              item_count(static_cast<int16_t>(item_count_value)),
              reserved(0) {}

        int32_t item_index;
        int16_t item_count;
        int16_t reserved;
    };

    LevelupSupportSciprt() : reward_items_by_level() {}
    ~LevelupSupportSciprt() { clearScript(); }

    int loadScript();
    int clearScript();

    std::map<int, std::vector<ItemInfo>* > reward_items_by_level;
};

// IDA unit: 11_event_runtime/LevelupSupport2ndEventManger.
class LevelupSupport2ndEventManger
    : public ARAD::Singleton<LevelupSupport2ndEventManger>,
      public ARAD::ScriptInterface {
public:
    LevelupSupport2ndEventManger();
    ~LevelupSupport2ndEventManger();

    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x8147880
    // IDA_NAME: _ZN28LevelupSupport2ndEventManger7ProcessEP5CUser
    void Process(CUser* user);

private:
    LevelupSupport2ndScript script_;
};

namespace ARAD {
template <>
Singleton<LevelupSupport2ndEventManger>::Singleton();
template <>
LevelupSupport2ndEventManger* Singleton<LevelupSupport2ndEventManger>::Get();
}  // namespace ARAD

#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
static_assert(sizeof(LevelupSupportSciprt::ItemInfo) == 0x08u,
              "LevelupSupportSciprt ItemInfo i386 ABI must stay 0x08");
static_assert(sizeof(LevelupSupportSciprt) == 0x18u,
              "LevelupSupportSciprt i386 ABI must stay 0x18");
static_assert(sizeof(LevelupSupport2ndEventManger) == 0x28u,
              "LevelupSupport2ndEventManger i386 ABI must stay 0x28");
#endif

#endif  // OLDSRV_OLD_GAME_EVENT_LEVELUP_SUPPORT_SCRIPT_SERVICES_H_
