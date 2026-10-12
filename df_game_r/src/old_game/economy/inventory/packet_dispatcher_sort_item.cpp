#include "old_game/economy/inventory/packet_dispatcher_sort_item.h"

#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kSortItemReadSignature[] =
    "virtual int DisPatcher_SortItem::read(PacketBuf&, MSG_BASE&)";
const char kSortItemCheckSignature[] =
    "int DisPatcher_SortItem::check_error(CUser*, MSG_BASE&)";

}  // namespace

// The state-3 specialization is this derived object's sole template/vtable
// provider.  It intentionally remains outside PacketDispatcher's unfinished
// 606-slot root construction table.
// IDA_EA: 0x825819C
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x82582EC
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x82582F6
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825AD00
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x82368D0 / 0x8236900
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EED2Ev /
//           _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EED0Ev
// IDA_EA: 0x85981D2
// IDA_NAME: _ZN17IPacketDispatcherI13MSG_SORT_ITEM17stOnSortItemParamL8ch_state3EEC2Ev
template class IPacketDispatcher<MSG_SORT_ITEM,
                                 stOnSortItemParam,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x8230E80
// IDA_NAME: _ZN17stOnSortItemParamC2Ev
// DEMANGLED: stOnSortItemParam::stOnSortItemParam(void)
stOnSortItemParam::stOnSortItemParam()
    : ParamBase(), sort_type(0), result(0) {}

// IDA_EA: 0x8247DF2
// IDA_NAME: _ZN17stOnSortItemParamD2Ev
// DEMANGLED: stOnSortItemParam::~stOnSortItemParam()
// IDA_EA: 0x8247E22
// IDA_NAME: _ZN17stOnSortItemParamD0Ev
// DEMANGLED: stOnSortItemParam::~stOnSortItemParam()
stOnSortItemParam::~stOnSortItemParam() {}

// IDA_EA: 0x85952D8
// IDA_NAME: _ZN19DisPatcher_SortItemC2Ev
// DEMANGLED: DisPatcher_SortItem::DisPatcher_SortItem(void)
DisPatcher_SortItem::DisPatcher_SortItem()
    : IPacketDispatcher<MSG_SORT_ITEM,
                        stOnSortItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x823271C
// IDA_NAME: _ZN19DisPatcher_SortItemD2Ev
// DEMANGLED: DisPatcher_SortItem::~DisPatcher_SortItem()
// IDA_EA: 0x823274C
// IDA_NAME: _ZN19DisPatcher_SortItemD0Ev
// DEMANGLED: DisPatcher_SortItem::~DisPatcher_SortItem()
DisPatcher_SortItem::~DisPatcher_SortItem() {}

// IDA_EA: 0x81C9BFE
// IDA_NAME: _ZN19DisPatcher_SortItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_SortItem::read(PacketBuf &,MSG_BASE &)
int DisPatcher_SortItem::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_SORT_ITEM& request = static_cast<MSG_SORT_ITEM&>(message);
    if (packet.get_byte(request.sort_type) == 1) return 0;
    return LineFunc(7446, kSortItemReadSignature, 0, 0);
}

// IDA_EA: 0x81C9C54
// IDA_NAME: _ZN19DisPatcher_SortItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_SortItem::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_SortItem::process(CUser* user,
                                 MSG_BASE& message,
                                 ParamBase& param) {
    const int error = check_error(user, message);
    if (error > 0) return error;
    if (error < 0) return -1;

    const MSG_SORT_ITEM& request = static_cast<const MSG_SORT_ITEM&>(message);
    stOnSortItemParam& result = static_cast<stOnSortItemParam&>(param);
    result.sort_type = request.sort_type;
    result.result = 0;

    switch (static_cast<signed char>(result.sort_type)) {
        case 0:
            result.result = user->getCurCharacInvenW()->sort(
                static_cast<INVEN_TYPE>(1));
            break;
        case 1:
            result.result = user->getCurCharacInvenW()->sort(
                static_cast<INVEN_TYPE>(2));
            break;
        case 2:
            user->getCurCharacCargoW()->sort();
            break;
        case 7:
            result.result = user->getCurCharacInvenW()->sort(
                static_cast<INVEN_TYPE>(3));
            break;
        default:
            break;
    }
    return 0;
}

// IDA_EA: 0x81C9D64
// IDA_NAME: _ZN19DisPatcher_SortItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_SortItem::check_error(CUser *,MSG_BASE &)
int DisPatcher_SortItem::check_error(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user) return LineFunc(7510, kSortItemCheckSignature, 0, 0);
    return user->get_state() > 2 ? 0 : -1;
}

// IDA_EA: 0x81C9DBA
// IDA_NAME: _ZN19DisPatcher_SortItem4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_SortItem::send(CUser *,ParamBase &)
void DisPatcher_SortItem::send(CUser* user, ParamBase& param) {
    const stOnSortItemParam& result =
        static_cast<const stOnSortItemParam&>(param);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_SORT_ITEM);
    if (result.result != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<char>(result.result));
    } else {
        packet.put_byte(1);
    }
    packet.put_byte(static_cast<char>(result.sort_type));
    packet.finalize(true);
    user->Send(packet);

    if (result.result != 0) return;

    user->send_itemspace(static_cast<char>(result.sort_type));
    switch (static_cast<signed char>(result.sort_type)) {
        case 0:
            user->getCurCharacInvenR()->SendItemLockListInven();
            break;
        case 1:
            user->getCurCharacInvenR()->SendItemLockListAvatar();
            break;
        case 2:
            user->getCurCharacCargoR()->SendItemLockList();
            break;
        case 7:
            user->getCurCharacInvenR()->SendItemLockListCreature();
            break;
        default:
            break;
    }
}
