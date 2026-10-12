#include "old_game/activity/schedule/packet_dispatcher_levelup_support_request_item.h"

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x825FC3A
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_LevelupSupportReqestItem::dispatch_sig(CUser* user,
                                                       PacketBuf& packet) {
    if (!user) {
        return 2633;
    }
    if (user->get_state() <= 2) {
        return 0;
    }

    if (GlobalData::s_event_manager->GetRepeatEvent(158)->IsEventing(0) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_LEVELUP_SUPPORT_REQ_ITEM, 1);
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_2.cpp",
            "virtual int Dispatcher_LevelupSupportReqestItem::dispatch_sig(CUser*, PacketBuf&)",
            0xa50,
            "charac_no:%d, error:%d",
            user->getCurCharacNo(),
            1);
        return 0;
    }

    int requested_level = 0;
    if (packet.get_int(requested_level) != 1) {
        return 2646;
    }

    ENUM_ERROR error = static_cast<ENUM_ERROR>(0);
    LevelupSupportEventManger* const manager =
        ARAD::Singleton<LevelupSupportEventManger>::Get();
    const int step_index = manager->GetStepIndex(requested_level);
    if (step_index == -1) {
        error = static_cast<ENUM_ERROR>(1);
    }

    // The old code deliberately indexes this vector even when step_index is
    // -1; preserve that exact order rather than adding a recovery guard.
    switch (user->levelup_support_events_[step_index].state) {
        case 1:
            manager->InsertItem(user, step_index, error);
            break;
        case 2:
            error = static_cast<ENUM_ERROR>(18);
            break;
        case 0:
            error = static_cast<ENUM_ERROR>(19);
            break;
        default:
            break;
    }

    if (static_cast<int>(error) != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_LEVELUP_SUPPORT_REQ_ITEM,
            static_cast<unsigned char>(error));
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_2.cpp",
            "virtual int Dispatcher_LevelupSupportReqestItem::dispatch_sig(CUser*, PacketBuf&)",
            0xa78,
            "charac_no:%d, request_level:%d, error:%d",
            user->getCurCharacNo(),
            requested_level,
            static_cast<int>(error));
        return 0;
    }

    PacketGuard response;
    response.clear();
    response.put_header(1, ENUM_CMDPACKET_LEVELUP_SUPPORT_REQ_ITEM);
    response.put_byte(1);
    response.put_int(requested_level);
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x82605E4
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_LevelupSupportReqestItem::read(PacketBuf&, MSG_BASE&) {
    return 0;
}

// IDA_EA: 0x82605EE
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_LevelupSupportReqestItem::process(CUser*, MSG_BASE&, ParamBase&) {
    return 0;
}

// IDA_EA: 0x82605F8
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::send(CUser*, ParamBase&)
void Dispatcher_LevelupSupportReqestItem::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8260956
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItemD2Ev
// IDA_EA: 0x8260986
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItemD0Ev
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::~Dispatcher_LevelupSupportReqestItem()
Dispatcher_LevelupSupportReqestItem::~Dispatcher_LevelupSupportReqestItem() {}

// IDA_EA: 0x8597F16
// IDA_NAME: _ZN35Dispatcher_LevelupSupportReqestItemC2Ev
// DEMANGLED: Dispatcher_LevelupSupportReqestItem::Dispatcher_LevelupSupportReqestItem(void)
Dispatcher_LevelupSupportReqestItem::Dispatcher_LevelupSupportReqestItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}
