#ifndef OLDSRV_OLD_GAME_GROUP_FRIEND_DISPATCHER_H_
#define OLDSRV_OLD_GAME_GROUP_FRIEND_DISPATCHER_H_

#include <stddef.h>
#include <stdint.h>

#include "infra/network/packet/message_base.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
#include "infra/network/packet/PacketHeader.h"
#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;
class PacketBuf;

// IDA packet body consumed by DisPatcher_RemoveFriend::read at 0x81CB52E.
// The request's client header is 13 bytes; the length and bounded target
// character name immediately follow it.
#pragma pack(push, 1)
struct MSG_REMOVE_FRIEND : public MSG_BASE {
    int32_t character_name_length;  // +0x0d
    char character_name[30];        // +0x11
};
#pragma pack(pop)

static_assert(sizeof(MSG_REMOVE_FRIEND) == 0x2fu,
              "MSG_REMOVE_FRIEND must retain its IDA extent");
static_assert(offsetof(MSG_REMOVE_FRIEND, character_name_length) == 0x0du,
              "MSG_REMOVE_FRIEND length offset changed");
static_assert(offsetof(MSG_REMOVE_FRIEND, character_name) == 0x11u,
              "MSG_REMOVE_FRIEND name offset changed");

// IDA packet body consumed by DisPatcher_AddFriend::read at 0x81CB1CA.
// It is a distinct type despite sharing the wire layout with CMD64: the
// original template instantiations carry different C++ type identities.
#pragma pack(push, 1)
struct MSG_ADD_FRIEND : public MSG_BASE {
    int32_t character_name_length;  // +0x0d
    char character_name[30];        // +0x11
};
#pragma pack(pop)

static_assert(sizeof(MSG_ADD_FRIEND) == 0x2fu,
              "MSG_ADD_FRIEND must retain its IDA extent");
static_assert(offsetof(MSG_ADD_FRIEND, character_name_length) == 0x0du,
              "MSG_ADD_FRIEND length offset changed");
static_assert(offsetof(MSG_ADD_FRIEND, character_name) == 0x11u,
              "MSG_ADD_FRIEND name offset changed");

// IDA unit: 99_misc_global/stRemoveFriendParam.
struct stRemoveFriendParam : public ParamBase {
    // IDA_EA: 0x8230F8A
    // IDA_NAME: _ZN19stRemoveFriendParamC2Ev
    // DEMANGLED: stRemoveFriendParam::stRemoveFriendParam(void)
    stRemoveFriendParam();

    // IDA_EA: 0x8247C1E
    // IDA_NAME: _ZN19stRemoveFriendParamD2Ev
    // IDA_EA: 0x8247C4E
    // IDA_NAME: _ZN19stRemoveFriendParamD0Ev
    // DEMANGLED: stRemoveFriendParam::~stRemoveFriendParam()
    virtual ~stRemoveFriendParam();

    int result;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stRemoveFriendParam) == 0x08u,
              "stRemoveFriendParam must retain its i386 ABI size");
#endif

// IDA unit: 99_misc_global/stAddFriendParam.
struct stAddFriendParam : public ParamBase {
    // IDA_EA: 0x8230F64
    // IDA_NAME: _ZN16stAddFriendParamC2Ev
    // DEMANGLED: stAddFriendParam::stAddFriendParam(void)
    stAddFriendParam();

    // IDA_EA: 0x8247C6C
    // IDA_NAME: _ZN16stAddFriendParamD2Ev
    // IDA_EA: 0x8247C9C
    // IDA_NAME: _ZN16stAddFriendParamD0Ev
    // DEMANGLED: stAddFriendParam::~stAddFriendParam()
    virtual ~stAddFriendParam();

    int result;  // +0x04
};

#if defined(__i386__)
static_assert(sizeof(stAddFriendParam) == 0x08u,
              "stAddFriendParam must retain its i386 ABI size");
#endif

