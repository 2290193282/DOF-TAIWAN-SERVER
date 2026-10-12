#ifndef OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SORT_ITEM_H_
#define OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SORT_ITEM_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// CMD20 has an unconstructed 14-byte request stack object in the original
// state-3 template.  MSG_BASE's C2 is empty, and this packed definition keeps
// the payload byte at its original +0x0d address without adding state.
#pragma pack(push, 1)
struct MSG_SORT_ITEM : public MSG_BASE {
    uint8_t sort_type;  // +0x0d
};
#pragma pack(pop)

struct stOnSortItemParam : public ParamBase {
    uint8_t sort_type;  // +0x04
    int result;         // +0x08

    // IDA_EA: 0x8230E80
    // IDA_NAME: _ZN17stOnSortItemParamC2Ev
    // DEMANGLED: stOnSortItemParam::stOnSortItemParam(void)
    stOnSortItemParam();

    // IDA_EA: 0x8247DF2 / 0x8247E22
    // IDA_NAME: _ZN17stOnSortItemParamD2Ev / _ZN17stOnSortItemParamD0Ev
    // DEMANGLED: stOnSortItemParam::~stOnSortItemParam()
    virtual ~stOnSortItemParam();
};

static_assert(sizeof(MSG_SORT_ITEM) == 0x0eu,
              "CMD20 request must retain the old 14-byte stack ABI");
#if defined(__i386__)
static_assert(sizeof(stOnSortItemParam) == 0x0cu,
              "CMD20 parameter must retain ParamBase+byte+result ABI");
#endif

// CMD20.  This concrete owner remains unregistered until the complete
// PacketDispatcher root's 606-slot construction/destruction transaction has
// been reconstructed.
class DisPatcher_SortItem
    : public IPacketDispatcher<MSG_SORT_ITEM,
                               stOnSortItemParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85952D8
    // IDA_NAME: _ZN19DisPatcher_SortItemC2Ev
    // DEMANGLED: DisPatcher_SortItem::DisPatcher_SortItem(void)
    DisPatcher_SortItem();

    // IDA_EA: 0x823271C / 0x823274C
    // IDA_NAME: _ZN19DisPatcher_SortItemD2Ev / _ZN19DisPatcher_SortItemD0Ev
    // DEMANGLED: DisPatcher_SortItem::~DisPatcher_SortItem()
    virtual ~DisPatcher_SortItem();

    // IDA_EA: 0x81C9BFE / 0x81C9C54 / 0x81C9DBA
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81C9D64
    // IDA_NAME: _ZN19DisPatcher_SortItem11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: DisPatcher_SortItem::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_SortItem) == 0x04u,
              "CMD20 handler must retain its old vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_INVENTORY_PACKET_DISPATCHER_SORT_ITEM_H_
