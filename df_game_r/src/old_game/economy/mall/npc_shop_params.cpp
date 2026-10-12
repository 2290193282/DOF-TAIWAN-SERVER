#include "old_game/economy/mall/npc_shop_params.h"

#include <cstring>
#include <new>

#include "common/config.h"
#include "old_game/player/core/cuser.h"
#include "old_game/economy/equipment/avatar_equipment_value_types.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/shop/npc_runtime.h"
#include "old_game/social/party/party.h"
#include "old_game/economy/shop/npc_dialog_value_types.h"
#include "old_game/economy/shop/npc_limit_packets.h"
#include "old_game/activity/schedule/event_value_types.h"
#include "old_game/economy/mail/mail_trade_params.h"
#include "old_game/economy/mail/mail_trade_runtime.h"
#include "old_data/PVF/data/datamanager/need_material_data_manager.h"
#include "common/manager/data_manager.h"
#include "old_game/activity/reward/event_base.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "common/manager/global_tools.h"
#include "old_game/economy/inventory/instance_rental_system_abi.h"
#include "old_game/economy/inventory/item.h"
#include "old_game/economy/inventory/item_limit_edition.h"
#include "old_game/economy/inventory/item_list.h"
#include "common/manager/server_utility.h"
#include "common/manager/user.h"
#include "common/manager/user_history_log_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/mall/secret_shop_runtime.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/premium_runtime.h"
#include "old_game/world/region/game_world.h"
#include "old_data/PVF/data/arad_data_manager.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "common/manager/system_time.h"


class CParty;
class CDungeon;
class GameWorld;

// Original CMD465 state-3 template family.  The declarations alone do not
// own template code; this explicit instantiation restores the one canonical
// constructor/destructor/vtable provider observed in the old ELF.
// IDA_EA: 0x824BA44 / 0x824BB94 / 0x824BB9E
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// IDA_EA: 0x825B7A6 / 0x825B7D6 / 0x825D5F4 / 0x8598F9A
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EED2Ev
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EED0Ev
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EE4sendEP5CUserR9ParamBase
// IDA_NAME: _ZN17IPacketDispatcherI18MsgLimitNpcBuyItem15LimitNpcBuyItemL8ch_state3EEC2Ev
template class IPacketDispatcher<MsgLimitNpcBuyItem,
                                 LimitNpcBuyItem,
                                 static_cast<ch_state>(3)>;

typedef char MsgLimitNpcBuyItem_Size_Must_Be_0x15[
    (sizeof(MsgLimitNpcBuyItem) == 0x15) ? 1 : -1];
typedef char MSG_SECRET_SHOP_BUY_ITEM_Size_Must_Be_0x15[
    (sizeof(MSG_SECRET_SHOP_BUY_ITEM) == 0x15) ? 1 : -1];
typedef char MSG_SECRET_SHOP_OPEN_CLOSE_Size_Must_Be_0x0e[
    (sizeof(MSG_SECRET_SHOP_OPEN_CLOSE) == 0x0e) ? 1 : -1];
typedef char LimitNpcBuyItem_Size_Must_Be_0x08[
    (sizeof(LimitNpcBuyItem) == 0x08) ? 1 : -1];
typedef char STQuestShopParam_Size_Must_Be_0x08[
    (sizeof(STQuestShopParam) == 0x08) ? 1 : -1];
typedef char STQuestShopBuyParam_Size_Must_Be_0x0c[
    (sizeof(STQuestShopBuyParam) == 0x0c) ? 1 : -1];
typedef char stSecretShopOpenCloseParam_Size_Must_Be_0x08[
    (sizeof(stSecretShopOpenCloseParam) == 0x08) ? 1 : -1];
typedef char stSecretShopBuyItemParam_Size_Must_Be_0x08[
    (sizeof(stSecretShopBuyItemParam) == 0x08) ? 1 : -1];
// CLIENT_PROTOCOL_EVIDENCE: direct native enum values; no protocol mapping.
typedef char CMD_SECRET_SHOP_BUY_ITEM_Must_Be_293[
    (ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM == 293) ? 1 : -1];
typedef char CMD_SECRET_SHOP_OPEN_CLOSE_Must_Be_294[
    (ENUM_CMDPACKET_SECRET_SHOP_OPEN_CLOSE == 294) ? 1 : -1];
typedef char CMD_SHOP_COIN_EVENT_Must_Be_207[
    (ENUM_CMDPACKET_SHOP_COIN_EVENT == 207) ? 1 : -1];
typedef char CMD_LIMIT_NPC_BUY_ITEM_Must_Be_462[
    (ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM == 462) ? 1 : -1];

// IDA unit: 99_misc_global/Packet_Item_Limit_Edition_Buyable_Query.
// This is global in the original ABI; an anonymous namespace would change the
// constructor symbol to a `_GLOBAL__N_1` name.
#pragma pack(push, 1)
struct Packet_Item_Limit_Edition_Buyable_Query : public PacketHeader {
    Packet_Item_Limit_Edition_Buyable_Query();

    unsigned char body[0x1d2];
};
#pragma pack(pop)

typedef char Packet_Item_Limit_Edition_Buyable_Query_Size_Must_Be_0x1dc[
    (sizeof(Packet_Item_Limit_Edition_Buyable_Query) == 0x1dcu) ? 1 : -1];

// IDA_EA: 0x822C064
// IDA_NAME: _ZN39Packet_Item_Limit_Edition_Buyable_QueryC2Ev
// DEMANGLED: Packet_Item_Limit_Edition_Buyable_Query::Packet_Item_Limit_Edition_Buyable_Query(void)
Packet_Item_Limit_Edition_Buyable_Query::
    Packet_Item_Limit_Edition_Buyable_Query()
    : PacketHeader(0x1009u, 0x01dcu),
      body() {}

namespace {

const char kBuyCeraShopItemDispatchSig[] =
    "virtual int Dispatcher_BuyCeraShopItem::dispatch_sig(CUser*, PacketBuf&)";
const char kBuyAutomatItemDispatchSig[] =
    "virtual int Dispatcher_BuyAutomatItem::dispatch_sig(CUser*, PacketBuf&)";
const char kBuyCeraShopLimitItemDispatchSig[] =
    "virtual int Dispatcher_BuyCeraShopLimitItem::dispatch_sig(CUser*, PacketBuf&)";

CItemList* ItemListFromDataManager(CDataManager* manager) {
    return manager->get_item_list();
}

int ReadIntLE(const unsigned char* data) {
    return *reinterpret_cast<const int*>(data);
}

unsigned int ReadUIntLE(const unsigned char* data) {
    return *reinterpret_cast<const unsigned int*>(data);
}

unsigned short ReadUShortLE(const unsigned char* data) {
    return *reinterpret_cast<const unsigned short*>(data);
}

void WriteIntLE(unsigned char* data, int value) {
    *reinterpret_cast<int*>(data) = value;
}

void WriteUIntLE(unsigned char* data, unsigned int value) {
    *reinterpret_cast<unsigned int*>(data) = value;
}

void WriteUShortLE(unsigned char* data, unsigned short value) {
    *reinterpret_cast<unsigned short*>(data) = value;
}

bool IsAvatarItem(const void* item) {
    if (!item) {
        return false;
    }
    void** vtable = *reinterpret_cast<void***>(const_cast<void*>(item));
    typedef unsigned char (*IsAvatarFn)(const void*);
    IsAvatarFn is_avatar = reinterpret_cast<IsAvatarFn>(vtable[4]);
    return static_cast<unsigned char>(is_avatar(item)) != 0;
}

}  // namespace

