#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

#include <type_traits>

#include "infra/log/format/trace.h"
#include "old_game/dungeon/instance/dungeon_pvp_value_types.h"
#include "old_game/economy/inventory/inventory_params.h"
#include "old_game/social/party/packet_dispatcher_set_party_info.h"
#include "old_game/economy/mall/npc_shop_params.h"
#include "old_game/economy/mall/quest_shop_runtime.h"
#include "old_game/player/core/character_messages.h"
#include "old_game/player/session/character_select_params.h"
#include "old_game/player/core/cuser.h"
#include "old_game/player/core/userinfo_messages.h"
#include "old_game/world/region/packet_dispatcher_world_business_043.h"
#include "infra/network/packet/packet_buf.h"

namespace {

// The old i386 dispatch templates reserve a physical, unconstructed 13-byte
// MSG_BASE extent.  Do not replace it with `MSG_BASE message;`: that would add
// a MSG_BASE::C2 call edge which is absent from all three raw functions.
typedef std::aligned_storage<sizeof(MSG_BASE), alignof(MSG_BASE)>::type
    RawMsgBaseStorage;

typedef std::aligned_storage<sizeof(MSG_USERINFO_REQ),
                             alignof(MSG_USERINFO_REQ)>::type
    RawMsgUserinfoReqStorage;

static_assert(sizeof(RawMsgBaseStorage) == sizeof(MSG_BASE),
              "the raw MSG_BASE dispatcher extent must remain 13 bytes");
static_assert(sizeof(RawMsgUserinfoReqStorage) == 0x10u,
              "the raw MSG_USERINFO_REQ dispatcher extent must remain 16 bytes");

typedef std::aligned_storage<sizeof(MSG_PARTY_SET_INFO),
                             alignof(MSG_PARTY_SET_INFO)>::type
    RawMsgPartySetInfoStorage;

typedef std::aligned_storage<sizeof(MSG_DECREASE_DURABILITY),
                             alignof(MSG_DECREASE_DURABILITY)>::type
    RawMsgDecreaseDurabilityStorage;

typedef std::aligned_storage<sizeof(MSG_REPAIR_EQUIP),
                             alignof(MSG_REPAIR_EQUIP)>::type
    RawMsgRepairEquipStorage;

typedef std::aligned_storage<sizeof(MSG_SECRET_SHOP_BUY_ITEM),
                             alignof(MSG_SECRET_SHOP_BUY_ITEM)>::type
    RawMsgSecretShopBuyItemStorage;

typedef std::aligned_storage<sizeof(MSG_SECRET_SHOP_OPEN_CLOSE),
                             alignof(MSG_SECRET_SHOP_OPEN_CLOSE)>::type
    RawMsgSecretShopOpenCloseStorage;

typedef std::aligned_storage<sizeof(MSG_WALKOUT_PARTY),
                             alignof(MSG_WALKOUT_PARTY)>::type
    RawMsgWalkoutPartyStorage;

static_assert(sizeof(RawMsgPartySetInfoStorage) == 0x36u,
              "the raw MSG_PARTY_SET_INFO dispatcher extent must remain 54 bytes");
static_assert(sizeof(RawMsgDecreaseDurabilityStorage) == 0x0eu,
              "the raw MSG_DECREASE_DURABILITY dispatcher extent must remain 14 bytes");
static_assert(sizeof(RawMsgRepairEquipStorage) == 0x12u,
              "the raw MSG_REPAIR_EQUIP dispatcher extent must remain 18 bytes");
static_assert(sizeof(RawMsgSecretShopBuyItemStorage) == 0x15u,
              "the raw MSG_SECRET_SHOP_BUY_ITEM dispatcher extent must remain 21 bytes");
static_assert(sizeof(RawMsgSecretShopOpenCloseStorage) == 0x0eu,
              "the raw MSG_SECRET_SHOP_OPEN_CLOSE dispatcher extent must remain 14 bytes");
static_assert(sizeof(RawMsgWalkoutPartyStorage) == 0x0eu,
              "the raw MSG_WALKOUT_PARTY dispatcher extent must remain 14 bytes");
static_assert(sizeof(MSG_REQUEST_CHARAC_SKILL_INFO) == 0x0eu,
              "the typed request-charac-skill message must remain 14 bytes");
static_assert(sizeof(stRequestCharacSkillInfo) == 0x08u,
              "the typed request-charac-skill parameter must retain +0x04");
static_assert(sizeof(MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET) == 0x0eu,
              "the typed combo-extension message must remain 14 bytes");
static_assert(sizeof(stComboSkillExtensionQuickSlotReset) == 0x08u,
              "the typed combo-extension parameter must remain 8 bytes");
static_assert(sizeof(MSG_SAVE_CHARACTER_OPTION) == 0x211u,
              "the typed save-character-option message must remain 529 bytes");
static_assert(sizeof(stSaveCharacOption) == 0x08u,
              "the typed save-character-option parameter must remain 8 bytes");
static_assert(sizeof(stCharacSlotExtendEffect) == 0x0cu,
              "the typed character-slot-effect parameter must remain 12 bytes");
static_assert(sizeof(MSG_QUEST_SHOP) == 0x0fu,
              "the typed quest-shop message must remain 15 bytes");
static_assert(sizeof(STQuestShopBuyParam) == 0x0cu,
              "the typed quest-shop-buy parameter must remain 12 bytes");

}  // namespace


