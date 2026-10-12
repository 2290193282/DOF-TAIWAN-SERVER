#include "old_game/social/party/world_quick_party_pool.h"

namespace QuickParty {

typedef char STQuickPartyPoolMap_Key_Size_Must_Be_0x04[
    (sizeof(STQuickPartyPoolMap_Key) == 0x04) ? 1 : -1];
typedef char STQuickPartyPoolMap_Data_Size_Must_Be_0x20[
    (sizeof(STQuickPartyPoolMap_Data) == 0x20) ? 1 : -1];

// IDA_EA: 0x82344CA
// IDA_NAME: _ZN10QuickParty23STQuickPartyPoolMap_KeyC2Ev
// DEMANGLED: QuickParty::STQuickPartyPoolMap_Key::STQuickPartyPoolMap_Key(void)
STQuickPartyPoolMap_Key::STQuickPartyPoolMap_Key()
    : pool_id(0),
      channel_type(0xffu) {}

// IDA_EA: 0x826B808
// IDA_NAME: _ZNK10QuickParty23STQuickPartyPoolMap_KeyltERKS0_
// DEMANGLED: QuickParty::STQuickPartyPoolMap_Key::operator<(QuickParty::STQuickPartyPoolMap_Key const&)const
bool STQuickPartyPoolMap_Key::operator<(
    const STQuickPartyPoolMap_Key& rhs) const {
    if (pool_id < rhs.pool_id) {
        return true;
    }
    return pool_id == rhs.pool_id && channel_type < rhs.channel_type;
}

// IDA_EA: 0x826B854
// IDA_NAME: _ZN10QuickParty24STQuickPartyPoolMap_DataC2Ev
// DEMANGLED: QuickParty::STQuickPartyPoolMap_Data::STQuickPartyPoolMap_Data(void)
STQuickPartyPoolMap_Data::STQuickPartyPoolMap_Data() {
    first_list.clear();
    second_list.clear();
    third_list.clear();
    fourth_list.clear();
}

// IDA_EA: 0x826B9D2
// IDA_NAME: _ZN10QuickParty24STQuickPartyPoolMap_DataD2Ev
// DEMANGLED: QuickParty::STQuickPartyPoolMap_Data::~STQuickPartyPoolMap_Data()
STQuickPartyPoolMap_Data::~STQuickPartyPoolMap_Data() {}

// IDA_EA: 0x826BB46
// IDA_NAME: _ZN10QuickParty24STQuickPartyPoolMap_DataC2ERKS0_
// DEMANGLED: QuickParty::STQuickPartyPoolMap_Data::STQuickPartyPoolMap_Data(QuickParty::STQuickPartyPoolMap_Data const&)
STQuickPartyPoolMap_Data::STQuickPartyPoolMap_Data(
    const STQuickPartyPoolMap_Data& rhs)
    : first_list(rhs.first_list),
      second_list(rhs.second_list),
      third_list(rhs.third_list),
      fourth_list(rhs.fourth_list) {}

}  // namespace QuickParty