// Old-server local data object at 0x8C101C0.
// IDA_NAME: _ZL19INVITATION_PAPER_NO
static int INVITATION_PAPER_NO[7] = {
    3213, 3214, 3215, 3219, 3256, 3280, 3282,
};

// IDA_EA: 0x82866D0
// IDA_NAME: _ZN33CHackLog_InvitationPaperSellToNPC17IsInvitationPaperEi
// DEMANGLED: CHackLog_InvitationPaperSellToNPC::IsInvitationPaper(int)
int CHackLog_InvitationPaperSellToNPC::IsInvitationPaper(int item_id) {
    for (unsigned int index = 0; index < 7u; ++index) {
        if (INVITATION_PAPER_NO[index] == item_id) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x8286714
// IDA_NAME: _ZN33CHackLog_InvitationPaperSellToNPC11GetHackTypeEv
// DEMANGLED: CHackLog_InvitationPaperSellToNPC::GetHackType(void)
int CHackLog_InvitationPaperSellToNPC::GetHackType() {
    return 802;
}

// IDA_EA: 0x8231AC0
// IDA_NAME: _ZN15LimitNpcBuyItemC2Ev
// DEMANGLED: LimitNpcBuyItem::LimitNpcBuyItem(void)
LimitNpcBuyItem::LimitNpcBuyItem()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8246982
// IDA_NAME: _ZN15LimitNpcBuyItemD2Ev
// DEMANGLED: LimitNpcBuyItem::~LimitNpcBuyItem()
LimitNpcBuyItem::~LimitNpcBuyItem() {}

// IDA_EA: 0x82469B2
// IDA_NAME: _ZN15LimitNpcBuyItemD0Ev
// DEMANGLED: LimitNpcBuyItem::~LimitNpcBuyItem() deleting destructor
void LimitNpcBuyItem_deleting_destructor(LimitNpcBuyItem* self) {
    if (self) {
        self->~LimitNpcBuyItem();
        operator delete(self);
    }
}

// IDA_EA: 0x82316AA
// IDA_NAME: _ZN16STQuestShopParamC2Ev
// DEMANGLED: STQuestShopParam::STQuestShopParam(void)
STQuestShopParam::STQuestShopParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8246E68
// IDA_NAME: _ZN16STQuestShopParamD2Ev
// DEMANGLED: STQuestShopParam::~STQuestShopParam()
STQuestShopParam::~STQuestShopParam() {}

// IDA_EA: 0x8246E98
// IDA_NAME: _ZN16STQuestShopParamD0Ev
// DEMANGLED: STQuestShopParam::~STQuestShopParam() deleting destructor
void STQuestShopParam_deleting_destructor(STQuestShopParam* self) {
    if (self) {
        self->~STQuestShopParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8231674
// IDA_NAME: _ZN19STQuestShopBuyParamC2Ev
// DEMANGLED: STQuestShopBuyParam::STQuestShopBuyParam(void)
STQuestShopBuyParam::STQuestShopBuyParam()
    : ParamBase(),
      status_id(0),
      buying_count(0),
      result(0) {}

// IDA_EA: 0x8246EB6
// IDA_NAME: _ZN19STQuestShopBuyParamD2Ev
// DEMANGLED: STQuestShopBuyParam::~STQuestShopBuyParam()
STQuestShopBuyParam::~STQuestShopBuyParam() {}

// IDA_EA: 0x8246EE6
// IDA_NAME: _ZN19STQuestShopBuyParamD0Ev
// DEMANGLED: STQuestShopBuyParam::~STQuestShopBuyParam() deleting destructor
void STQuestShopBuyParam_deleting_destructor(STQuestShopBuyParam* self) {
    if (self) {
        self->~STQuestShopBuyParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82314DA
// IDA_NAME: _ZN26stSecretShopOpenCloseParamC2Ev
// DEMANGLED: stSecretShopOpenCloseParam::stSecretShopOpenCloseParam(void)
stSecretShopOpenCloseParam::stSecretShopOpenCloseParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82471C2
// IDA_NAME: _ZN26stSecretShopOpenCloseParamD2Ev
// DEMANGLED: stSecretShopOpenCloseParam::~stSecretShopOpenCloseParam()
stSecretShopOpenCloseParam::~stSecretShopOpenCloseParam() {}

// IDA_EA: 0x82471F2
// IDA_NAME: _ZN26stSecretShopOpenCloseParamD0Ev
// DEMANGLED: stSecretShopOpenCloseParam::~stSecretShopOpenCloseParam() deleting destructor
void stSecretShopOpenCloseParam_deleting_destructor(
    stSecretShopOpenCloseParam* self) {
    if (self) {
        self->~stSecretShopOpenCloseParam();
        operator delete(self);
    }
}

// IDA_EA: 0x82314B4
// IDA_NAME: _ZN24stSecretShopBuyItemParamC2Ev
// DEMANGLED: stSecretShopBuyItemParam::stSecretShopBuyItemParam(void)
stSecretShopBuyItemParam::stSecretShopBuyItemParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8247210
// IDA_NAME: _ZN24stSecretShopBuyItemParamD2Ev
// DEMANGLED: stSecretShopBuyItemParam::~stSecretShopBuyItemParam()
stSecretShopBuyItemParam::~stSecretShopBuyItemParam() {}

// IDA_EA: 0x8247240
// IDA_NAME: _ZN24stSecretShopBuyItemParamD0Ev
// DEMANGLED: stSecretShopBuyItemParam::~stSecretShopBuyItemParam() deleting destructor
void stSecretShopBuyItemParam_deleting_destructor(
    stSecretShopBuyItemParam* self) {
    if (self) {
        self->~stSecretShopBuyItemParam();
        operator delete(self);
    }
}

// IDA_EA: 0x81D25F8
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecretShopBuyItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecretShopBuyItem::read(PacketBuf& packet,
                                       MSG_BASE& message) {
    MSG_SECRET_SHOP_BUY_ITEM& request =
        static_cast<MSG_SECRET_SHOP_BUY_ITEM&>(message);
    if (packet.get_int(request.shop_item_index) != 1) {
        return LineFunc(
            12160,
            "virtual int Dispatcher_SecretShopBuyItem::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(request.buy_count) != 1) {
        return LineFunc(
            12161,
            "virtual int Dispatcher_SecretShopBuyItem::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (request.buy_count > 0) {
        return 0;
    }
    return LineFunc(
        12164,
        "virtual int Dispatcher_SecretShopBuyItem::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81D26C0
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItem11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecretShopBuyItem::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecretShopBuyItem::check_error(CUser* user,
                                              MSG_BASE& message,
                                              ParamBase& param) {
    (void)message;
    (void)param;
    return user->get_state() == 5 ? 0 : 0x7FFFFFFF;
}

// IDA_EA: 0x81D26EA
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecretShopBuyItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecretShopBuyItem::process(CUser* user,
                                          MSG_BASE& message,
                                          ParamBase& param) {
    stSecretShopBuyItemParam& buy_param =
        reinterpret_cast<stSecretShopBuyItemParam&>(param);
    buy_param.result = check_error(user, message, param);
    if (buy_param.result > 0) {
        return 0;
    }
    if (buy_param.result < 0) {
        return LineFunc(
            12193,
            "virtual int Dispatcher_SecretShopBuyItem::process(CUser*, MSG_BASE&, ParamBase&)",
            buy_param.result,
            0);
    }

    secretshop::SECRET_SHOP_DATA* secret_shop_data =
        user->GetSecretShopData();
    secretshop::SECRET_SHOP_INFO* user_secret_shop =
        user->GetUserSecretShop();
    if (!secret_shop_data || !user_secret_shop) {
        buy_param.result = 0x7FFFFFFF;
        return 0;
    }
    if (static_cast<unsigned char>(
            secret_shop_data->IsOpen())) {
        const MSG_SECRET_SHOP_BUY_ITEM& request =
            static_cast<const MSG_SECRET_SHOP_BUY_ITEM&>(message);
        GlobalData::s_secret_shop->BuyItem(
            user,
            *user_secret_shop,
            static_cast<int>(request.shop_item_index),
            request.buy_count);
    }
    return 0;
}

// IDA_EA: 0x81D27EC
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SecretShopBuyItem::send(CUser *,ParamBase &)
void Dispatcher_SecretShopBuyItem::send(CUser* user, ParamBase& param) {
    stSecretShopBuyItemParam& buy_param =
        reinterpret_cast<stSecretShopBuyItemParam&>(param);
    if (buy_param.result != 0x7FFFFFFF && buy_param.result) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_SECRET_SHOP_BUY_ITEM,
            static_cast<unsigned char>(buy_param.result));
    }
}

// IDA_EA: 0x824563A
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItemD2Ev
// DEMANGLED: Dispatcher_SecretShopBuyItem::~Dispatcher_SecretShopBuyItem()
// IDA_EA: 0x824566A
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItemD0Ev
// DEMANGLED: Dispatcher_SecretShopBuyItem::~Dispatcher_SecretShopBuyItem()
Dispatcher_SecretShopBuyItem::~Dispatcher_SecretShopBuyItem() {}

// IDA_EA: 0x85970A0
// IDA_NAME: _ZN28Dispatcher_SecretShopBuyItemC2Ev
// DEMANGLED: Dispatcher_SecretShopBuyItem::Dispatcher_SecretShopBuyItem(void)
Dispatcher_SecretShopBuyItem::Dispatcher_SecretShopBuyItem()
    : IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                        stSecretShopBuyItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81D2834
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenClose4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_SecretShopOpenClose::read(PacketBuf &,MSG_BASE &)
int Dispatcher_SecretShopOpenClose::read(PacketBuf& packet,
                                         MSG_BASE& message) {
    MSG_SECRET_SHOP_OPEN_CLOSE& request =
        static_cast<MSG_SECRET_SHOP_OPEN_CLOSE&>(message);
    if (packet.get_byte(request.buying) == 1) {
        return 0;
    }
    return LineFunc(
        12228,
        "virtual int Dispatcher_SecretShopOpenClose::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81D288A
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenClose11check_errorEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecretShopOpenClose::check_error(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecretShopOpenClose::check_error(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)message;
    (void)param;
    return user->get_state() == 5 ? 0 : 0x7FFFFFFF;
}

// IDA_EA: 0x81D28B4
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenClose7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_SecretShopOpenClose::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_SecretShopOpenClose::process(CUser* user,
                                            MSG_BASE& message,
                                            ParamBase& param) {
    stSecretShopOpenCloseParam& open_close_param =
        reinterpret_cast<stSecretShopOpenCloseParam&>(param);
    open_close_param.result = check_error(user, message, param);
    if (open_close_param.result > 0) {
        return 0;
    }
    if (open_close_param.result < 0) {
        return LineFunc(
            12252,
            "virtual int Dispatcher_SecretShopOpenClose::process(CUser*, MSG_BASE&, ParamBase&)",
            open_close_param.result,
            0);
    }

    secretshop::SECRET_SHOP_DATA* secret_shop_data =
        user->GetSecretShopData();
    if (secret_shop_data) {
        const MSG_SECRET_SHOP_OPEN_CLOSE& request =
            static_cast<const MSG_SECRET_SHOP_OPEN_CLOSE&>(message);
        const bool buying = request.buying != 0;
        const int user_pos = user->GetUserPosInParty();
        secret_shop_data->SetBuying(user_pos, buying);
    } else {
        open_close_param.result = 0x7FFFFFFF;
    }
    return 0;
}

// IDA_EA: 0x81D2992
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenClose4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_SecretShopOpenClose::send(CUser *,ParamBase &)
void Dispatcher_SecretShopOpenClose::send(CUser* user, ParamBase& param) {
    (void)user;
    (void)param;
}

// IDA_EA: 0x82455EC
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenCloseD2Ev
// DEMANGLED: Dispatcher_SecretShopOpenClose::~Dispatcher_SecretShopOpenClose()
// IDA_EA: 0x824561C
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenCloseD0Ev
// DEMANGLED: Dispatcher_SecretShopOpenClose::~Dispatcher_SecretShopOpenClose()
Dispatcher_SecretShopOpenClose::~Dispatcher_SecretShopOpenClose() {}

// IDA_EA: 0x85970BC
// IDA_NAME: _ZN30Dispatcher_SecretShopOpenCloseC2Ev
// DEMANGLED: Dispatcher_SecretShopOpenClose::Dispatcher_SecretShopOpenClose(void)
Dispatcher_SecretShopOpenClose::Dispatcher_SecretShopOpenClose()
    : IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                        stSecretShopOpenCloseParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x81E0F60
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_LimitNpcBuyItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_LimitNpcBuyItem::read(PacketBuf& packet, MSG_BASE& message) {
    MsgLimitNpcBuyItem& request =
        static_cast<MsgLimitNpcBuyItem&>(message);
    if (packet.get_int(request.item_id) != 1) {
        return LineFunc(
            21914,
            "virtual int Dispatcher_LimitNpcBuyItem::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_int(request.npc_id) == 1) {
        return 0;
    }
    return LineFunc(
        21917,
        "virtual int Dispatcher_LimitNpcBuyItem::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81E0FF8
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_LimitNpcBuyItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_LimitNpcBuyItem::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    LimitNpcBuyItem& limit_param = reinterpret_cast<LimitNpcBuyItem&>(param);
    limit_param.result = check_error(user, message);
    if (limit_param.result > 0) {
        return 0;
    }
    if (limit_param.result < 0) {
        return LineFunc(
            21934,
            "virtual int Dispatcher_LimitNpcBuyItem::process(CUser*, MSG_BASE&, ParamBase&)",
            limit_param.result,
            0);
    }

    const MsgLimitNpcBuyItem& request =
        static_cast<const MsgLimitNpcBuyItem&>(message);
    LimitNpcBuyItemInfo packet;
    packet.user_id = static_cast<uint32_t>(user->GetUID());
    packet.account_id = static_cast<uint32_t>(user->get_acc_id());
    packet.charac_no = static_cast<uint32_t>(user->get_charac_no(-1));
    packet.item_id = request.item_id;
    packet.item_count = 1;
    packet.response_tag = 0;
    packet.error_code = 0;

    CMonitorServerProxy* proxy =
        ::GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&packet),
                      static_cast<int>(sizeof(packet)));
    return 0;
}

// IDA_EA: 0x81E1100
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItem4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_LimitNpcBuyItem::send(CUser *,ParamBase &)
void Dispatcher_LimitNpcBuyItem::send(CUser* user, ParamBase& param) {
    LimitNpcBuyItem& limit_param = reinterpret_cast<LimitNpcBuyItem&>(param);
    PacketGuard packet;
    if (limit_param.result > 0) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_LIMIT_NPC_BUY_ITEM,
                                 static_cast<unsigned char>(
                                     limit_param.result));
    }
}

// IDA_EA: 0x81E1172
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItem11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_LimitNpcBuyItem::check_error(CUser *,MSG_BASE &)
int Dispatcher_LimitNpcBuyItem::check_error(CUser* user,
                                            MSG_BASE& message) {
    if (!user) {
        return -1;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return -1;
    }

    const MsgLimitNpcBuyItem& request =
        static_cast<const MsgLimitNpcBuyItem&>(message);
    const unsigned int item_id = request.item_id;
    const unsigned int npc_id = request.npc_id;
    CDataManager* data_manager = G_CDataManager();
    if (static_cast<unsigned char>(
            data_manager->checkLimitNpcBuyItemList(npc_id, item_id)) != 1) {
        const char* acc_id_text =
            NumberToString(static_cast<unsigned int>(
                               user->get_acc_id()),
                           0);
        cMyTrace trace(
            "int Dispatcher_LimitNpcBuyItem::check_error(CUser*, MSG_BASE&)",
            21985,
            5);
        trace("May be HackUser or Must be HackUser : m_id=%s, charac_no=%u",
              acc_id_text,
              static_cast<unsigned int>(user->get_charac_no(-1)));
        return -1;
    }

    if (static_cast<unsigned char>(user->CheckInTrade())) {
        return 19;
    }

    private_store::CPrivateStoreMgr* private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (static_cast<unsigned char>(
            private_store_manager->IsBusyPrivateStore(user))) {
        return 60;
    }

    CItem* item = data_manager->find_item(static_cast<int>(item_id));
    if (!item) {
        return 17;
    }

    Inven_Item material_item;
    NeedMaterialDataManager* material_manager =
        NeedMaterialDataManager::getInstance();
    const NeedMaterialDataManager::NeedMaterialPair* need_material =
        material_manager->getNeedMaterial(item);
    if (!need_material || !need_material->first ||
        need_material->first == -1) {
        return 23;
    }

    CInventory* inventory = user->getCurCharacInvenRefR();
    if (inventory->GetInvenData(need_material->first, material_item) < 0) {
        return 21;
    }
    if (need_material->second <= 0) {
        return 22;
    }
    if (need_material->second > material_item.get_add_info()) {
        return 22;
    }

    Inven_Item target_item;
    target_item.item_id_ = static_cast<int32_t>(item_id);
    ItemListFromDataManager(data_manager)->create_item(
        ENUM_ITEM_CREATE_TYPE_DEFAULT, target_item, 1);
    if (inventory->tryInsertItemIntoInventory(target_item) >= 0) {
        return 0;
    }
    return 4;
}

// IDA_EA: 0x8597B42
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItemC2Ev
// DEMANGLED: Dispatcher_LimitNpcBuyItem::Dispatcher_LimitNpcBuyItem(void)
Dispatcher_LimitNpcBuyItem::Dispatcher_LimitNpcBuyItem()
    : IPacketDispatcher<MsgLimitNpcBuyItem,
                        LimitNpcBuyItem,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x82442BA
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItemD2Ev
// DEMANGLED: Dispatcher_LimitNpcBuyItem::~Dispatcher_LimitNpcBuyItem()
// IDA_EA: 0x82442EA
// IDA_NAME: _ZN26Dispatcher_LimitNpcBuyItemD0Ev
// DEMANGLED: Dispatcher_LimitNpcBuyItem::~Dispatcher_LimitNpcBuyItem()
Dispatcher_LimitNpcBuyItem::~Dispatcher_LimitNpcBuyItem() {}

// IDA_EA: 0x81EA622
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_Event12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_Shop_Coin_Event::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_Shop_Coin_Event::dispatch_sig(CUser* user,
                                             PacketBuf& packet) {
    (void)packet;
    if (user->get_state() > 2) {
        unsigned int coin_no_per_day = 1;
        CCoinEventPerDay* repeat_event = static_cast<CCoinEventPerDay*>(
            GlobalData::s_event_manager
                ? GlobalData::s_event_manager->GetRepeatEvent(4)
                : 0);
        if (repeat_event &&
            static_cast<unsigned char>(repeat_event->IsEventing(0))) {
            coin_no_per_day =
                static_cast<unsigned int>(repeat_event->GetCoinNoPerDay(1));
        }
        if (static_cast<unsigned char>(user->IsHavePremiumAdvantage())) {
            WongWork::CUserPremium* premium = user->GetPremiumInfo();
            coin_no_per_day +=
                static_cast<unsigned int>(
                    premium->GetAdvantageCoin());
        }

        CInventory* read_inventory =
            static_cast<CInventory*>(user->getCurCharacInvenR());
        if (read_inventory->GetCoin() >=
                static_cast<int>(coin_no_per_day) ||
            static_cast<unsigned char>(user->GetOpencoin()) == 1) {
            PacketGuard packet_guard;
            packet_guard.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                    ENUM_CMDPACKET_SHOP_COIN_EVENT);
            packet_guard.put_byte(1);
            packet_guard.put_byte(0);
            packet_guard.finalize(true);
            user->Send(packet_guard);
            user->SetOpencoin(1);
        } else {
            CInventory* read_inventory_again =
                static_cast<CInventory*>(
                    user->getCurCharacInvenR());
            const int add_count =
                static_cast<int>(coin_no_per_day) -
                read_inventory_again->GetCoin();
            CInventory* write_inventory =
                static_cast<CInventory*>(
                    user->getCurCharacInvenW());
            write_inventory->SetCoin(coin_no_per_day);
            user->SetOpencoin(1);
            CInventory* final_inventory =
                static_cast<CInventory*>(
                    user->getCurCharacInvenR());
            const int coin = final_inventory->GetCoin();
            CUser_history_log(user)->CoinAdd(
                coin, add_count, static_cast<eCoinAddReason>(4));

            PacketGuard packet_guard;
            packet_guard.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                    ENUM_CMDPACKET_SHOP_COIN_EVENT);
            packet_guard.put_byte(1);
            packet_guard.put_byte(add_count);
            packet_guard.finalize(true);
            user->Send(packet_guard);
            user->SendCoin();
        }
    }
    return 0;
}

// IDA_EA: 0x8233BBE
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_Event4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_Shop_Coin_Event::read(PacketBuf &,MSG_BASE &)
int Dispatcher_Shop_Coin_Event::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233BC8
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_Event7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_Shop_Coin_Event::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_Shop_Coin_Event::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596930
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_EventC2Ev
// DEMANGLED: Dispatcher_Shop_Coin_Event::Dispatcher_Shop_Coin_Event(void)
Dispatcher_Shop_Coin_Event::Dispatcher_Shop_Coin_Event()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243948
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_EventD2Ev
// DEMANGLED: Dispatcher_Shop_Coin_Event::~Dispatcher_Shop_Coin_Event()
// IDA_EA: 0x8243978
// IDA_NAME: _ZN26Dispatcher_Shop_Coin_EventD0Ev
// DEMANGLED: Dispatcher_Shop_Coin_Event::~Dispatcher_Shop_Coin_Event()
Dispatcher_Shop_Coin_Event::~Dispatcher_Shop_Coin_Event() {}

// IDA_EA: 0x8223BB6
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfo12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_DungeonNPCBuffInfo::dispatch_sig(CUser* user,
                                                PacketBuf& packet) {
    (void)user;
    (void)packet;
    return 0;
}

// IDA_EA: 0x8234046
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfo4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::read(PacketBuf &,MSG_BASE &)
int Dispatcher_DungeonNPCBuffInfo::read(PacketBuf& packet,
                                        MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8234050
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfo7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_DungeonNPCBuffInfo::process(CUser* user,
                                           MSG_BASE& message,
                                           ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596EFC
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfoC2Ev
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::Dispatcher_DungeonNPCBuffInfo(void)
Dispatcher_DungeonNPCBuffInfo::Dispatcher_DungeonNPCBuffInfo()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8240BE2
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfoD2Ev
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::~Dispatcher_DungeonNPCBuffInfo()
// IDA_EA: 0x8240C12
// IDA_NAME: _ZN29Dispatcher_DungeonNPCBuffInfoD0Ev
// DEMANGLED: Dispatcher_DungeonNPCBuffInfo::~Dispatcher_DungeonNPCBuffInfo()
Dispatcher_DungeonNPCBuffInfo::~Dispatcher_DungeonNPCBuffInfo() {}

// IDA_EA: 0x81FD396
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BuyCeraShopItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BuyCeraShopItem::dispatch_sig(CUser* user,
                                             PacketBuf& packet) {
    if (!user || static_cast<unsigned char>(user->CheckInTrade())) {
        return 0;
    }
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(36731, kBuyCeraShopItemDispatchSig, 0, 0);
    }

    PacketGuard packet_guard;
    private_store::CPrivateStoreMgr* private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (static_cast<unsigned char>(
            private_store_manager->IsBusyPrivateStore(user))) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            0x3cu,
            0,
            0,
            1,
            1,
            0);
        return 0;
    }
    if (static_cast<unsigned char>(user->CheckInTrade())) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            0x13u,
            0,
            0,
            1,
            1,
            0);
        return 0;
    }

    MSG_BUY_CERASHOP_ITEM message;
    std::memset(&message, 0, sizeof(message));
    // The packet has a recovered named layout; do not regress this route to
    // byte-offset access when adding Cera-shop validation.
    char receiver[38];
    std::memset(receiver, 0, sizeof(receiver));

    unsigned short buy_type = 0;
    if (packet.get_short(buy_type) != 1) {
        return LineFunc(36777, kBuyCeraShopItemDispatchSig, 0, 0);
    }
    message.buy_type = buy_type;
    if (buy_type >= 2u) {
        return LineFunc(36781, kBuyCeraShopItemDispatchSig, 0, 0);
    }

    unsigned char gift = 0;
    if (packet.get_byte(gift) != 1) {
        return LineFunc(36791, kBuyCeraShopItemDispatchSig, 0, 0);
    }
    message.gift = gift;

    if (gift == 1) {
        int receiver_len = 0;
        if (packet.get_int(receiver_len) != 1) {
            return LineFunc(36795, kBuyCeraShopItemDispatchSig, 0, 0);
        }
        message.receiver_name_length = receiver_len;
        if (packet.get_str(receiver, 30, receiver_len) != 1) {
            return LineFunc(36796, kBuyCeraShopItemDispatchSig, 0, 0);
        }
        std::memcpy(message.receiver_name, receiver,
                    sizeof(message.receiver_name));
    }
    if (gift == 1 && buy_type == 1) {
        return LineFunc(36802, kBuyCeraShopItemDispatchSig, 0, 0);
    }

    if (G_GameWorld()->IsIntegratedPvPBaseChannel() &&
        (G_GameWorld()->GetChannelType() != 15 || gift == 1)) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            0x3cu,
            0,
            0,
            1,
            1,
            0);
        return 0;
    }

    {
        const char* account_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace(kBuyCeraShopItemDispatchSig, 36868, 0);
        trace(
            "NEXON BILLING : BuyCeraShopItem m_id(%s), charac_no(%d), charac_name(%s), buytype(%d)",
            account_id_text,
            user->getCurCharacNo(),
            user->getCurCharacName(),
            buy_type);
    }

    unsigned char item_count = 0;
    if (packet.get_byte(item_count) != 1 || item_count == 0 ||
        item_count > 0x23u) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            1u,
            0,
            message.items[0].goods_no,
            1,
            1,
            0);
        return 0;
    }
    message.item_count = item_count;

    for (int i = 0; i < item_count; ++i) {
        CeraShopBuyItem& item_msg = message.items[i];
        if (packet.get_byte(item_msg.select_type) != 1) {
            return LineFunc(36887, kBuyCeraShopItemDispatchSig, 0, 0);
        }
        if (packet.get_byte(item_msg.avatar_select) != 1) {
            return LineFunc(36888, kBuyCeraShopItemDispatchSig, 0, 0);
        }
        unsigned int product_no = 0;
        if (packet.get_int(product_no) != 1) {
            return LineFunc(36889, kBuyCeraShopItemDispatchSig, 0, 0);
        }
        item_msg.goods_no = product_no;

        unsigned char option_count = 0;
        if (packet.get_byte(option_count) != 1 || option_count > 0x23u) {
            ::GlobalData::s_pCeraShop->ProcessError(
                user,
                1u,
                0,
                message.items[0].goods_no,
                1,
                1,
                0);
            return 0;
        }
        item_msg.option_count = option_count;

        if (item_msg.select_type == 0xffu && option_count == 0) {
            CCeraShopGoods* goods = G_CDataManager()->FindGoods(product_no);
            if (!goods) {
                return LineFunc(36914, kBuyCeraShopItemDispatchSig, 0, 0);
            }
            CItem* item = G_CDataManager()->find_item(goods->GetItemIdx());
            if (item && IsAvatarItem(item)) {
                const CEquipItem* const avatar =
                    reinterpret_cast<const CEquipItem*>(item);
                const int avatar_option_size = avatar->getAvatarOptionSize();
                const int option_key = avatar_option_size > 0
                    ? avatar->getAvatarOptionKeyByOrdinal(
                          static_cast<int>(server_utility::get_rand_int(
                              avatar_option_size)))
                    : -1;
                if (option_key >= 0 && option_key <= 0xff) {
                    item_msg.select_type =
                        static_cast<unsigned char>(option_key);
                }
            }
        }

        for (int j = 0; j < option_count; ++j) {
            unsigned int option_item = 0;
            if (packet.get_int(option_item) != 1) {
                return LineFunc(36933, kBuyCeraShopItemDispatchSig, 0, 0);
            }
            item_msg.options[j].item_id = option_item;
            if (packet.get_byte(item_msg.options[j].select) != 1) {
                return LineFunc(36934, kBuyCeraShopItemDispatchSig, 0, 0);
            }
        }

        unsigned char add_item_count = 0;
        if (packet.get_byte(add_item_count) != 1 ||
            add_item_count > 0x23u) {
            ::GlobalData::s_pCeraShop->ProcessError(
                user,
                1u,
                0,
                message.items[0].goods_no,
                1,
                1,
                0);
            return 0;
        }
        item_msg.additional_item_count = add_item_count;
        for (int k = 0; k < add_item_count; ++k) {
            unsigned long item_idx = 0;
            if (packet.get_item_idx(item_idx) != 1) {
                return LineFunc(36947, kBuyCeraShopItemDispatchSig, 0, 0);
            }
            item_msg.additional_items[k].item_id =
                static_cast<unsigned int>(item_idx);
            unsigned short first_short = 0;
            if (packet.get_short(first_short) != 1) {
                return LineFunc(36948, kBuyCeraShopItemDispatchSig, 0, 0);
            }
            item_msg.additional_items[k].first_option = first_short;
            unsigned short second_short = 0;
            if (packet.get_short(second_short) != 1) {
                return LineFunc(36949, kBuyCeraShopItemDispatchSig, 0, 0);
            }
            item_msg.additional_items[k].second_option = second_short;
        }

        for (int m = 0; m < add_item_count; ++m) {
            for (int n = m + 1; n < add_item_count; ++n) {
                if (item_msg.additional_items[m].item_id ==
                        item_msg.additional_items[n].item_id &&
                    item_msg.additional_items[m].first_option ==
                        item_msg.additional_items[n].first_option) {
                    return LineFunc(36958,
                                    kBuyCeraShopItemDispatchSig,
                                    0,
                                    0);
                }
            }
        }

        private_store::CPrivateStore* store_seller =
            private_store::GetInstancePrivateStoreMgr()->FindStoreSeller(user);
        if (store_seller &&
            static_cast<unsigned char>(
                store_seller->isDollCreated()) &&
            product_no == 117421u) {
            ::GlobalData::s_pCeraShop->ProcessError(
                user,
                0xe7u,
                0,
                message.items[0].goods_no,
                1,
                1,
                0);
            return 0;
        }

        const char* account_id_text =
            NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                           0);
        cMyTrace trace(kBuyCeraShopItemDispatchSig, 36976, 0);
        trace(
            "NEXON BILLING : BuyCeraShopItem m_id(%s), charac_no(%d), charac_name(%s), buytype(%d), product_no(%d)",
            account_id_text,
            user->getCurCharacNo(),
            user->getCurCharacName(),
            buy_type,
            product_no);
    }

    ServiceRestrictManager* restrict_manager =
        ARAD::Singleton<ServiceRestrictManager>::Get();
    if (static_cast<unsigned char>(restrict_manager->isRestricted(
            user,
            static_cast<RestrictCategory::Enum>(1),
            17))) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            0xd1u,
            0,
            message.items[0].goods_no,
            1,
            1,
            gift);
        return 0;
    }

    const int security_error = (::GlobalData::s_pSecuProtectionField)->Check(
        user,
        static_cast<SECURITY_PROTCTION>(56));
    if (security_error) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            static_cast<unsigned short>((::GlobalData::s_pSecuProtectionField)->GetCashShopErr(
                security_error)),
            0,
            message.items[0].goods_no,
            1,
            1,
            0);
        return 0;
    }

    if (useCountDownCoinInFreeCoinDungeon(user, message)) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            0x13u,
            0,
            0,
            1,
            1,
            0);
        return 0;
    }

    if (gift == 1) {
        MSG_CHECK_MAIL_RECEIVER check_message;
        check_message.set<MSG_CHECK_MAIL_RECEIVER::kBuyCeraShopItem,
                          MSG_BUY_CERASHOP_ITEM>(message);
        const int check_result =
            WongWork::CMailBoxHelper::ReqDBCheckReceiver(
                user,
                receiver,
                &check_message);
        if (check_result) {
            LogManager::logFormat(
                1,
                "PacketDispatcher_Impl_1.cpp",
                kBuyCeraShopItemDispatchSig,
                37058,
                "[ITEM_GIFT] Fail charac_no[%u] error_code[%d]",
                user->get_charac_no(-1),
                check_result);
            user->SendCmdErrorPacket2Byte(
                ENUM_CMDPACKET_BUY_CERASHOP_ITEM,
                static_cast<unsigned short>(check_result));
        }
        return 0;
    }

    if (static_cast<unsigned char>(
            ::GlobalData::s_pItemVendingMachine->BuyItem(
            user,
            message))) {
        return 0;
    }
    return LineFunc(37070, kBuyCeraShopItemDispatchSig, 0, 0);
}

