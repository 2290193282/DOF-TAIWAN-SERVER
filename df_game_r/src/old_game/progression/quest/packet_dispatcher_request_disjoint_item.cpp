#include "old_game/progression/quest/packet_dispatcher_request_disjoint_item.h"

#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_game/progression/career/packet_dispatcher_disjoint_item.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_RequestDisjointItem::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_RequestDisjointItem::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x8596B60
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItemC2Ev
// DEMANGLED: Dispatcher_RequestDisjointItem::Dispatcher_RequestDisjointItem(void)
Dispatcher_RequestDisjointItem::Dispatcher_RequestDisjointItem()
    : IPacketDispatcher<MSG_REQUEST_DISJOINT_ITEM,
                        stRequestDisjointItemParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x8245A7E
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItemD2Ev
// DEMANGLED: Dispatcher_RequestDisjointItem::~Dispatcher_RequestDisjointItem()
// IDA_EA: 0x8245AAE
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItemD0Ev
// DEMANGLED: Dispatcher_RequestDisjointItem::~Dispatcher_RequestDisjointItem()
Dispatcher_RequestDisjointItem::~Dispatcher_RequestDisjointItem() {}

// IDA_EA: 0x81CFB28
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestDisjointItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestDisjointItem::read(PacketBuf& packet,
                                         MSG_BASE& base_message) {
    MSG_REQUEST_DISJOINT_ITEM& message =
        static_cast<MSG_REQUEST_DISJOINT_ITEM&>(base_message);
    if (packet.get_short(message.owner_unique_id) != 1) {
        return LineFunc(10733, kReadSignature, 0, 0);
    }
    if (packet.get_short(message.item_slot) != 1) {
        return LineFunc(10736, kReadSignature, 0, 0);
    }
    return packet.get_byte(message.item_space) == 1
        ? 0
        : LineFunc(10739, kReadSignature, 0, 0);
}

// IDA_EA: 0x81CFC04
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestDisjointItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestDisjointItem::check_error(
    CUser* user,
    const MSG_REQUEST_DISJOINT_ITEM& message) const {
    if (!user || user->get_state() <= 2 || !user->getCurCharacR()) return -1;
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 19)) {
        return 209;
    }
    return G_GameWorld()->find_from_world(message.owner_unique_id) ? 0 : 3;
}

// IDA_EA: 0x81CFCAC
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestDisjointItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestDisjointItem::process(CUser* user,
                                            MSG_BASE& base_message,
                                            ParamBase& base_param) {
    MSG_REQUEST_DISJOINT_ITEM& message =
        static_cast<MSG_REQUEST_DISJOINT_ITEM&>(base_message);
    stRequestDisjointItemParam& param =
        static_cast<stRequestDisjointItemParam&>(base_param);
    param.result = check_error(user, message);
    if (param.result > 0) return 0;
    if (param.result < 0) {
        return LineFunc(10783, kProcessSignature, param.result,
                        static_cast<unsigned int>(user->get_acc_id()));
    }

    CUser* const owner = G_GameWorld()->find_from_world(message.owner_unique_id);
    DisPatcher_DisJointItem::_disjoint(
        user,
        static_cast<int>(static_cast<int16_t>(message.item_slot)),
        static_cast<int>(message.item_space),
        ENUM_CMDPACKET_REQUEST_DISJOINT_ITEM,
        owner,
        0xffffu);
    return 0;
}

// IDA_EA: 0x81CFD9A
// IDA_NAME: _ZN30Dispatcher_RequestDisjointItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RequestDisjointItem::send(CUser *,ParamBase &)
void Dispatcher_RequestDisjointItem::send(CUser* user, ParamBase& base_param) {
    const stRequestDisjointItemParam& param =
        static_cast<const stRequestDisjointItemParam&>(base_param);
    if (param.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_REQUEST_DISJOINT_ITEM,
                                 static_cast<unsigned char>(param.result));
    }
}

// The seven raw identity providers below belong exclusively to CMD239's
// state-3 template specialization.
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
template class IPacketDispatcher<MSG_REQUEST_DISJOINT_ITEM,
                                stRequestDisjointItemParam,
                                static_cast<ch_state>(3)>;
