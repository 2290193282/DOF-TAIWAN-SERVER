#include "old_game/economy/mail/packet_dispatcher_mailbox_send.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "common/manager/slang_filter.h"
#include "common/manager/server_utility.h"
#include "old_game/economy/trade/trade_limit_item_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/etc/guild_parameter_script.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_space_runtime.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>
#include <string>

namespace {

const char kMailboxSendRead[] =
    "virtual int Dispatcher_MailBox_Send::read(PacketBuf&, MSG_BASE&)";
const char kMailboxSendProcess[] =
    "virtual int Dispatcher_MailBox_Send::process(CUser*, MSG_BASE&, ParamBase&)";
const char kMultiMailboxDispatch[] =
    "virtual int Dispatcher_MultiMailBoxSend::dispatch_sig(CUser*, PacketBuf&)";

bool IsLowValueMailItem(uint32_t item_id) {
    CItem* const item = G_CDataManager()->find_item(item_id);
    return item && item->is_stackable() != 1 && item->get_rarity() <= 1;
}

void QueueFactoryHubMonitor(CUser* user, const MSG_MAILBOX_SEND& request) {
    const int current_money = user->getCurCharacInvenR()->get_money();
    const int random_percent = static_cast<int>(get_rand_int(25));
    const long double threshold_factor =
        0.7L + static_cast<long double>(random_percent) * 0.01L;
    if (static_cast<unsigned int>(
            static_cast<long long>(static_cast<long double>(current_money) *
                                   threshold_factor)) > request.money) {
        return;
    }

    CStreamGuard guard(GlobalData::s_stream_pool->Acquire(
                           "PacketDispatcher_Impl_1.cpp", 9245),
                       true);
    guard << 394;
    guard << user->GetUID();
    SIG_FIND_FACTORY_HUB* const payload =
        guard.GetInBuffer<SIG_FIND_FACTORY_HUB>();
    std::memset(payload, 0, sizeof(*payload));
    payload->account_id = user->get_acc_id();
    std::strncpy(payload->charac_name,
                 request.receiver_name,
                 request.receiver_name_length);
    payload->fatigue_check =
        user->getCurCharacUsedFatigue() >= get_rand_int(20) + 120 ? 1u : 0u;
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
}

}  // namespace

// IDA_EA: 0x8256854
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x82569B0
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x82569BA
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825D98A
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::send(CUser *,ParamBase &)
// IDA_EA: 0x825D18C
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EED2Ev
// IDA_EA: 0x825D1BC
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x85985DE
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_MAILBOX_SEND18stMailboxSendParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_MAILBOX_SEND,stMailboxSendParam,(ch_state)0>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_MAILBOX_SEND,
                                 stMailboxSendParam,
                                 static_cast<ch_state>(0)>;

// IDA_EA: 0x8595ECC
// IDA_NAME: _ZN23Dispatcher_MailBox_SendC2Ev
// DEMANGLED: Dispatcher_MailBox_Send::Dispatcher_MailBox_Send(void)
Dispatcher_MailBox_Send::Dispatcher_MailBox_Send()
    : IPacketDispatcher<MSG_MAILBOX_SEND,
                        stMailboxSendParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8245FFA
// IDA_NAME: _ZN23Dispatcher_MailBox_SendD2Ev
// IDA_EA: 0x824602A
// IDA_NAME: _ZN23Dispatcher_MailBox_SendD0Ev
// DEMANGLED: Dispatcher_MailBox_Send::~Dispatcher_MailBox_Send()
Dispatcher_MailBox_Send::~Dispatcher_MailBox_Send() {}

