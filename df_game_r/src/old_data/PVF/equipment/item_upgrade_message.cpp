#include "old_data/PVF/equipment/item_upgrade.h"

#include <string.h>

// IDA_EA: 0x822DCEA
// IDA_NAME: _ZN16MSG_ITEM_UPGRADEC2Ev
// DEMANGLED: MSG_ITEM_UPGRADE::MSG_ITEM_UPGRADE(void)
MSG_ITEM_UPGRADE::MSG_ITEM_UPGRADE()
    : MSG_BASE() {
    body_.upgrade_count = 0;
    ::memset(body_.guild_item_name, 0, sizeof(body_.guild_item_name));
    body_.upgrade_rate = 0;
}

// IDA_EA: 0x8231048
// IDA_NAME: _ZN16STUdateItemParamC2Ev
// DEMANGLED: STUdateItemParam::STUdateItemParam(void)
STUdateItemParam::STUdateItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247A98
// IDA_NAME: _ZN16STUdateItemParamD2Ev
// DEMANGLED: STUdateItemParam::~STUdateItemParam()
// IDA_EA: 0x8247AC8
// IDA_NAME: _ZN16STUdateItemParamD0Ev
// DEMANGLED: STUdateItemParam::~STUdateItemParam()
STUdateItemParam::~STUdateItemParam() {}
