#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_DISPATCHER_READS_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_DISPATCHER_READS_H_

#include "old_data/db/event/db_create_dnf_user_info.h"
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x81C3D46 in the companion source.


#include "old_game/economy/inventory/inventory_params.h"
#include "old_game/player/core/userinfo_messages.h"
#include "common/manager/taiwan_interface_inter_dispatcher.h"
#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class PacketBuf;
class CUser;
class Stream;

// IDA unit: 06_userinfo_stat_inventory/DisPatcher_GetUserInfo.
struct DisPatcher_GetUserInfo
    : public IPacketDispatcher<MSG_USERINFO_REQ,
                               stOnGetUserInfoParam,
                               static_cast<ch_state>(1)> {
    DisPatcher_GetUserInfo();
    virtual ~DisPatcher_GetUserInfo();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_GetUserInfo) == 0x04u,
              "DisPatcher_GetUserInfo must retain its i386 ABI size");
#endif

// IDA unit: 06_userinfo_stat_inventory/DisPatcher_UseEquip.
struct DisPatcher_UseEquip
    : public IPacketDispatcher<MSG_DECREASE_DURABILITY,
                               stOnUseEquipParam,
                               static_cast<ch_state>(0)> {
    DisPatcher_UseEquip();
    virtual ~DisPatcher_UseEquip();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_UseEquip) == 0x04u,
              "DisPatcher_UseEquip must retain its i386 ABI size");
#endif

// IDA unit: 06_userinfo_stat_inventory/DisPatcher_RepairEquip.
struct DisPatcher_RepairEquip
    : public IPacketDispatcher<MSG_REPAIR_EQUIP,
                               stRepairEquipParam,
                               static_cast<ch_state>(0)> {
    DisPatcher_RepairEquip();
    virtual ~DisPatcher_RepairEquip();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_RepairEquip) == 0x04u,
              "DisPatcher_RepairEquip must retain its i386 ABI size");
#endif

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_ImageCommunicationEquipmentUse.
struct Dispatcher_ImageCommunicationEquipmentUse
    : public IPacketDispatcher<MSG_BASE,
                               stImageCommunicationEquipmentUse,
                               static_cast<ch_state>(3)> {
    Dispatcher_ImageCommunicationEquipmentUse();
    virtual ~Dispatcher_ImageCommunicationEquipmentUse();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ImageCommunicationEquipmentUse) == 0x04u,
              "Dispatcher_ImageCommunicationEquipmentUse must retain its i386 ABI size");
#endif

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_UpgradeCargo.
struct Dispatcher_UpgradeCargo
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
    Dispatcher_UpgradeCargo();
    virtual ~Dispatcher_UpgradeCargo();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpgradeCargo) == 0x04u,
              "Dispatcher_UpgradeCargo must retain its i386 ABI size");
#endif

// IDA unit: 06_userinfo_stat_inventory/Dispatcher_UpgradeInventory.
struct Dispatcher_UpgradeInventory
    : public IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)> {
    Dispatcher_UpgradeInventory();
    virtual ~Dispatcher_UpgradeInventory();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_UpgradeInventory) == 0x04u,
              "Dispatcher_UpgradeInventory must retain its i386 ABI size");
#endif


// IDA unit: 06_userinfo_stat_inventory/InterSelectCreateDnfUserInfo.
// IDA_EA: dispatch/makeRequest @0x8161012/@0x8161218.
struct InterSelectCreateDnfUserInfo : public TaiwanInterfaceInterDispatcher {
    // IDA_EA: 0x816208C
    // IDA_NAME: _ZN28InterSelectCreateDnfUserInfoC2Ev
    // DEMANGLED: InterSelectCreateDnfUserInfo::InterSelectCreateDnfUserInfo(void)
    InterSelectCreateDnfUserInfo();

    // IDA_EA: 0x81620B0
    // IDA_NAME: _ZN28InterSelectCreateDnfUserInfoD2Ev
    // DEMANGLED: InterSelectCreateDnfUserInfo::~InterSelectCreateDnfUserInfo()
    // IDA_EA: 0x81620E0
    // IDA_NAME: _ZN28InterSelectCreateDnfUserInfoD0Ev
    // DEMANGLED: InterSelectCreateDnfUserInfo::~InterSelectCreateDnfUserInfo()
    virtual ~InterSelectCreateDnfUserInfo();

    struct Msg {
        unsigned int account_id;
        unsigned char received;
        unsigned char reserved_05[3];
    };

    virtual int dispatch_sig(CUser* user, char* msg, int length);
    static void makeRequest(int uid, unsigned int account_id, bool received);
};

#if defined(__i386__)
static_assert(sizeof(InterSelectCreateDnfUserInfo) == 0x08u,
              "InterSelectCreateDnfUserInfo must retain the Taiwan ABI");
#endif


#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_USERINFO_DISPATCHER_READS_H_