// Declare every member specialization before defining a constructor.  That
// prevents vtable emission from implicitly instantiating a primary-template
// member before its exact raw specialization below.
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    ~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    ~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       PcRoomPlayTimeRewardParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnRecoverStaminaParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnRecoverStaminaParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_USERINFO_REQ,
                       stOnRecoverStaminaParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_PARTY_SET_INFO,
                  stSetPartyInfo,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_PARTY_SET_INFO,
                  stSetPartyInfo,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_PARTY_SET_INFO,
                       stSetPartyInfo,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                  stRequestCharacSkillInfo,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                  stRequestCharacSkillInfo,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                       stRequestCharacSkillInfo,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                  stComboSkillExtensionQuickSlotReset,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                  stComboSkillExtensionQuickSlotReset,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                       stComboSkillExtensionQuickSlotReset,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                  stSaveCharacOption,
                  static_cast<ch_state>(2)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                  stSaveCharacOption,
                  static_cast<ch_state>(2)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                       stSaveCharacOption,
                       static_cast<ch_state>(2)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE,
                  stCharacSlotExtendEffect,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stCharacSlotExtendEffect,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       stCharacSlotExtendEffect,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_QUEST_SHOP,
                  STQuestShopBuyParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_QUEST_SHOP,
                  STQuestShopBuyParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_QUEST_SHOP,
                       STQuestShopBuyParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_DECREASE_DURABILITY,
                  stOnUseEquipParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_DECREASE_DURABILITY,
                  stOnUseEquipParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_DECREASE_DURABILITY,
                       stOnUseEquipParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_REPAIR_EQUIP,
                  stRepairEquipParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_REPAIR_EQUIP,
                  stRepairEquipParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_REPAIR_EQUIP,
                       stRepairEquipParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnGetUserInfoParam,
                  static_cast<ch_state>(1)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnGetUserInfoParam,
                  static_cast<ch_state>(1)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_USERINFO_REQ,
                       stOnGetUserInfoParam,
                       static_cast<ch_state>(1)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_BASE,
                  stImageCommunicationEquipmentUse,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stImageCommunicationEquipmentUse,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE,
                       stImageCommunicationEquipmentUse,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                  stSecretShopBuyItemParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                  stSecretShopBuyItemParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                       stSecretShopBuyItemParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

template <>
IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                  stSecretShopOpenCloseParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                  stSecretShopOpenCloseParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::dispatch_template(CUser*,
                                                                     PacketBuf&);
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                       stSecretShopOpenCloseParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&);

// These three concrete specializations must be declared before any earlier
// constructor body emits a dispatcher vtable.  Their definitions remain in
// the original-IDB order near the end of this translation unit.
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&);
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&);
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::check_state(CUser*,
                                                               PacketBuf&);
