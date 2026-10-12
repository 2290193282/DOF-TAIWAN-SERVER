#include "old_game/economy/shop/npc_limit_packets.h"

#include <cstring>


typedef char LimitNpcBuyItemRequestInfo_Size_Must_Be_0x0a[
    (sizeof(LimitNpcBuyItemRequestInfo) == 0x0a) ? 1 : -1];
typedef char LimitNpcBuyItemInfo_Size_Must_Be_0x26[
    (sizeof(LimitNpcBuyItemInfo) == 0x26) ? 1 : -1];
typedef char LimitNpcBuyItemInfoAll_Size_Must_Be_0x182[
    (sizeof(LimitNpcBuyItemInfoAll) == 0x182) ? 1 : -1];
typedef char LimitNpcBuyItemUpdate_Size_Must_Be_0x1a[
    (sizeof(LimitNpcBuyItemUpdate) == 0x1a) ? 1 : -1];

// IDA_EA: 0x84B41FE
// IDA_NAME: _ZN26LimitNpcBuyItemRequestInfoC2Ev
// DEMANGLED: LimitNpcBuyItemRequestInfo::LimitNpcBuyItemRequestInfo(void)
LimitNpcBuyItemRequestInfo::LimitNpcBuyItemRequestInfo()
    : PacketHeader(0x27D8u, 0x000Au) {}

// IDA_EA: 0x822C4D0
// IDA_NAME: _ZN19LimitNpcBuyItemInfoC2Ev
// DEMANGLED: LimitNpcBuyItemInfo::LimitNpcBuyItemInfo(void)
LimitNpcBuyItemInfo::LimitNpcBuyItemInfo()
    : PacketHeader(0x27D9u, 0x0026u),
      user_id(0),
      account_id(0),
      charac_no(0),
      item_id(0),
      item_count(0),
      response_tag(0),
      error_code(0) {}

// IDA_EA: 0x822C53A
// IDA_NAME: _ZN22LimitNpcBuyItemInfoAllC2Ev
// DEMANGLED: LimitNpcBuyItemInfoAll::LimitNpcBuyItemInfoAll(void)
LimitNpcBuyItemInfoAll::LimitNpcBuyItemInfoAll()
    : PacketHeader(0x27DAu, 0x0182u),
      user_id(0),
      account_id(0),
      charac_no(0),
      item_count(0),
      items() {
    std::memset(items, 0, sizeof(items));
}

// IDA_EA: 0x84E9570
// IDA_NAME: _ZN21LimitNpcBuyItemUpdateC2Ev
// DEMANGLED: LimitNpcBuyItemUpdate::LimitNpcBuyItemUpdate(void)
LimitNpcBuyItemUpdate::LimitNpcBuyItemUpdate()
    : PacketHeader(0x27DDu, 0x001Au),
      item_id(0),
      charac_no(0),
      response_tag(0),
      result(0) {}