// IDA_EA: 0x81CC3DA
// IDA_NAME: _ZN23Dispatcher_MailBox_Send4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Send::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MailBox_Send::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_MAILBOX_SEND& request = static_cast<MSG_MAILBOX_SEND&>(message);
    if (packet.get_int(request.receiver_name_length) != 1) {
        return LineFunc(8969, kMailboxSendRead, 0, 0);
    }
    if (packet.get_str(request.receiver_name,
                       sizeof(request.receiver_name),
                       request.receiver_name_length) != 1 ||
        packet.get_int(request.money) != 1 ||
        packet.get_byte(request.inventory_type) != 1 ||
        packet.get_short(request.inventory_slot) != 1 ||
        packet.get_int(request.item_id) != 1 ||
        packet.get_int(request.item_add_info) != 1) {
        return LineFunc(8981, kMailboxSendRead, 0, 0);
    }

    request.contents_length = 0u;
    request.unknown_32 = 0u;
    std::memset(request.contents, 0, sizeof(request.contents));
    if (packet.get_int(request.contents_length) != 1) {
        return LineFunc(8987, kMailboxSendRead, 0, 0);
    }
    if (packet.get_str(request.contents,
                       sizeof(request.contents),
                       request.contents_length) != 1) {
        return LineFunc(8990, kMailboxSendRead, 0, 0);
    }
    if (packet.get_int(request.unknown_32) != 1) {
        return LineFunc(8993, kMailboxSendRead, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81CC636
// IDA_NAME: _ZN23Dispatcher_MailBox_Send11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Send::check_error(CUser *,MSG_BASE &)
int Dispatcher_MailBox_Send::check_error(CUser* user, MSG_BASE& message) {
    MSG_MAILBOX_SEND& request = static_cast<MSG_MAILBOX_SEND&>(message);
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return -1;
    }
    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 4)) {
        return 209;
    }
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(63));
    if (security_error) {
        return security_error;
    }

    const int inventory_type = GetInvenTypeFromItemSpace(
        static_cast<ENUM_ITEMSPACE>(request.inventory_type));
    if (user->CheckItemLock(inventory_type, request.inventory_slot)) {
        return 213;
    }
    bool can_move = true;
    CheckItemTradeLimitCountToMoveByUser(*user,
                                         inventory_type,
                                         request.inventory_slot,
                                         can_move);
    if (!can_move) {
        return 114;
    }

    if (CSyncSlangFilter::GetInstance()->HasSlang(
            std::string(request.contents))) {
        return 159;
    }
    if (std::strlen(request.contents) != request.contents_length) {
        return -3;
    }
    if (request.item_id || request.money || request.contents[0]) {
        if (request.unknown_32) {
            const int guild_error = GuildMailCheck(user, message);
            if (guild_error) {
                return guild_error;
            }
        }
        if (user->GetReliablePerson() == 1 ||
            user->IsAvailableCurCharacTradeGoldDaily(request.money)) {
            return 0;
        }
        return 70;
    }
    return 22;
}

// IDA_EA: 0x81CC8C2
// IDA_NAME: _ZN23Dispatcher_MailBox_Send14GuildMailCheckEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_MailBox_Send::GuildMailCheck(CUser *,MSG_BASE &)
int Dispatcher_MailBox_Send::GuildMailCheck(CUser* user, MSG_BASE& message) {
    (void)message;
    if (!user->get_charac_guildkey()) {
        return 34;
    }
    if (user->IsSubGuildMaster() != 1 && user->IsGuildMaster() != 1) {
        return 36;
    }
    const int fee = G_CDataManager()->GetGuildParameterScriptForBuild()
                        ->guild_mail;
    return user->getCurCharacInvenR()->get_money() >= fee ? 0 : 35;
}

// IDA_EA: 0x81CC958
// IDA_NAME: _ZN23Dispatcher_MailBox_Send7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MailBox_Send::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MailBox_Send::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param_base) {
    MSG_MAILBOX_SEND& request = static_cast<MSG_MAILBOX_SEND&>(message);
    stMailboxSendParam& param = static_cast<stMailboxSendParam&>(param_base);
    param.result = check_error(user, message);
    if (param.result > 0) {
        return 0;
    }
    if (param.result < 0) {
        return LineFunc(9166,
                        kMailboxSendProcess,
                        param.result,
                        static_cast<unsigned int>(user->get_acc_id()));
    }

    if (request.unknown_32) {
        const int fee = G_CDataManager()->GetGuildParameterScriptForBuild()
                            ->guild_mail;
        user->getCurCharacInvenW()->use_money(
            fee, static_cast<eMoneySubReason>(22), true);
        CGuildServerProxy* const proxy =
            GlobalData::s_guild_proxy_mgr->GetServerProxy(
                static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
        proxy->SendGuildMail(static_cast<unsigned int>(user->getCurCharacNo()),
                             static_cast<unsigned int>(user->get_charac_guildkey()),
                             static_cast<int>(request.contents_length),
                             request.contents);
        return 0;
    }

    if (request.money && user->reqSendMailCertify()) {
        user->set_MailMsgData(&request);
        return 0;
    }

    MSG_CHECK_MAIL_RECEIVER check;
    check.set<MSG_CHECK_MAIL_RECEIVER::kMailboxSend>(request);
    const int db_error = WongWork::CMailBoxHelper::ReqDBCheckReceiver(
        user, request.receiver_name, &check);
    if (db_error) {
        param.result = db_error;
    } else {
        const bool low_value = request.item_id == 0 ||
                               IsLowValueMailItem(request.item_id);
        user->IncrementTradeGold(request.money, low_value ? 1 : 0);
    }
    QueueFactoryHubMonitor(user, request);
    return 0;
}

// IDA_EA: 0x81CCDB2
// IDA_NAME: _ZN23Dispatcher_MailBox_Send4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_MailBox_Send::send(CUser *,ParamBase &)
void Dispatcher_MailBox_Send::send(CUser* user, ParamBase& param_base) {
    const stMailboxSendParam& param =
        static_cast<const stMailboxSendParam&>(param_base);
    if (param.result != 0x7fffffff && param.result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MAILBOX_SEND,
                                 static_cast<unsigned char>(param.result));
    }
}

// IDA_EA: 0x85972D6
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendC2Ev
// DEMANGLED: Dispatcher_MultiMailBoxSend::Dispatcher_MultiMailBoxSend(void)
Dispatcher_MultiMailBoxSend::Dispatcher_MultiMailBoxSend()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240492
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendD2Ev
// IDA_EA: 0x82404C2
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSendD0Ev
// DEMANGLED: Dispatcher_MultiMailBoxSend::~Dispatcher_MultiMailBoxSend()
Dispatcher_MultiMailBoxSend::~Dispatcher_MultiMailBoxSend() {}