template <>
void IPacketDispatcher<MSG_WALKOUT_PARTY,
                       stWalkoutPartyParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&);
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser*, PacketBuf&);
template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&);
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    check_state(CUser*, PacketBuf&);
template <>
void IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&);
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser*, PacketBuf&);
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher();

// IDA_EA: 0x8598712
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B536
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B566
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    ~IPacketDispatcher() {}

// IDA_EA: 0x824AA8C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    dispatch_template(CUser* user, PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    ParamBase param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x824ABDC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    dispatch_sig(CUser*, PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824ABE6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    check_state(CUser* user, PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    const int packet_length = packet.get_len();
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet_length);
    return 149;
}

// IDA_EA: 0x824AC7A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state3EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)3>::send(CUser *,ParamBase&)
template <>
void IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(3)>::
    send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598E4A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BB4E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825BB7E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    ~IPacketDispatcher() {}

// IDA_EA: 0x824D436
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    dispatch_template(CUser* user, PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    ParamBase param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x824D586
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    dispatch_sig(CUser*, PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824D590
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    check_state(CUser* user, PacketBuf& packet) {
    if (user->get_state() > 1) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    const int packet_length = packet.get_len();
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = ParamBase, ch_state STATE = (ch_state)2u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet_length);
    return 149;
}

// IDA_EA: 0x825D63C
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE9ParamBaseL8ch_state2EE4sendEP5CUserRS1_
// DEMANGLED: IPacketDispatcher<MSG_BASE,ParamBase,(ch_state)2>::send(CUser *,ParamBase&)
template <>
void IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(2)>::
    send(CUser*, ParamBase&) {}

// IDA_EA: 0x85990B2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B362
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B392
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  PcRoomPlayTimeRewardParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8249D0A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    PcRoomPlayTimeRewardParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    if (this->dispatch_sig(user, packet) > 0) {
        return 133;
    }
    return 0;
}

// IDA_EA: 0x8249E5A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8249E64
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      PcRoomPlayTimeRewardParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    const int packet_length = packet.get_len();
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = PcRoomPlayTimeRewardParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet_length);
    return 149;
}

// IDA_EA: 0x825D5B2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE25PcRoomPlayTimeRewardParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,PcRoomPlayTimeRewardParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       PcRoomPlayTimeRewardParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x85980BA
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnRecoverStaminaParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82365C4
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x82365F4
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnRecoverStaminaParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x82597AA
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgUserinfoReqStorage message_storage;
    MSG_USERINFO_REQ& message =
        *reinterpret_cast<MSG_USERINFO_REQ*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stOnRecoverStaminaParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82598FA
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8259904
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnRecoverStaminaParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_USERINFO_REQ, PARAM_TYPE = stOnRecoverStaminaParam, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825ADC2
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ23stOnRecoverStaminaParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnRecoverStaminaParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_USERINFO_REQ,
                       stOnRecoverStaminaParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598472
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_PARTY_SET_INFO,
                  stSetPartyInfo,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D498
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D4C8
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_PARTY_SET_INFO,
                  stSetPartyInfo,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x8259462
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgPartySetInfoStorage message_storage;
    MSG_PARTY_SET_INFO& message =
        *reinterpret_cast<MSG_PARTY_SET_INFO*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stSetPartyInfo param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82595B2
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x82595BC
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_PARTY_SET_INFO,
                      stSetPartyInfo,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_PARTY_SET_INFO, PARAM_TYPE = stSetPartyInfo, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D9AE
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_PARTY_SET_INFO14stSetPartyInfoL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_PARTY_SET_INFO,stSetPartyInfo,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_PARTY_SET_INFO,
                       stSetPartyInfo,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598392
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                  stRequestCharacSkillInfo,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B49A
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B4CA
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                  stRequestCharacSkillInfo,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x824A6A6
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_REQUEST_CHARAC_SKILL_INFO message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stRequestCharacSkillInfo param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824A802
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824A80C
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                      stRequestCharacSkillInfo,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_REQUEST_CHARAC_SKILL_INFO, PARAM_TYPE = stRequestCharacSkillInfo, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D5CA
// IDA_NAME: _ZN17IPacketDispatcherI29MSG_REQUEST_CHARAC_SKILL_INFO24stRequestCharacSkillInfoL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,stRequestCharacSkillInfo,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_REQUEST_CHARAC_SKILL_INFO,
                       stRequestCharacSkillInfo,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8599192
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                  stComboSkillExtensionQuickSlotReset,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82623E6
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x8262416
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                  stComboSkillExtensionQuickSlotReset,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x826132A
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stComboSkillExtensionQuickSlotReset param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8261486
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8261490
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                      stComboSkillExtensionQuickSlotReset,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET, PARAM_TYPE = stComboSkillExtensionQuickSlotReset, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x826261E
// IDA_NAME: _ZN17IPacketDispatcherI42MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET35stComboSkillExtensionQuickSlotResetL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,stComboSkillExtensionQuickSlotReset,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_COMBO_SKILL_EXTENSION_QUICK_SLOT_RESET,
                       stComboSkillExtensionQuickSlotReset,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598782
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                  stSaveCharacOption,
                  static_cast<ch_state>(2)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825CE80
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::~IPacketDispatcher()
// IDA_EA: 0x825CEB0
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                  stSaveCharacOption,
                  static_cast<ch_state>(2)>::~IPacketDispatcher() {}

