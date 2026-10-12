#include "old_game/economy/equipment/packet_dispatcher_upgrade_item.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/secu_protection_field_abi.h"
#include "infra/security/policy/service_restrict_manager_abi.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_data/PVF/genuine_upgrade_table.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"

namespace {

const char kUpgradeItemDispatchSignature[] =
    "virtual int Dispatcher_UpgradeItem::dispatch_sig(CUser*, PacketBuf&)";
const char kUpgradeItemSeparateReadSignature[] =
    "virtual int Dispatcher_UpgradeItemSeparate::read(PacketBuf&, MSG_BASE&)";

}  // namespace

// IDA_EA: 0x8595CF0
// IDA_NAME: _ZN22Dispatcher_UpgradeItemC2Ev
// DEMANGLED: Dispatcher_UpgradeItem::Dispatcher_UpgradeItem(void)
Dispatcher_UpgradeItem::Dispatcher_UpgradeItem()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8243294
// IDA_NAME: _ZN22Dispatcher_UpgradeItemD2Ev
// DEMANGLED: Dispatcher_UpgradeItem::~Dispatcher_UpgradeItem()
// IDA_EA: 0x82432C4
// IDA_NAME: _ZN22Dispatcher_UpgradeItemD0Ev
// DEMANGLED: Dispatcher_UpgradeItem::~Dispatcher_UpgradeItem()
Dispatcher_UpgradeItem::~Dispatcher_UpgradeItem() {}