// IDA unit: 99_misc_global/Packet_Monitor_Del_Buddy.  PacketHeader is the
// real ten-byte monitor header; the remaining members are written by the
// dispatcher before the monitor proxy sends exactly header.size bytes.
#pragma pack(push, 1)
struct Packet_Monitor_Del_Buddy {
    // IDA_EA: 0x822BCB2
    // IDA_NAME: _ZN24Packet_Monitor_Del_BuddyC2Ev
    // DEMANGLED: Packet_Monitor_Del_Buddy::Packet_Monitor_Del_Buddy(void)
    Packet_Monitor_Del_Buddy();

    PacketHeader header;        // +0x00, type 0x674, size 0x30
    uint32_t character_no;      // +0x0a
    uint32_t uid;               // +0x0e
    char character_name[30];    // +0x12
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Del_Buddy) == 0x30u,
              "Packet_Monitor_Del_Buddy must retain its monitor ABI size");
static_assert(offsetof(Packet_Monitor_Del_Buddy, character_no) == 0x0au,
              "Packet_Monitor_Del_Buddy character offset changed");
static_assert(offsetof(Packet_Monitor_Del_Buddy, uid) == 0x0eu,
              "Packet_Monitor_Del_Buddy uid offset changed");
static_assert(offsetof(Packet_Monitor_Del_Buddy, character_name) == 0x12u,
              "Packet_Monitor_Del_Buddy name offset changed");

// IDA unit: 99_misc_global/Packet_Monitor_Add_Buddy.  It intentionally has
// the same 48-byte body layout as delete-buddy but a distinct monitor packet
// id and C++ type identity.
#pragma pack(push, 1)
struct Packet_Monitor_Add_Buddy {
    Packet_Monitor_Add_Buddy();

    PacketHeader header;        // +0x00, type 0x672, size 0x30
    uint32_t character_no;      // +0x0a
    uint32_t uid;               // +0x0e
    char character_name[30];    // +0x12
};
#pragma pack(pop)

static_assert(sizeof(Packet_Monitor_Add_Buddy) == 0x30u,
              "Packet_Monitor_Add_Buddy must retain its monitor ABI size");
static_assert(offsetof(Packet_Monitor_Add_Buddy, character_no) == 0x0au &&
                  offsetof(Packet_Monitor_Add_Buddy, uid) == 0x0eu &&
                  offsetof(Packet_Monitor_Add_Buddy, character_name) == 0x12u,
              "Packet_Monitor_Add_Buddy field offsets changed");

// Slot 830: reply to DB_CheckAddFriend, then forward a successful request to
// the monitor buddy service for the user's server group.
class Inter_CheckAddFriend : public ISigDispatcher {
public:
    Inter_CheckAddFriend();
    virtual ~Inter_CheckAddFriend();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_CheckAddFriend) == 0x04u,
              "Inter_CheckAddFriend must remain base-only");
#endif