// IDA_EA: 0x8255492
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_SAVE_CHARACTER_OPTION message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stSaveCharacOption param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82555FC
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8255606
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                      stSaveCharacOption,
                      static_cast<ch_state>(2)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 1) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SAVE_CHARACTER_OPTION, PARAM_TYPE = stSaveCharacOption, ch_state STATE = (ch_state)2u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825569A
// IDA_NAME: _ZN17IPacketDispatcherI25MSG_SAVE_CHARACTER_OPTION18stSaveCharacOptionL8ch_state2EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,stSaveCharacOption,(ch_state)2>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SAVE_CHARACTER_OPTION,
                       stSaveCharacOption,
                       static_cast<ch_state>(2)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598D32
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stCharacSlotExtendEffect,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825BF92
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825BFC2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stCharacSlotExtendEffect,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x824F3DA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stCharacSlotExtendEffect param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824F52A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824F534
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stCharacSlotExtendEffect,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stCharacSlotExtendEffect, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D67E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE24stCharacSlotExtendEffectL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stCharacSlotExtendEffect,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stCharacSlotExtendEffect,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598C52
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_QUEST_SHOP,
                  STQuestShopBuyParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C202
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C232
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_QUEST_SHOP,
                  STQuestShopBuyParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x82502C8
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    MSG_QUEST_SHOP message;
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    STQuestShopBuyParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8250424
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825042E
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_QUEST_SHOP,
                      STQuestShopBuyParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_QUEST_SHOP, PARAM_TYPE = STQuestShopBuyParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D728
// IDA_NAME: _ZN17IPacketDispatcherI14MSG_QUEST_SHOP19STQuestShopBuyParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_QUEST_SHOP,STQuestShopBuyParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_QUEST_SHOP,
                       STQuestShopBuyParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598012
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_DECREASE_DURABILITY,
                  stOnUseEquipParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82363F0
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x8236420
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_DECREASE_DURABILITY,
                  stOnUseEquipParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x825A428
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgDecreaseDurabilityStorage message_storage;
    MSG_DECREASE_DURABILITY& message =
        *reinterpret_cast<MSG_DECREASE_DURABILITY*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stOnUseEquipParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x825A578
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825A582
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_DECREASE_DURABILITY,
                      stOnUseEquipParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_DECREASE_DURABILITY, PARAM_TYPE = stOnUseEquipParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825AE74
