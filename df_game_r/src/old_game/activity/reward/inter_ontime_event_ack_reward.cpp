#include "old_game/activity/reward/inter_ontime_event_ack_reward.h"

#include <cstdint>
#include <cstring>

#include "common/manager/data_manager.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/rdar_string_manager_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

uint16_t read_u16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t read_u32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84E11EC
// IDA_NAME: _ZN26Inter_OnTimeEventAckReward12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_OnTimeEventAckReward::dispatch_sig(CUser *,char *,int)
int Inter_OnTimeEventAckReward::dispatch_sig(CUser* user,
                                             char* payload,
                                             int /*payload_size*/) {
    const uint16_t result_no = read_u16(payload, 10);
    cMyTrace trace(
        "virtual int Inter_OnTimeEventAckReward::dispatch_sig(CUser*, char*, int)",
        20525,
        0);
    trace("[ON_TIME_EVENT] Request Reward Ack(result_no:%u)", result_no);

    if (!user) {
        return 0;
    }

    if (result_no != 0) {
        if (user->get_state() > 2) {
            user->SendCmdErrorPacket(
                ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                static_cast<unsigned char>(result_no));
        }
        cMyTrace error_trace(
            "virtual int Inter_OnTimeEventAckReward::dispatch_sig(CUser*, char*, int)",
            20538,
            0);
        error_trace(
            "[ON_TIME_EVENT] Request Reward Ack:err_wrong(m_id:%s,char_no:%u,result_no:%u)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            read_u32(payload, 16),
            result_no);
        return 0;
    }

    const uint32_t charac_no = read_u32(payload, 16);
    const uint32_t item_id = read_u32(payload, 20);
    const uint32_t item_count = read_u32(payload, 24);
    if ((!user->getCurCharacR() || user->get_state() <= 2) && charac_no != 0) {
        SendOnTimeRewardByMail(charac_no, item_id, item_count);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                                 17);
        cMyTrace logout_trace(
            "virtual int Inter_OnTimeEventAckReward::dispatch_sig(CUser*, char*, int)",
            20553,
            0);
        logout_trace(
            "[ON_TIME_EVENT] Request Reward Ack:err_logout(m_id:%s,char_no:%u,result_no:%u)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            charac_no,
            result_no);
        return 0;
    }

    if (charac_no != static_cast<uint32_t>(user->get_charac_no(-1))) {
        return 0;
    }

    if (user->InsertOnTimeEventReward(item_id, item_count) == 1) {
        cMyTrace success_trace(
            "virtual int Inter_OnTimeEventAckReward::dispatch_sig(CUser*, char*, int)",
            20573,
            0);
        success_trace(
            "[ON_TIME_EVENT] Request Reward Ack:success(m_id:%s,char_no:%u,result_no:%u)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            charac_no,
            result_no);
    } else {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ONTIME_EVENT_REQUEST_REWARD,
                                 17);
        cMyTrace item_trace(
            "virtual int Inter_OnTimeEventAckReward::dispatch_sig(CUser*, char*, int)",
            20570,
            0);
        item_trace(
            "[ON_TIME_EVENT] Request Reward Ack:err_item(m_id:%s,char_no:%u,result_no:%u)",
            NumberToString(static_cast<unsigned int>(user->get_acc_id()), 0),
            charac_no,
            result_no);
    }
    return 0;
}

// IDA_EA: 0x84E1578
// IDA_NAME: _ZN26Inter_OnTimeEventAckReward22SendOnTimeRewardByMailEjjj
// DEMANGLED: Inter_OnTimeEventAckReward::SendOnTimeRewardByMail(unsigned int,
//                                                                unsigned int,
//                                                                unsigned int)
int Inter_OnTimeEventAckReward::SendOnTimeRewardByMail(unsigned int charac_no,
                                                        unsigned int item_id,
                                                        unsigned int item_count) {
    CDataManager* const data_manager = G_CDataManager();
    CItem* const found_item = data_manager->find_item(static_cast<int>(item_id));
    if (!found_item) {
        return static_cast<int>(reinterpret_cast<uintptr_t>(found_item));
    }

    Inven_Item item;
    item.item_id_ = static_cast<int>(item_id);
    CItemList* const item_list = data_manager->get_item_list();
    item_list->create_item(ENUM_ITEM_CREATE_TYPE_DEFAULT,
                           item,
                           static_cast<int>(item_count));
    if (item_id == 0 && item.get_add_info() == 0) {
        return 0;
    }

    char title[21];
    const char* const title_text =
        g_scriptStringManager_.findString(4, "game_server_msg_132", 0);
    std::strncpy(title, title_text, 20);

    char body[256];
    const char* const body_text =
        g_scriptStringManager_.findString(4, "game_server_msg_134", 0);
    std::strncpy(body, body_text, 255);

    WongWork::CMailBoxHelper::ReqDBSendNewSystemMail(
        title,
        item,
        0,
        charac_no,
        body,
        static_cast<int>(std::strlen(body)),
        0,
        static_cast<ENUM_SERVER_GROUP>(0),
        false,
        false);
    return static_cast<int>(reinterpret_cast<uintptr_t>(item.reset()));
}

// IDA_EA: constructor / destructors 0x84F6FF0 and 0x84F7020.
Inter_OnTimeEventAckReward::Inter_OnTimeEventAckReward() : ISigDispatcher() {}
Inter_OnTimeEventAckReward::~Inter_OnTimeEventAckReward() {}