// IDA_EA: 0x82006D0
// IDA_NAME: _ZN22Dispatcher_UpgradeItem12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_UpgradeItem::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_UpgradeItem::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user || user->CheckInTrade()) {
        return 0;
    }
    if (user->get_state() != static_cast<ch_state>(3) ||
        !user->getCurCharacR()) {
        return LineFunc(38168, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (user->CheckInTrade()) {
        return 0;
    }

    MSG_ITEM_UPGRADE message;
    if (packet.get_byte(message.body_.mode) != 1) {
        return LineFunc(38192, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(message.body_.target_slot) != 1) {
        return LineFunc(38194, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (packet.get_int(message.body_.target_item_id) != 1) {
        return LineFunc(38196, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(message.body_.material_slot) != 1) {
        return LineFunc(38198, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (packet.get_short(message.body_.ticket_slot) != 1) {
        return LineFunc(38201, kUpgradeItemDispatchSignature, 0, 0);
    }

    unsigned int item_name_length = 0;
    if (packet.get_int(item_name_length) != 1) {
        return LineFunc(38208, kUpgradeItemDispatchSignature, 0, 0);
    }
    if (packet.get_str(message.body_.guild_item_name,
                       sizeof(message.body_.guild_item_name),
                       item_name_length) != 1) {
        return LineFunc(38210, kUpgradeItemDispatchSignature, 0, 0);
    }

    if (ARAD::Singleton<ServiceRestrictManager>::Get()->isRestricted(
            user, RestrictCategory::AuthMobile, 7)) {
        user->SendCmdErrorPacket2Byte(ENUM_CMDPACKET_UPGRADE_ITEM, 209);
        return 0;
    }

    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(37));
    if (protection_result != 0) {
        user->SendCmdErrorPacket2Byte(
            ENUM_CMDPACKET_UPGRADE_ITEM,
            static_cast<unsigned short>(protection_result));
        return 0;
    }
    if (user->CheckItemLock(1, message.body_.target_slot)) {
        user->SendCmdErrorPacket2Byte(ENUM_CMDPACKET_UPGRADE_ITEM, 213);
        return 0;
    }

    G_CDataManager()->get_item_upgrade()->item_upgrade_system(
        user, &message, ENUM_CMDPACKET_UPGRADE_ITEM);
    return 0;
}

// IDA_EA: 0x82335BC
// IDA_NAME: _ZN22Dispatcher_UpgradeItem4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeItem::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeItem::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82335C6
// IDA_NAME: _ZN22Dispatcher_UpgradeItem7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeItem::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeItem::process(CUser* user,
                                    MSG_BASE& message,
                                    ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x8595D0C
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparateC2Ev
// DEMANGLED: Dispatcher_UpgradeItemSeparate::Dispatcher_UpgradeItemSeparate(void)
Dispatcher_UpgradeItemSeparate::Dispatcher_UpgradeItemSeparate()
    : IPacketDispatcher<MSG_ITEM_UPGRADE,
                        STUdateItemParam,
                        static_cast<ch_state>(0)>() {}

// IDA_EA: 0x8246096
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparateD2Ev
// DEMANGLED: Dispatcher_UpgradeItemSeparate::~Dispatcher_UpgradeItemSeparate()
// IDA_EA: 0x82460C6
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparateD0Ev
// DEMANGLED: Dispatcher_UpgradeItemSeparate::~Dispatcher_UpgradeItemSeparate()
Dispatcher_UpgradeItemSeparate::~Dispatcher_UpgradeItemSeparate() {}

// IDA_EA: 0x81CBE84
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparate11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeItemSeparate::check_error(CUser *,MSG_BASE &)
int Dispatcher_UpgradeItemSeparate::check_error(CUser* user,
                                                MSG_BASE& base_message) {
    const MSG_ITEM_UPGRADE& message =
        static_cast<const MSG_ITEM_UPGRADE&>(base_message);
    if (user->get_state() != static_cast<ch_state>(3) ||
        !user->getCurCharacR()) {
        return 7;
    }
    const int protection_result = GlobalData::s_pSecuProtectionField->Check(
        user, static_cast<SECURITY_PROTCTION>(39));
    if (protection_result != 0) {
        return protection_result;
    }
    return user->CheckItemLock(1, message.body_.target_slot) ? 213 : 0;
}

// IDA_EA: 0x81CBF24
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparate4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_UpgradeItemSeparate::read(PacketBuf &,MSG_BASE &)
int Dispatcher_UpgradeItemSeparate::read(PacketBuf& packet,
                                         MSG_BASE& base_message) {
    MSG_ITEM_UPGRADE& message = static_cast<MSG_ITEM_UPGRADE&>(base_message);
    if (packet.get_short(message.body_.target_slot) != 1) {
        return LineFunc(8846, kUpgradeItemSeparateReadSignature, 0, 0);
    }
    if (packet.get_int(message.body_.target_item_id) != 1) {
        return LineFunc(8848, kUpgradeItemSeparateReadSignature, 0, 0);
    }
    if (packet.get_short(message.body_.material_slot) != 1) {
        return LineFunc(8850, kUpgradeItemSeparateReadSignature, 0, 0);
    }

    unsigned int item_name_length = 0;
    if (packet.get_int(item_name_length) != 1) {
        return LineFunc(8858, kUpgradeItemSeparateReadSignature, 0, 0);
    }
    if (packet.get_str(message.body_.guild_item_name,
                       sizeof(message.body_.guild_item_name),
                       item_name_length) != 1) {
        return LineFunc(8861, kUpgradeItemSeparateReadSignature, 0, 0);
    }
    return 0;
}

// IDA_EA: 0x81CC106
// IDA_NAME: _ZN30Dispatcher_UpgradeItemSeparate7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_UpgradeItemSeparate::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_UpgradeItemSeparate::process(CUser* user,
                                            MSG_BASE& base_message,
                                            ParamBase& param) {
    const int error = check_error(user, base_message);
    if (error > 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_UPGRADE_ITEM_SEPARATE,
            static_cast<unsigned char>(error));
        return -1;
    }

    (void)param;
    G_CDataManager()->get_item_upgrade_separate()->item_upgrade_system(
        user,
        &static_cast<MSG_ITEM_UPGRADE&>(base_message),
        ENUM_CMDPACKET_UPGRADE_ITEM_SEPARATE);
    return 0;
}