// IDA_NAME: _ZN17IPacketDispatcherI23MSG_DECREASE_DURABILITY17stOnUseEquipParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_DECREASE_DURABILITY,stOnUseEquipParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_DECREASE_DURABILITY,
                       stOnUseEquipParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859812A
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_REPAIR_EQUIP,
                  stRepairEquipParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82366FC
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x823672C
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_REPAIR_EQUIP,
                  stRepairEquipParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8258D3C
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgRepairEquipStorage message_storage;
    MSG_REPAIR_EQUIP& message =
        *reinterpret_cast<MSG_REPAIR_EQUIP*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stRepairEquipParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8258E8C
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8258E96
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_REPAIR_EQUIP,
                      stRepairEquipParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_REPAIR_EQUIP, PARAM_TYPE = stRepairEquipParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825AD1E
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_REPAIR_EQUIP18stRepairEquipParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_REPAIR_EQUIP,stRepairEquipParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_REPAIR_EQUIP,
                       stRepairEquipParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859809E
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnGetUserInfoParam,
                  static_cast<ch_state>(1)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x8236576
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::~IPacketDispatcher()
// IDA_EA: 0x82365A6
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_USERINFO_REQ,
                  stOnGetUserInfoParam,
                  static_cast<ch_state>(1)>::~IPacketDispatcher() {}

// IDA_EA: 0x8259998
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgUserinfoReqStorage message_storage;
    MSG_USERINFO_REQ& message =
        *reinterpret_cast<MSG_USERINFO_REQ*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stOnGetUserInfoParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8259AE8
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8259AF2
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_USERINFO_REQ,
                      stOnGetUserInfoParam,
                      static_cast<ch_state>(1)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_USERINFO_REQ, PARAM_TYPE = stOnGetUserInfoParam, ch_state STATE = (ch_state)1u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x8259B86
// IDA_NAME: _ZN17IPacketDispatcherI16MSG_USERINFO_REQ20stOnGetUserInfoParamL8ch_state1EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_USERINFO_REQ,stOnGetUserInfoParam,(ch_state)1>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_USERINFO_REQ,
                       stOnGetUserInfoParam,
                       static_cast<ch_state>(1)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859900A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE,
                  stImageCommunicationEquipmentUse,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825B6BC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825B6EC
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE,
                  stImageCommunicationEquipmentUse,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x824B470
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stImageCommunicationEquipmentUse param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x824B5C0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x824B5CA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE,
                      stImageCommunicationEquipmentUse,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stImageCommunicationEquipmentUse, ch_state STATE = (ch_state)3u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D5E2
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE32stImageCommunicationEquipmentUseL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stImageCommunicationEquipmentUse,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE,
                       stImageCommunicationEquipmentUse,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859895E
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                  stSecretShopBuyItemParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C868
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C898
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                  stSecretShopBuyItemParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8252C68
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgSecretShopBuyItemStorage message_storage;
    MSG_SECRET_SHOP_BUY_ITEM& message =
        *reinterpret_cast<MSG_SECRET_SHOP_BUY_ITEM*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stSecretShopBuyItemParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8252DB8
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8252DC2
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                      stSecretShopBuyItemParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SECRET_SHOP_BUY_ITEM, PARAM_TYPE = stSecretShopBuyItemParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D88E
// IDA_NAME: _ZN17IPacketDispatcherI24MSG_SECRET_SHOP_BUY_ITEM24stSecretShopBuyItemParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,stSecretShopBuyItemParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SECRET_SHOP_BUY_ITEM,
                       stSecretShopBuyItemParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859897A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                  stSecretShopOpenCloseParam,
                  static_cast<ch_state>(0)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825C81A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825C84A
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                  stSecretShopOpenCloseParam,
                  static_cast<ch_state>(0)>::~IPacketDispatcher() {}

// IDA_EA: 0x8252A7C
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }

    RawMsgSecretShopOpenCloseStorage message_storage;
    MSG_SECRET_SHOP_OPEN_CLOSE& message =
        *reinterpret_cast<MSG_SECRET_SHOP_OPEN_CLOSE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }

    stSecretShopOpenCloseParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8252BCC
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8252BD6
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                      stSecretShopOpenCloseParam,
                      static_cast<ch_state>(0)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1,
        "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_SECRET_SHOP_OPEN_CLOSE, PARAM_TYPE = stSecretShopOpenCloseParam, ch_state STATE = (ch_state)0u]",
        148,
        "return __LINE__: code(%d) size(%d)",
        code,
        packet.get_len());
    return 149;
}