// IDA_EA: 0x81FE4B8
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItem33useCountDownCoinInFreeCoinDungeonEP5CUserRK21MSG_BUY_CERASHOP_ITEM
// DEMANGLED: Dispatcher_BuyCeraShopItem::useCountDownCoinInFreeCoinDungeon(CUser *,MSG_BUY_CERASHOP_ITEM const&)
bool Dispatcher_BuyCeraShopItem::useCountDownCoinInFreeCoinDungeon(
    CUser* user,
    const MSG_BUY_CERASHOP_ITEM& message) {
    if (!user) {
        return false;
    }
    if (message.item_count != 1) {
        return false;
    }
    CCeraShopGoods* goods =
        G_CDataManager()->FindGoods(message.items[0].goods_no);
    if (!goods) {
        return false;
    }
    if (goods->GetItemIdx() != 1) {
        return false;
    }
    CParty* party = user->GetParty();
    if (!party) {
        return false;
    }
    const CDungeon* dungeon =
        reinterpret_cast<CDungeon**>(party)[811];
    if (!dungeon) {
        return false;
    }
    return static_cast<unsigned char>(
               party->checkFreeRevivalCondition(dungeon)) != 0;
}

// IDA_EA: 0x82334B8
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BuyCeraShopItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BuyCeraShopItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82334C2
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BuyCeraShopItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BuyCeraShopItem::process(CUser* user,
                                        MSG_BASE& message,
                                        ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595B4C
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItemC2Ev
// DEMANGLED: Dispatcher_BuyCeraShopItem::Dispatcher_BuyCeraShopItem(void)
Dispatcher_BuyCeraShopItem::Dispatcher_BuyCeraShopItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824368A
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItemD2Ev
// DEMANGLED: Dispatcher_BuyCeraShopItem::~Dispatcher_BuyCeraShopItem()
// IDA_EA: 0x82436BA
// IDA_NAME: _ZN26Dispatcher_BuyCeraShopItemD0Ev
// DEMANGLED: Dispatcher_BuyCeraShopItem::~Dispatcher_BuyCeraShopItem()
Dispatcher_BuyCeraShopItem::~Dispatcher_BuyCeraShopItem() {}

// IDA_EA: 0x81FE578
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BuyAutomatItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BuyAutomatItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user) {
        cMyTrace trace(kBuyAutomatItemDispatchSig, 37189, 0);
        trace("pUser doesn't exist in Dispatcher_BuyAutomatItem::dispatch_sig");
        return LineFunc(37190, kBuyAutomatItemDispatchSig, 0, 0);
    }

    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(37194, kBuyAutomatItemDispatchSig, 0, 0);
    }
    if (static_cast<unsigned char>(user->CheckInTrade())) {
        ::GlobalData::s_pCeraShop->ProcessError(user, 0x13u, 0, 0, 1, 1, 0);
        return 0;
    }

    const int security_error = (::GlobalData::s_pSecuProtectionField)->Check(
        user,
        static_cast<SECURITY_PROTCTION>(68));
    if (security_error) {
        ::GlobalData::s_pCeraShop->ProcessError(
            user,
            static_cast<unsigned short>(security_error == 123 ? 118 : 134),
            0,
            0,
            1,
            1,
            0);
        return 0;
    }

    MSG_BUY_CERASHOP_ITEM message;
    unsigned char item_count = 0;
    if (packet.get_byte(item_count) != 1) {
        return LineFunc(37234, kBuyAutomatItemDispatchSig, 0, 0);
    }
    if (item_count == 0 || item_count > 0x23u) {
        // Original stack layout supplies the first goods word to this error
        // call even before an item record is decoded.  The named message
        // layout preserves that exact source location without inventing a
        // fallback goods id.
        ::GlobalData::s_pCeraShop->ProcessError(
            user, 1u, 0, message.items[0].goods_no, 1, 1, 0);
        return 0;
    }

    message.item_count = item_count;
    PurchaseLimitItem::Info purchase_limit;
    for (int index = 0; index < item_count; ++index) {
        CeraShopBuyItem& item = message.items[index];
        if (packet.get_byte(item.select_type) != 1) {
            return LineFunc(37234, kBuyAutomatItemDispatchSig, 0, 0);
        }
        if (packet.get_byte(item.avatar_select) != 1) {
            return LineFunc(37237, kBuyAutomatItemDispatchSig, 0, 0);
        }
        if (packet.get_int(item.goods_no) != 1) {
            return LineFunc(37240, kBuyAutomatItemDispatchSig, 0, 0);
        }
        if (purchase_limit.reserved == 0) {
            ARAD::CheckAccountItem(user, item.goods_no, purchase_limit);
        }
    }

    if (purchase_limit.reserved != 0) {
        // The original function still emits this legacy stream.  The exact
        // PurchaseLimitItem layout proves reserved is initialized, parsed and
        // copied as zero, while DBTask has no signal-250 slot; do not replace
        // this unreachable original branch with a fabricated DB provider.
        CStreamGuard guard(::GlobalData::s_stream_pool->Acquire(
                               "PacketDispatcher_Impl_1.cpp", 37255),
                           true);
        guard << 506;
        guard << user->GetUID();
        guard << user->get_acc_id();
        guard << user->get_charac_no(-1);
        // IDA calls CStreamGuard::operator<<(unsigned int) here, followed by
        // two calls to the signed-short overload (0x80E2292, 0x822AD66,
        // 0x822AD66).  Keep the original 4/2/2 byte stream shape.
        guard << purchase_limit.reserved;
        guard << static_cast<short>(purchase_limit.purchase_type);
        guard << static_cast<short>(purchase_limit.purchase_limit_count);
        MSG_BUY_CERASHOP_ITEM* const stream_message =
            guard.GetInBuffer<MSG_BUY_CERASHOP_ITEM>();
        *stream_message = message;
        ::GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
        return 0;
    }

    if (static_cast<unsigned char>(
            ::GlobalData::s_pItemVendingMachine->BuyItem(user, message))) {
        return 0;
    }
    return LineFunc(37251, kBuyAutomatItemDispatchSig, 0, 0);
}

