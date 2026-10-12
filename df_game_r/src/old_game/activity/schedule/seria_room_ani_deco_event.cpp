#include "old_game/activity/schedule/seria_room_ani_deco_event.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_game/economy/inventory/item.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/common/rdar_script.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/db/event/db_update_seria_room_animation_deco.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

void BuildMailText(char (&destination)[256], const char* script_key) {
    const char* const source = g_scriptStringManager_.findString(4, script_key, 0);
    std::strncpy(destination, source, 0xffu);
}

}  // namespace

// IDA_EA: 0x827B490
// IDA_NAME: _ZN21SeriaRoomAniDecoEventC2Ev
// DEMANGLED: SeriaRoomAniDecoEvent::SeriaRoomAniDecoEvent(void)
SeriaRoomAniDecoEvent::SeriaRoomAniDecoEvent() : CEventBase() {}

// IDA_EA: 0x827B4AC / 0x827B4DC
// IDA_NAME: _ZN21SeriaRoomAniDecoEventD2Ev / _ZN21SeriaRoomAniDecoEventD0Ev
// DEMANGLED: SeriaRoomAniDecoEvent::~SeriaRoomAniDecoEvent()
SeriaRoomAniDecoEvent::~SeriaRoomAniDecoEvent() {}

// IDA_EA: 0x827B4FA
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent10StartEventEv
// DEMANGLED: SeriaRoomAniDecoEvent::StartEvent(void)
void SeriaRoomAniDecoEvent::StartEvent() {
    SetEventFlag(true);
    cMyTrace trace("virtual void SeriaRoomAniDecoEvent::StartEvent()", 24, 0);
    trace("SeriaRoomAniDecoEvent Event start!");
}

// IDA_EA: 0x827B54C
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent10StartEventE10Word_Param
// DEMANGLED: SeriaRoomAniDecoEvent::StartEvent(Word_Param)
void SeriaRoomAniDecoEvent::StartEvent(Word_Param param) {
    (void)param;
    SetEventFlag(true);
    cMyTrace trace("virtual void SeriaRoomAniDecoEvent::StartEvent(Word_Param)",
                   30,
                   0);
    trace("SeriaRoomAniDecoEvent Event start! [monitor request]");
}

// IDA_EA: 0x827B59E
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent8EndEventEv
// DEMANGLED: SeriaRoomAniDecoEvent::EndEvent(void)
void SeriaRoomAniDecoEvent::EndEvent() {
    SetEventFlag(false);
    cMyTrace trace("virtual void SeriaRoomAniDecoEvent::EndEvent()", 36, 0);
    trace("SeriaRoomAniDecoEvent Event end!");
}

// IDA_EA: 0x827B5F0
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent14dailyresetDataEv
// DEMANGLED: SeriaRoomAniDecoEvent::dailyresetData(void)
void SeriaRoomAniDecoEvent::dailyresetData() {
    G_GameWorld()->ProcessAllUsers([this](CUser* user) {
        if (user && user->get_state() > 2) {
            SendAniDecoItem(*user);
        }
    });
}

// IDA_EA: 0x827B6A6
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent11StartActionEv
// DEMANGLED: SeriaRoomAniDecoEvent::StartAction(void)
void SeriaRoomAniDecoEvent::StartAction() {
    G_GameWorld()->ProcessAllUsers([this](CUser* user) {
        if (user && user->get_state() > 2) {
            SendEventInfo(*user);
        }
    });
}

