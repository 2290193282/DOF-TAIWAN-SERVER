#ifndef OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BLACKLIST_REGISTER_DELETE_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BLACKLIST_REGISTER_DELETE_DISPATCHERS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"
#include "old_game/progression/quest/quest_skill_messages.h"
#include "old_game/progression/quest/quest_skill_params.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// IDA units: 99_misc_global/Packet_Register_To_BlackList and
// Packet_Delete_To_BlackList.  Their constructors leave account_id untouched
// and zero only the 30-byte character-name buffer.
#pragma pack(push, 1)
struct Packet_Register_To_BlackList {
    Packet_Register_To_BlackList();

    PacketHeader header;   // +0x00, type 0x5dc, size 0x2c
    uint32_t account_id;   // +0x0a, written by CMD121
    char target_name[30];  // +0x0e, zeroed by C2
};

struct Packet_Delete_To_BlackList {
    Packet_Delete_To_BlackList();

    PacketHeader header;   // +0x00, type 0x5dd, size 0x2c
    uint32_t account_id;   // +0x0a, written by CMD122
    char target_name[30];  // +0x0e, zeroed by C2
};

// The monitor reply preserves the request prefix and appends the target
// character number and operation result.  Both real Inter handlers read only
// +0x0e, +0x2c and +0x30, but retaining the prefix documents the on-wire ABI.
struct Monitor_BlackList_Result {
    PacketHeader header;   // +0x00
    uint32_t account_id;   // +0x0a
    char target_name[30];  // +0x0e
    uint32_t charac_no;    // +0x2c
    uint8_t result;        // +0x30
};
#pragma pack(pop)

static_assert(sizeof(Packet_Register_To_BlackList) == 0x2cu &&
                  sizeof(Packet_Delete_To_BlackList) == 0x2cu,
              "CMD121/122 monitor packet extent changed");
static_assert(offsetof(Packet_Register_To_BlackList, account_id) == 0x0au &&
                  offsetof(Packet_Register_To_BlackList, target_name) == 0x0eu &&
                  offsetof(Monitor_BlackList_Result, charac_no) == 0x2cu &&
                  offsetof(Monitor_BlackList_Result, result) == 0x30u,
              "CMD121/122 monitor packet offsets changed");

class Dispatcher_RegisterToBlackList
    : public IPacketDispatcher<MSG_REGISTER_TO_BLACKLIST,
                               stRegisterToBlackListParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x859608C
    // IDA_NAME: _ZN30Dispatcher_RegisterToBlackListC2Ev
    // DEMANGLED: Dispatcher_RegisterToBlackList::Dispatcher_RegisterToBlackList(void)
    Dispatcher_RegisterToBlackList();

    // IDA_EA: 0x8245E74 / 0x8245EA4
    // IDA_NAME: _ZN30Dispatcher_RegisterToBlackListD2Ev /
    //           _ZN30Dispatcher_RegisterToBlackListD0Ev
    // DEMANGLED: Dispatcher_RegisterToBlackList::~Dispatcher_RegisterToBlackList()
    virtual ~Dispatcher_RegisterToBlackList();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

class Dispatcher_DeleteToBlackList
    : public IPacketDispatcher<MSG_DELETE_TO_BLACKLIST,
                               stDeleteToBlackListParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x85960A8
    // IDA_NAME: _ZN28Dispatcher_DeleteToBlackListC2Ev
    // DEMANGLED: Dispatcher_DeleteToBlackList::Dispatcher_DeleteToBlackList(void)
    Dispatcher_DeleteToBlackList();

    // IDA_EA: 0x8245E26 / 0x8245E56
    // IDA_NAME: _ZN28Dispatcher_DeleteToBlackListD2Ev /
    //           _ZN28Dispatcher_DeleteToBlackListD0Ev
    // DEMANGLED: Dispatcher_DeleteToBlackList::~Dispatcher_DeleteToBlackList()
    virtual ~Dispatcher_DeleteToBlackList();

    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
    int check_error(CUser* user, MSG_BASE& message);
};

// IDA units: 99_misc_global/Inter_Monitor_{Register,Delete}BlackListResult.
// These handlers have no registration owner yet; the complete InterDispatcher
// table remains deliberately outside this derived-object closure.
class Inter_Monitor_RegisterBlackListResult : public ISigDispatcher {
public:
    Inter_Monitor_RegisterBlackListResult();
    virtual ~Inter_Monitor_RegisterBlackListResult();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Monitor_DeleteBlackListResult : public ISigDispatcher {
public:
    Inter_Monitor_DeleteBlackListResult();
    virtual ~Inter_Monitor_DeleteBlackListResult();

    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_RegisterToBlackList) == 0x04u &&
                  sizeof(Dispatcher_DeleteToBlackList) == 0x04u &&
                  sizeof(Inter_Monitor_RegisterBlackListResult) == 0x04u &&
                  sizeof(Inter_Monitor_DeleteBlackListResult) == 0x04u,
              "CMD121/122 handler objects must retain their vptr-only ABI");
#endif

#endif  // OLDSRV_OLD_DATA_LOGIN_PACKET_DISPATCH_BLACKLIST_REGISTER_DELETE_DISPATCHERS_H_