// IDA_EA: 0x82334CC
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BuyAutomatItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BuyAutomatItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82334D6
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BuyAutomatItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BuyAutomatItem::process(CUser* user,
                                       MSG_BASE& message,
                                       ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x859616C
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItemC2Ev
// DEMANGLED: Dispatcher_BuyAutomatItem::Dispatcher_BuyAutomatItem(void)
Dispatcher_BuyAutomatItem::Dispatcher_BuyAutomatItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x824363C
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItemD2Ev
// IDA_EA: 0x824366C
// IDA_NAME: _ZN25Dispatcher_BuyAutomatItemD0Ev
// DEMANGLED: Dispatcher_BuyAutomatItem::~Dispatcher_BuyAutomatItem()
Dispatcher_BuyAutomatItem::~Dispatcher_BuyAutomatItem() {}

// IDA_EA: 0x821F72A
// IDA_NAME: _ZNK31Dispatcher_BuyCeraShopLimitItem15_checkUserStateEP5CUser
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::_checkUserState(CUser *)const
int Dispatcher_BuyCeraShopLimitItem::_checkUserState(CUser* user) const {
    const int security_error = (::GlobalData::s_pSecuProtectionField)->Check(
        user,
        static_cast<SECURITY_PROTCTION>(57));
    if (security_error) {
        return security_error;
    }
    private_store::CPrivateStoreMgr* private_store_manager =
        private_store::GetInstancePrivateStoreMgr();
    if (static_cast<unsigned char>(
            private_store_manager->IsBusyPrivateStore(user))) {
        return 19;
    }
    if (static_cast<unsigned char>(user->CheckInTrade())) {
        return 19;
    }
    const int cur_sec = GlobalData::system_time()->getCurSec();
    if (cur_sec - user->getLastItemLimitEditionBuyTime() > 29) {
        return 0;
    }
    return 9;
}

// IDA_EA: 0x821F7CC
// IDA_NAME: _ZNK31Dispatcher_BuyCeraShopLimitItem18_checkBuyConditionEP5CUserPK17CItemLimitEdition
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::_checkBuyCondition(CUser *,CItemLimitEdition const*)const
int Dispatcher_BuyCeraShopLimitItem::_checkBuyCondition(
    CUser* user,
    const CItemLimitEdition* item_info) const {
    if (item_info->getCondCharacJob() >= 0 &&
        item_info->getCondCharacJob() <
            oldsrv::config::kCharacterJobCount) {
        if (user->get_charac_job() !=
            item_info->getCondCharacJob()) {
            return 7;
        }
    }
    if (static_cast<unsigned short>(
            item_info->getCondCharacLevelBegin())) {
        if (static_cast<unsigned short>(
                item_info->getCondCharacLevelBegin()) >
            user->get_charac_level()) {
            return 7;
        }
    }
    if (static_cast<unsigned short>(item_info->getCondCharacLevelEnd())) {
        if (static_cast<unsigned short>(item_info->getCondCharacLevelEnd()) <
            user->get_charac_level()) {
            return 7;
        }
    }
    if (item_info->getCondCharacCreateTimeBegin()) {
        if (item_info->getCondCharacCreateTimeBegin() >
            user->getCurCharacCreateTime()) {
            return 7;
        }
    }
    if (item_info->getCondCharacCreateTimeEnd()) {
        if (item_info->getCondCharacCreateTimeEnd() <
            user->getCurCharacCreateTime()) {
            return 7;
        }
    }
    if (item_info->getCondAccCreateTimeBegin()) {
        if (item_info->getCondAccCreateTimeBegin() >
            user->getRegTime()) {
            return 7;
        }
    }
    if (item_info->getCondAccCreateTimeEnd() &&
        item_info->getCondAccCreateTimeEnd() < user->getRegTime()) {
        return 7;
    }
    return 0;
}

// IDA_EA: 0x821F9BA
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_BuyCeraShopLimitItem::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2) {
        return LineFunc(55431, kBuyCeraShopLimitItemDispatchSig, 0, 0);
    }

    Packet_Item_Limit_Edition_Buyable_Query query_packet;
    unsigned char* query =
        reinterpret_cast<unsigned char*>(&query_packet);

    unsigned int ipg_no = 0;
    if (packet.get_int(ipg_no) != 1) {
        return LineFunc(55436, kBuyCeraShopLimitItemDispatchSig, 0, 0);
    }
    WriteUIntLE(query + 14, ipg_no);

    unsigned char raw_avatar_select = 0;
    if (packet.get_byte(raw_avatar_select) != 1) {
        return LineFunc(55437, kBuyCeraShopLimitItemDispatchSig, 0, 0);
    }
    query[18] = raw_avatar_select;

    unsigned char avatar_option_count = 0;
    if (packet.get_byte(avatar_option_count) != 1 ||
        avatar_option_count > 0x23u) {
        return LineFunc(55440, kBuyCeraShopLimitItemDispatchSig, 0, 0);
    }
    query[19] = avatar_option_count;
    for (int i = 0; i < avatar_option_count; ++i) {
        unsigned long item_idx = 0;
        if (packet.get_item_idx(item_idx) != 1) {
            return LineFunc(55444, kBuyCeraShopLimitItemDispatchSig, 0, 0);
        }
        WriteUIntLE(query + 20 + 5 * i,
                    static_cast<unsigned int>(item_idx));
        if (packet.get_byte(query[24 + 5 * i]) != 1) {
            return LineFunc(55445, kBuyCeraShopLimitItemDispatchSig, 0, 0);
        }
    }

    unsigned char additional_item_count = 0;
    if (packet.get_byte(additional_item_count) != 1 ||
        additional_item_count > 0x23u) {
        return LineFunc(55453, kBuyCeraShopLimitItemDispatchSig, 0, 0);
    }
    query[195] = additional_item_count;
    for (int j = 0; j < additional_item_count; ++j) {
        unsigned long item_idx = 0;
        if (packet.get_item_idx(item_idx) != 1) {
            return LineFunc(55457, kBuyCeraShopLimitItemDispatchSig, 0, 0);
        }
        WriteUIntLE(query + 196 + 8 * j,
                    static_cast<unsigned int>(item_idx));
        unsigned short first_short = 0;
        if (packet.get_short(first_short) != 1) {
            return LineFunc(55458, kBuyCeraShopLimitItemDispatchSig, 0, 0);
        }
        WriteUShortLE(query + 200 + 8 * j, first_short);
        unsigned short second_short = 0;
        if (packet.get_short(second_short) != 1) {
            return LineFunc(55459, kBuyCeraShopLimitItemDispatchSig, 0, 0);
        }
        WriteUShortLE(query + 202 + 8 * j, second_short);
    }

    for (int k = 0; k < additional_item_count; ++k) {
        for (int m = k + 1; m < additional_item_count; ++m) {
            if (ReadUIntLE(query + 196 + 8 * k) ==
                    ReadUIntLE(query + 196 + 8 * m) &&
                ReadUShortLE(query + 200 + 8 * k) ==
                    ReadUShortLE(query + 200 + 8 * m)) {
                return LineFunc(55468,
                                kBuyCeraShopLimitItemDispatchSig,
                                0,
                                0);
            }
        }
    }

    int error = _checkUserState(user);
    if (error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
            static_cast<unsigned char>(error));
        return 0;
    }

    CItemLimitEditionMgr* item_limit_edition_mgr =
        G_CDataManager()->getItemLimitEditionMgr();
    CItemLimitEdition* item_info =
        item_limit_edition_mgr
            ? item_limit_edition_mgr->getItemInfo(ipg_no)
            : 0;
    if (!item_info) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 22);
        return 0;
    }
    if (static_cast<unsigned char>(item_info->isSellComplete())) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 95);
        return 0;
    }
    if (item_info->getSellEndTime() < GlobalData::system_time()->getCurSec()) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 22);
        return 0;
    }

    CItem* related_item = 0;
    if (item_info->getRelateditem() != 1) {
        related_item =
            G_CDataManager()->find_item(item_info->getRelateditem());
        if (!related_item) {
            user->SendCmdErrorPacket(
                                     ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                     22);
            return 0;
        }
    }

    if (user->GetCera() < item_info->getPriceCera()) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 11);
        return 0;
    }
    CInventory* inventory = static_cast<CInventory*>(
        user->getCurCharacInvenR());
    if (inventory->get_money() < item_info->getPriceGold()) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 10);
        return 0;
    }
    if (user->GetWinPoint() < item_info->getPriceWinPoint()) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 12);
        return 0;
    }

    error = _checkBuyCondition(user, item_info);
    if (error) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
            static_cast<unsigned char>(error));
        return 0;
    }

    unsigned short avatar_type_select = static_cast<unsigned short>(
        static_cast<signed char>(item_info->getAvatarTypeSelect()));
    if (related_item && IsAvatarItem(related_item)) {
        if (static_cast<unsigned char>(item_info->getAvatarTypeSelect()) ==
            0xffu) {
            std::vector<stAvatarTypeSelect_t>* avatar_select =
                reinterpret_cast<std::vector<stAvatarTypeSelect_t>*>(
                    reinterpret_cast<const CEquipItem*>(related_item)
                        ->getAvatarTypeSelect());
            for (std::vector<stAvatarTypeSelect_t>::const_iterator it =
                     avatar_select->begin();
                 it != avatar_select->end();
                 ++it) {
                ++avatar_type_select;
                if (it->socket_count > 0) {
                    break;
                }
            }
        }
    }

    if (static_cast<unsigned char>(
            ::GlobalData::s_pCeraShop->checkBuyingCondition(
            user,
            ipg_no,
            static_cast<unsigned long>(item_info->getRelateditem()),
            static_cast<unsigned int>(item_info->getRelatedItemCnt()),
            avatar_type_select,
            raw_avatar_select,
            avatar_type_select)) != 1) {
        user->SendCmdErrorPacket(
                                 ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                 7);
        return 0;
    }

    bool special_flag = false;
    int special_value_1 = 0;
    int special_value_2 = 0;
    int featured_idx = item_info->getFeaturedIdx();
    if (!featured_idx &&
        static_cast<unsigned char>(
            ::GlobalData::s_pCeraShop->IsSpecialItem(
            item_info->getRelateditem(),
            special_value_1,
            special_value_2,
            special_flag))) {
        featured_idx =
            ItemListFromDataManager(G_CDataManager())->GetRestrictCode(
                item_info->getRelateditem());
    }
    if (featured_idx) {
        if (static_cast<unsigned char>(
                user->IsRestrictedGoods(featured_idx))) {
            user->SendCmdErrorPacket(
                                     ENUM_CMDPACKET_BUY_CERASHOP_LIMIT_ITEM,
                                     18);
            return 0;
        }
        user->SetRestrictedGoods(featured_idx);
    }

    user->updateLastItemLimitEditionBuyTime(
        GlobalData::system_time()->getCurSec());
    WriteUIntLE(query + 10, static_cast<unsigned int>(user->GetUID()));

    CMonitorServerProxy* proxy =
        ::GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendTcpPacket(
        reinterpret_cast<char*>(query),
        query_packet.size);
    return 0;
}

// IDA_EA: 0x8233F42
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_BuyCeraShopLimitItem::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233F4C
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_BuyCeraShopLimitItem::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8596D20
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItemC2Ev
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::Dispatcher_BuyCeraShopLimitItem(void)
Dispatcher_BuyCeraShopLimitItem::Dispatcher_BuyCeraShopLimitItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8241026
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItemD2Ev
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::~Dispatcher_BuyCeraShopLimitItem()
// IDA_EA: 0x8241056
// IDA_NAME: _ZN31Dispatcher_BuyCeraShopLimitItemD0Ev
// DEMANGLED: Dispatcher_BuyCeraShopLimitItem::~Dispatcher_BuyCeraShopLimitItem()
Dispatcher_BuyCeraShopLimitItem::~Dispatcher_BuyCeraShopLimitItem() {}