// Exact A-domain instantiation declarations.  The implementation belongs to
// packet_dispatcher_friend_templates.cpp; declaring it here prevents C-domain
// derived-object TUs from implicitly instantiating a competing template ABI.
template <>
IPacketDispatcher<MSG_REMOVE_FRIEND,
                  stRemoveFriendParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_REMOVE_FRIEND,
                  stRemoveFriendParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
                      static_cast<ch_state>(3)>::dispatch_sig(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_REMOVE_FRIEND,
                      stRemoveFriendParam,
                      static_cast<ch_state>(3)>::check_state(
    CUser* user,
    PacketBuf& packet);
template <>
void IPacketDispatcher<MSG_REMOVE_FRIEND,
                       stRemoveFriendParam,
                       static_cast<ch_state>(3)>::send(
    CUser* user,
    ParamBase& param);

template <>
IPacketDispatcher<MSG_ADD_FRIEND,
                  stAddFriendParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();
template <>
IPacketDispatcher<MSG_ADD_FRIEND,
                  stAddFriendParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
                      static_cast<ch_state>(3)>::dispatch_sig(
    CUser* user,
    PacketBuf& packet);
template <>
int IPacketDispatcher<MSG_ADD_FRIEND,
                      stAddFriendParam,
                      static_cast<ch_state>(3)>::check_state(
    CUser* user,
    PacketBuf& packet);
template <>
void IPacketDispatcher<MSG_ADD_FRIEND,
                       stAddFriendParam,
                       static_cast<ch_state>(3)>::send(
    CUser* user,
    ParamBase& param);

// IDA unit: 13_database_dispatch/DisPatcher_RemoveFriend.
struct DisPatcher_RemoveFriend
    : public IPacketDispatcher<MSG_REMOVE_FRIEND,
                               stRemoveFriendParam,
                               static_cast<ch_state>(3)> {
    // IDA_EA: 0x8595B14
    // IDA_NAME: _ZN23DisPatcher_RemoveFriendC2Ev
    // DEMANGLED: DisPatcher_RemoveFriend::DisPatcher_RemoveFriend(void)
    DisPatcher_RemoveFriend();

    // IDA_EA: 0x824621C / 0x824624C
    // IDA_NAME: _ZN23DisPatcher_RemoveFriendD2Ev / D0Ev
    // DEMANGLED: DisPatcher_RemoveFriend::~DisPatcher_RemoveFriend()
    virtual ~DisPatcher_RemoveFriend();

    // IDA_EA: 0x81CB52E
    // IDA_NAME: _ZN23DisPatcher_RemoveFriend4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CB618
    // IDA_NAME: _ZN23DisPatcher_RemoveFriend7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CB5D8
    // IDA_NAME: _ZN23DisPatcher_RemoveFriend11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81CB738
    // IDA_NAME: _ZN23DisPatcher_RemoveFriend4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_RemoveFriend) == 0x04u,
              "DisPatcher_RemoveFriend must retain its i386 ABI size");
#endif

// IDA unit: 13_database_dispatch/DisPatcher_AddFriend.
struct DisPatcher_AddFriend
    : public IPacketDispatcher<MSG_ADD_FRIEND,
                               stAddFriendParam,
                               static_cast<ch_state>(3)> {
    // IDA_EA: 0x8595AF8
    // IDA_NAME: _ZN20DisPatcher_AddFriendC2Ev
    // DEMANGLED: DisPatcher_AddFriend::DisPatcher_AddFriend(void)
    DisPatcher_AddFriend();

    // IDA_EA: 0x824626A / 0x824629A
    // IDA_NAME: _ZN20DisPatcher_AddFriendD2Ev / D0Ev
    // DEMANGLED: DisPatcher_AddFriend::~DisPatcher_AddFriend()
    virtual ~DisPatcher_AddFriend();

    // IDA_EA: 0x81CB1CA
    // IDA_NAME: _ZN20DisPatcher_AddFriend4readER9PacketBufR8MSG_BASE
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81CB2E2
    // IDA_NAME: _ZN20DisPatcher_AddFriend7processEP5CUserR8MSG_BASER9ParamBase
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CB30A
    // IDA_NAME: _ZN20DisPatcher_AddFriend18process_packet_jpnEP5CUserR8MSG_BASER9ParamBase
    int process_packet_jpn(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81CB274
    // IDA_NAME: _ZN20DisPatcher_AddFriend11check_errorEP5CUserR8MSG_BASE
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81CB4E6
    // IDA_NAME: _ZN20DisPatcher_AddFriend4sendEP5CUserR9ParamBase
    virtual void send(CUser* user, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(DisPatcher_AddFriend) == 0x04u,
              "DisPatcher_AddFriend must retain its i386 ABI size");
#endif

#endif  // OLDSRV_OLD_GAME_GROUP_FRIEND_DISPATCHER_H_