// IDA_EA: 0x827B75A
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent15SendAniDecoItemER5CUser
// DEMANGLED: SeriaRoomAniDecoEvent::SendAniDecoItem(CUser &)
int SeriaRoomAniDecoEvent::SendAniDecoItem(CUser& user) {
    seriaRoom_AniDeco deco = user.getSeriaRoomAniDecoInfo();
    CDataManager* const data_manager = G_CDataManager();
    if (static_cast<int16_t>(deco.receive_flow) >=
        data_manager->maxAniDecoItem()) {
        return 0;
    }

    const int item_id =
        data_manager->getSeriaRoomAniDecoItem(deco.receive_flow);
    if (!item_id) {
        return 0;
    }

    Inven_Item reward;
    CItem* const prototype = data_manager->find_item(item_id);
    if (!prototype) {
        return 0;
    }

    const int item_index = prototype->get_index();
    prototype->vtable_->make_item(prototype, reward);
    reward.set_add_info(1);

    char mail_text[256];
    BuildMailText(mail_text, "game_server_msg_290");
    if (item_index == 0) {
        return item_id;
    }

    const char* const mail_title =
        g_scriptStringManager_.findString(4, "game_server_msg_289", 0);
    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        mail_title,
        reward,
        0,
        static_cast<unsigned int>(user.getCurCharacNo()),
        mail_text,
        static_cast<int>(std::strlen(mail_text)),
        0,
        static_cast<ENUM_SERVER_GROUP>(user.GetServerGroupToPvP()),
        false,
        false);

    ++deco.receive_flow;
    user.setSeriaRoomAniDecoInfo(deco);
    DB_UpdateSeriaRoomAnimationDeco::makeRequest(
        static_cast<unsigned int>(user.get_acc_id()),
        static_cast<short>(user.GetServerGroupToPvP()),
        user.getCurCharacNo(),
        static_cast<short>(deco.receive_flow),
        static_cast<short>(deco.deco_flow));
    return item_id;
}

// IDA_EA: 0x827B99A
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent13SendEventInfoER5CUser
// DEMANGLED: SeriaRoomAniDecoEvent::SendEventInfo(CUser &)
void SeriaRoomAniDecoEvent::SendEventInfo(CUser& user) {
    PacketGuard packet;
    packet.put_header(0, 449);
    packet.put_short(user.getSeriaRoomAniDecoInfo().deco_flow);
    packet.finalize(true);
    user.Send(packet);
}

// IDA_EA: 0x827BA48
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent14CheckEventInfoER5CUser
// DEMANGLED: SeriaRoomAniDecoEvent::CheckEventInfo(CUser &)
int SeriaRoomAniDecoEvent::CheckEventInfo(CUser& user) {
    (void)user;
    return 0;
}

// IDA_EA: 0x827BA52
// IDA_NAME: _ZN21SeriaRoomAniDecoEvent12processEventER5CUser
// DEMANGLED: SeriaRoomAniDecoEvent::processEvent(CUser &)
int SeriaRoomAniDecoEvent::processEvent(CUser& user) {
    const seriaRoom_AniDeco deco = user.getSeriaRoomAniDecoInfo();
    const int item_id =
        G_CDataManager()->getSeriaRoomAniDecoRewardItem(deco.deco_flow);
    if (!item_id) {
        return 0;
    }

    Inven_Item reward;
    CItem* const prototype = G_CDataManager()->find_item(item_id);
    if (!prototype) {
        return 0;
    }

    prototype->vtable_->make_item(prototype, reward);
    reward.set_add_info(1);

    char mail_text[256];
    BuildMailText(mail_text, "game_server_msg_291");
    if (prototype->get_index() != 0) {
        const char* const mail_title =
            g_scriptStringManager_.findString(4, "game_server_msg_289", 0);
        WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
            mail_title,
            reward,
            0,
            static_cast<unsigned int>(user.getCurCharacNo()),
            mail_text,
            static_cast<int>(std::strlen(mail_text)),
            0,
            static_cast<ENUM_SERVER_GROUP>(user.GetServerGroupToPvP()),
            false,
            false);
    }
    return 0;
}

// IDA_EA: 0x827BC4C
// IDA_NAME: _ZNK21SeriaRoomAniDecoEvent10IsEventingEP5CUser
// DEMANGLED: SeriaRoomAniDecoEvent::IsEventing(CUser *)const
int SeriaRoomAniDecoEvent::IsEventing(CUser* user) const {
    return CEventBase::IsEventing(user);
}