// IDA_EA: 0x825D888
// IDA_NAME: _ZN17IPacketDispatcherI26MSG_SECRET_SHOP_OPEN_CLOSE26stSecretShopOpenCloseParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,stSecretShopOpenCloseParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_SECRET_SHOP_OPEN_CLOSE,
                       stSecretShopOpenCloseParam,
                       static_cast<ch_state>(0)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859848E
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D4E6
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D516
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher() {}

// IDA_EA: 0x8259650
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser* user, PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }
    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }
    stLeavePartyParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }
    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82597A0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&) {
    return 0;
}

// IDA_EA: 0x825D9B4
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    check_state(CUser* user, PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }
    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1, "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stLeavePartyParam, ch_state STATE = (ch_state)0u]",
        148, "return __LINE__: code(%d) size(%d)", code, packet.get_len());
    return 149;
}

// IDA_EA: 0x825DA46
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE17stLeavePartyParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stLeavePartyParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE, stLeavePartyParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&) {}

// IDA_EA: 0x8598226
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x82369BA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x82369EA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_WALKOUT_PARTY,
                  stWalkoutPartyParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x82579EA
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }
    RawMsgWalkoutPartyStorage message_storage;
    MSG_WALKOUT_PARTY& message =
        *reinterpret_cast<MSG_WALKOUT_PARTY*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }
    stWalkoutPartyParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }
    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8257B3A
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                                PacketBuf&) {
    return 0;
}

// IDA_EA: 0x8257B44
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_WALKOUT_PARTY,
                      stWalkoutPartyParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                               PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }
    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1, "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_WALKOUT_PARTY, PARAM_TYPE = stWalkoutPartyParam, ch_state STATE = (ch_state)3u]",
        148, "return __LINE__: code(%d) size(%d)", code, packet.get_len());
    return 149;
}

// IDA_EA: 0x825ACEE
// IDA_NAME: _ZN17IPacketDispatcherI17MSG_WALKOUT_PARTY19stWalkoutPartyParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_WALKOUT_PARTY,stWalkoutPartyParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_WALKOUT_PARTY,
                       stWalkoutPartyParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x85984AA
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    IPacketDispatcher()
    : IDispatcher() {}

// IDA_EA: 0x825D44A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::~IPacketDispatcher()
// IDA_EA: 0x825D47A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    ~IPacketDispatcher() {}

// IDA_EA: 0x825810A
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    check_state(CUser* user, PacketBuf& packet) {
    if (user->get_state() >= 0) {
        return 0;
    }
    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(
        1, "PacketDispatcher.h",
        "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_BASE, PARAM_TYPE = stStartGameParam, ch_state STATE = (ch_state)0u]",
        148, "return __LINE__: code(%d) size(%d)", code, packet.get_len());
    return 149;
}

// IDA_EA: 0x825D9A8
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    send(CUser*, ParamBase&) {}

// IDA_EA: 0x8257FB0
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_template(CUser* user, PacketBuf& packet) {
    if (this->check_state(user, packet) > 0) {
        return 0;
    }
    RawMsgBaseStorage message_storage;
    MSG_BASE& message = *reinterpret_cast<MSG_BASE*>(&message_storage);
    int result = this->read(packet, message);
    if (result != 0) {
        return result;
    }
    stStartGameParam param;
    result = this->process(user, message, param);
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }
    this->send(user, param);
    return this->dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x8258100
// IDA_NAME: _ZN17IPacketDispatcherI8MSG_BASE16stStartGameParamL8ch_state0EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_BASE,stStartGameParam,(ch_state)0>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_BASE, stStartGameParam, static_cast<ch_state>(0)>::
    dispatch_sig(CUser*, PacketBuf&) {
    return 0;
}