// IDA_EA: 0x82262A2
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_MultiMailBoxSend::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_MultiMailBoxSend::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (user->get_state() != 3 || !user->getCurCharacR()) {
        return 0;
    }
    const int security_error = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(64));
    if (security_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND,
                                 static_cast<unsigned char>(security_error));
        return 0;
    }
    if (user->isHackUser()) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 23);
        return 0;
    }

    MSG_MULTI_MAILBOX_SEND request;
    if (packet.get_int(request.receiver_name_length) != 1) {
        return LineFunc(57834, kMultiMailboxDispatch, 0, 0);
    }
    if (request.receiver_name_length == 0) {
        return LineFunc(57837, kMultiMailboxDispatch, 0, 0);
    }
    if (packet.get_str(request.receiver_name,
                       sizeof(request.receiver_name),
                       request.receiver_name_length) != 1) {
        return LineFunc(57840, kMultiMailboxDispatch, 0, 0);
    }
    if (packet.get_int(request.money) != 1) {
        return LineFunc(57843, kMultiMailboxDispatch, 0, 0);
    }
    request.item_count = 1u;
    if (packet.get_byte(request.item_count) != 1) {
        return LineFunc(57847, kMultiMailboxDispatch, 0, 0);
    }
    if (request.item_count > 10u) {
        return LineFunc(57850, kMultiMailboxDispatch, 0, 0);
    }

    for (uint8_t index = 0; index < request.item_count; ++index) {
        MultiMailboxSendItem& item = request.items[index];
        if (packet.get_byte(item.inventory_type) != 1) {
            return LineFunc(57855, kMultiMailboxDispatch, 0, 0);
        }
        if (packet.get_short(item.inventory_slot) != 1) {
            return LineFunc(57858, kMultiMailboxDispatch, 0, 0);
        }
        if (packet.get_int(item.item_id) != 1) {
            return LineFunc(57861, kMultiMailboxDispatch, 0, 0);
        }
        if (!item.item_id) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 17);
            return 0;
        }
        if (packet.get_int(item.item_add_info) != 1) {
            return LineFunc(57870, kMultiMailboxDispatch, 0, 0);
        }

        const int inventory_type = GetInvenTypeFromItemSpace(
            static_cast<ENUM_ITEMSPACE>(item.inventory_type));
        if (user->CheckItemLock(inventory_type, item.inventory_slot)) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 213);
            return 0;
        }
        bool can_move = true;
        CheckItemTradeLimitCountToMoveByUser(*user,
                                             inventory_type,
                                             item.inventory_slot,
                                             can_move);
        if (!can_move) {
            user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 114);
            return 0;
        }
    }

    request.contents_length = 0u;
    std::memset(request.contents, 0, sizeof(request.contents));
    if (packet.get_int(request.contents_length) != 1) {
        return LineFunc(57897, kMultiMailboxDispatch, 0, 0);
    }
    if (packet.get_str(request.contents,
                       sizeof(request.contents),
                       request.contents_length) != 1) {
        return LineFunc(57899, kMultiMailboxDispatch, 0, 0);
    }
    if (CSyncSlangFilter::GetInstance()->HasSlang(
            std::string(request.contents))) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 159);
        return 0;
    }
    if (std::strlen(request.contents) != request.contents_length) {
        return LineFunc(57911, kMultiMailboxDispatch, 0, 0);
    }
    if (user->GetReliablePerson() != 1 &&
        !user->IsAvailableCurCharacTradeGoldDaily(request.money)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND, 70);
        return 0;
    }
    if (request.money && user->reqSendMailCertify()) {
        user->set_MultimailMsgData(&request);
        return 0;
    }

    MSG_CHECK_MAIL_RECEIVER check;
    check.set<MSG_CHECK_MAIL_RECEIVER::kMultiMailboxSend>(request);
    const int db_error = WongWork::CMailBoxHelper::ReqDBCheckReceiver(
        user, request.receiver_name, &check);
    if (db_error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_MULTI_MAILBOX_SEND,
                                 static_cast<unsigned char>(db_error));
        return 0;
    }

    bool low_value = false;
    for (uint8_t index = 0; index < request.item_count; ++index) {
        const uint32_t item_id = request.items[index].item_id;
        if (!item_id || IsLowValueMailItem(item_id)) {
            low_value = true;
        }
        user->IncrementTradeGold(request.money, low_value ? 1 : 0);
    }
    return 0;
}

// IDA_EA: 0x8234226
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_MultiMailBoxSend::read(PacketBuf &,MSG_BASE &)
int Dispatcher_MultiMailBoxSend::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234230
// IDA_NAME: _ZN27Dispatcher_MultiMailBoxSend7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_MultiMailBoxSend::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_MultiMailBoxSend::process(CUser* user,
                                         MSG_BASE& message,
                                         ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
