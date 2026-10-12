#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_BLACKLIST_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_BLACKLIST_H_

#include <stddef.h>
#include <stdint.h>

#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"

class CUser;
class PacketBuf;

// IDA unit: 08_quest_skill_growth/Packet_Request_BlackList.
// IDA_EA 0x822BBB2 constructs PacketHeader(0x5e0, 0x12); the dispatcher
// writes the account and UID fields immediately afterwards.
#pragma pack(push, 1)
struct Packet_Request_BlackList {
    // IDA_EA: 0x822BBB2
    // IDA_NAME: _ZN24Packet_Request_BlackListC2Ev
    // DEMANGLED: Packet_Request_BlackList::Packet_Request_BlackList(void)
    Packet_Request_BlackList();

    PacketHeader header;       // +0x00
    uint32_t account_id;       // +0x0a
    uint32_t uid;              // +0x0e
};
#pragma pack(pop)

static_assert(sizeof(Packet_Request_BlackList) == 0x12u,
              "Packet_Request_BlackList must retain its monitor ABI size");
static_assert(offsetof(Packet_Request_BlackList, account_id) == 0x0au,
              "Packet_Request_BlackList account offset changed");
static_assert(offsetof(Packet_Request_BlackList, uid) == 0x0eu,
              "Packet_Request_BlackList UID offset changed");

// The state-3 generic implementation is owned by the protocol-domain TU.
template <>
IPacketDispatcher<MSG_BASE,
                  stRequestBlackListParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_BASE,
                  stRequestBlackListParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::dispatch_sig(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_BASE,
                      stRequestBlackListParam,
                      static_cast<ch_state>(3)>::check_state(
    CUser* user,
    PacketBuf& packet);
template <>
void IPacketDispatcher<MSG_BASE,
                       stRequestBlackListParam,
                       static_cast<ch_state>(3)>::send(
    CUser* user,
    ParamBase& param);

// IDA unit: 08_quest_skill_growth/Dispatcher_RequestBlackList.
struct Dispatcher_RequestBlackList
    : public IPacketDispatcher<MSG_BASE,
                               stRequestBlackListParam,
                               static_cast<ch_state>(3)> {
    // IDA_EA: 0x85960C4
    // IDA_NAME: _ZN27Dispatcher_RequestBlackListC2Ev
    // DEMANGLED: Dispatcher_RequestBlackList::Dispatcher_RequestBlackList(void)
    Dispatcher_RequestBlackList();

    // IDA_EA: 0x8245DD8 / 0x8245E08
    // IDA_NAME: _ZN27Dispatcher_RequestBlackListD2Ev / D0Ev
    // DEMANGLED: Dispatcher_RequestBlackList::~Dispatcher_RequestBlackList()
    virtual ~Dispatcher_RequestBlackList();

    // IDA_EA: 0x81CE03C
    // IDA_NAME: _ZN27Dispatcher_RequestBlackList4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CE070
    // IDA_NAME: _ZN27Dispatcher_RequestBlackList7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CE046
    // IDA_NAME: _ZN27Dispatcher_RequestBlackList11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RequestBlackList) == 0x04u,
              "Dispatcher_RequestBlackList must retain its i386 ABI size");
#endif

#endif  // OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_REQUEST_BLACKLIST_H_
