#include "old_game/progression/career/packet_dispatcher_ingame_advertisement.h"

#include <cstring>

#include "common/manager/global_tools.h"
#include "common/manager/in_game_advertisement_abi.h"
#include "old_game/economy/inventory/item_list.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kReadSignature[] =
    "virtual int Dispatcher_RequestIngameAdvertisement::read(PacketBuf&, MSG_BASE&)";
const char kProcessSignature[] =
    "virtual int Dispatcher_RequestIngameAdvertisement::process(CUser*, MSG_BASE&, ParamBase&)";

}  // namespace

// IDA_EA: 0x85958C8
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisementC2Ev
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::Dispatcher_RequestIngameAdvertisement(void)
Dispatcher_RequestIngameAdvertisement::Dispatcher_RequestIngameAdvertisement()
    : IPacketDispatcher<MSG_REQUEST_INGAME_ADVERTISEMENT,
                        stRequestIngameAdvertisement,
                        static_cast<ch_state>(2)>() {}

// IDA_EA: 0x8243E76
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisementD2Ev
// IDA_EA: 0x8243EA6
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisementD0Ev
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::~Dispatcher_RequestIngameAdvertisement()
Dispatcher_RequestIngameAdvertisement::~Dispatcher_RequestIngameAdvertisement() {}

// IDA_EA: 0x81E434C
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::read(PacketBuf &,MSG_BASE &)
int Dispatcher_RequestIngameAdvertisement::read(PacketBuf& packet,
                                                 MSG_BASE& message) {
    MSG_REQUEST_INGAME_ADVERTISEMENT& request =
        static_cast<MSG_REQUEST_INGAME_ADVERTISEMENT&>(message);
    request.request_count = 0;

    // 0828_EVIDENCE: sub_70DE00 writes u8 count followed by count u16 IDs.
    // Preserve every ID in the native batch even while the corresponding
    // non-empty response records remain evidence-blocked.
    if (packet.get_byte(request.request_count) != 1) {
        return LineFunc(23568, kReadSignature, 0, 0);
    }
    for (uint16_t index = 0; index < request.request_count; ++index) {
        if (packet.get_short(request.request_ids[index]) != 1) {
            return LineFunc(23568, kReadSignature, 0, 0);
        }
    }
    return 0;
}

// IDA_EA: 0x81E4662
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::check_error(CUser *,MSG_BASE &)
int Dispatcher_RequestIngameAdvertisement::check_error(CUser* user,
                                                        MSG_BASE&) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 1) {
        return 0x7fffffff;
    }
    return 0;
}

// IDA_EA: 0x81E43A2
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_RequestIngameAdvertisement::process(CUser* user,
                                                    MSG_BASE& message,
                                                    ParamBase& param) {
    MSG_REQUEST_INGAME_ADVERTISEMENT& request =
        static_cast<MSG_REQUEST_INGAME_ADVERTISEMENT&>(message);
    stRequestIngameAdvertisement& result =
        static_cast<stRequestIngameAdvertisement&>(param);

    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(23584, kProcessSignature, result.result, 0);
    }

    (void)request;
    result.response_count = 0;
    // 0627 selected one advertisement and attached side effects to its
    // single-record reply.  0828 consumes a batch of different records; until
    // each record's fields and state transitions are recovered, report the
    // real empty list rather than execute mismatched mail/log side effects.
    return 0;
}

// IDA_EA: 0x81E46C8
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::send(CUser *,ParamBase &)
void Dispatcher_RequestIngameAdvertisement::send(CUser* user, ParamBase& param) {
    const stRequestIngameAdvertisement& result =
        static_cast<const stRequestIngameAdvertisement&>(param);
    if (result.result == 0x7fffffff) {
        return;
    }

    PacketGuard packet;
    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_REQUEST_INGAME_ADVERTISEMENT);
    if (result.result != 0) {
        packet.put_byte(0);
        packet.put_byte(static_cast<unsigned char>(result.result));
        packet.finalize(true);
        user->Send(packet);
        return;
    }

    // The generic CMD dispatcher consumes this success byte before invoking
    // sub_70D1E0.  Its first direct read is therefore the u8 record count.
    // A zero count is a complete, cursor-safe 0828 empty advertisement list.
    packet.put_byte(1);
    packet.put_byte(result.response_count);
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x81E4B6C
// IDA_NAME: _ZN37Dispatcher_RequestIngameAdvertisement24sendRewardItemSystemMailEP5CUsermi
// DEMANGLED: Dispatcher_RequestIngameAdvertisement::sendRewardItemSystemMail(CUser *,unsigned long,int)
void Dispatcher_RequestIngameAdvertisement::sendRewardItemSystemMail(
    CUser* user,
    unsigned long item_id,
    int item_count) {
    if (!user || !G_CDataManager()->find_item(static_cast<int>(item_id))) {
        return;
    }

    Inven_Item item;
    item.item_id_ = static_cast<int>(item_id);
    G_CDataManager()->get_item_list()->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, item, item_count);

    char title[30];
    char contents[256];
    std::strncpy(title,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_295", 0),
                 sizeof(title) - 1u);
    title[sizeof(title) - 1u] = '\0';
    std::strncpy(contents,
                 g_scriptStringManager_.findString(
                     4, "game_server_msg_296", 0),
                 sizeof(contents) - 1u);
    contents[sizeof(contents) - 1u] = '\0';

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        user->getCurCharacNo(),
        contents,
        static_cast<int>(std::strlen(contents)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()),
        false,
        false);
}

// IDA_EA: 0x824A1C6
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_EA: 0x824A33A
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_EA: 0x824A344
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B3FE / 0x825B42E
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EED2Ev / D0Ev
// IDA_EA: 0x825D5BE
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EE4sendEP5CUserR9ParamBase
// IDA_EA: 0x8598376
// IDA_NAME: _ZN17IPacketDispatcherI32MSG_REQUEST_INGAME_ADVERTISEMENT28stRequestIngameAdvertisementL8ch_state2EEC2Ev
template class IPacketDispatcher<MSG_REQUEST_INGAME_ADVERTISEMENT,
                                 stRequestIngameAdvertisement,
                                 static_cast<ch_state>(2)>;
