#include "old_game/social/friend/friend_dispatcher.h"

#include <algorithm>
#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/gm_accounts_abi.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "infra/log/format/trace.h"
#include "old_data/db/legacy_dispatch/db_check_add_friend.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

// IDA_EA: 0x8230F8A
// IDA_NAME: _ZN19stRemoveFriendParamC2Ev
// DEMANGLED: stRemoveFriendParam::stRemoveFriendParam(void)
stRemoveFriendParam::stRemoveFriendParam() : ParamBase(), result(0) {}

// IDA_EA: 0x8247C1E
// IDA_NAME: _ZN19stRemoveFriendParamD2Ev
// IDA_EA: 0x8247C4E
// IDA_NAME: _ZN19stRemoveFriendParamD0Ev
// DEMANGLED: stRemoveFriendParam::~stRemoveFriendParam()
stRemoveFriendParam::~stRemoveFriendParam() {}

// IDA_EA: 0x8230F64
// IDA_NAME: _ZN16stAddFriendParamC2Ev
// DEMANGLED: stAddFriendParam::stAddFriendParam(void)
stAddFriendParam::stAddFriendParam() : ParamBase(), result(0) {}

// IDA_EA: 0x8247C6C
// IDA_NAME: _ZN16stAddFriendParamD2Ev
// IDA_EA: 0x8247C9C
// IDA_NAME: _ZN16stAddFriendParamD0Ev
// DEMANGLED: stAddFriendParam::~stAddFriendParam()
stAddFriendParam::~stAddFriendParam() {}

// IDA_EA: 0x822BCB2
// IDA_NAME: _ZN24Packet_Monitor_Del_BuddyC2Ev
// DEMANGLED: Packet_Monitor_Del_Buddy::Packet_Monitor_Del_Buddy(void)
Packet_Monitor_Del_Buddy::Packet_Monitor_Del_Buddy()
    : header(0x674u, 0x30u), character_no(0), uid(0), character_name() {}

// IDA_EA: 0x84E9360
// IDA_NAME: _ZN24Packet_Monitor_Add_BuddyC2Ev
// DEMANGLED: Packet_Monitor_Add_Buddy::Packet_Monitor_Add_Buddy(void)
Packet_Monitor_Add_Buddy::Packet_Monitor_Add_Buddy()
    : header(0x672u, 0x30u), character_no(0), uid(0), character_name() {}

namespace {

int BoundedCStringLength(const char* text, int maximum_length) {
    int length = 0;
    while (length < maximum_length && text[length]) {
        ++length;
    }
    return length;
}

}  // namespace

// IDA_EA: 0x84E8A42
// IDA_NAME: _ZN20Inter_CheckAddFriend12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CheckAddFriend::dispatch_sig(CUser *,char *,int)
int Inter_CheckAddFriend::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!user || !payload || payload_size < 5) {
        cMyTrace trace(
            "virtual int Inter_CheckAddFriend::dispatch_sig(CUser*, char*, int)",
            28027, 5);
        trace("[%s] pUser or p is null.",
              "virtual int Inter_CheckAddFriend::dispatch_sig(CUser*, char*, int)");
        return 28028;
    }

    int result = 0;
    std::memcpy(&result, payload, sizeof(result));
    if (result != 0) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_ADD_FRIEND,
                                 static_cast<unsigned char>(result));
        return 0;
    }

    Packet_Monitor_Add_Buddy monitor_packet;
    monitor_packet.character_no = static_cast<uint32_t>(user->getCurCharacNo());
    monitor_packet.uid = static_cast<uint32_t>(user->GetUID());
    const int name_length = BoundedCStringLength(payload + 5,
                                                  payload_size - 5);
    const int stored_length = std::min(
        name_length, static_cast<int>(sizeof(monitor_packet.character_name)));
    std::memcpy(monitor_packet.character_name, payload + 5,
                static_cast<size_t>(stored_length));
    GlobalData::s_monitor_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))->SendPacket(
        reinterpret_cast<char*>(&monitor_packet), monitor_packet.header.size);
    return 0;
}

// IDA_EA: 0x84EEED8 / 0x84F57FA / 0x84F582A
Inter_CheckAddFriend::Inter_CheckAddFriend() : ISigDispatcher() {}
Inter_CheckAddFriend::~Inter_CheckAddFriend() {}

// IDA_EA: 0x81CB52E
// IDA_NAME: _ZN23DisPatcher_RemoveFriend4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_RemoveFriend::read(PacketBuf &,MSG_BASE &)
int DisPatcher_RemoveFriend::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_REMOVE_FRIEND& request =
        reinterpret_cast<MSG_REMOVE_FRIEND&>(message);
    if (packet.get_int(request.character_name_length) != 1) {
        return LineFunc(
            9934,
            "virtual int DisPatcher_RemoveFriend::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_str(request.character_name,
                       sizeof(request.character_name),
                       request.character_name_length) == 1) {
        return 0;
    }
    return LineFunc(
        9937,
        "virtual int DisPatcher_RemoveFriend::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CB5D8
// IDA_NAME: _ZN23DisPatcher_RemoveFriend11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_RemoveFriend::check_error(CUser *,MSG_BASE &)
int DisPatcher_RemoveFriend::check_error(CUser* user, MSG_BASE& message) {
    if (user->get_state() <= 2) {
        return 0x7fffffff;
    }

    // The original -O0 i386 code checks the address of the in-place name
    // storage (+0x11) rather than its contents.  Preserve that exact branch
    // contract; a non-null message reference consequently produces zero.
    MSG_REMOVE_FRIEND& request =
        reinterpret_cast<MSG_REMOVE_FRIEND&>(message);
    return request.character_name == 0 ? 1 : 0;
}

// IDA_EA: 0x81CB618
// IDA_NAME: _ZN23DisPatcher_RemoveFriend7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_RemoveFriend::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_RemoveFriend::process(CUser* user,
                                     MSG_BASE& message,
                                     ParamBase& param) {
    stRemoveFriendParam& result =
        reinterpret_cast<stRemoveFriendParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        return LineFunc(
            8513,
            "virtual int DisPatcher_RemoveFriend::process(CUser*, MSG_BASE&, ParamBase&)",
            result.result,
            user->get_acc_id());
    }

    const MSG_REMOVE_FRIEND& request =
        reinterpret_cast<const MSG_REMOVE_FRIEND&>(message);
    Packet_Monitor_Del_Buddy monitor_packet;
    monitor_packet.character_no = user->getCurCharacNo();
    monitor_packet.uid = user->GetUID();
    const size_t name_length = std::strlen(request.character_name);
    std::memcpy(monitor_packet.character_name,
                request.character_name,
                name_length);

    CMonitorServerProxy* const proxy =
        GlobalData::s_monitor_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
    proxy->SendPacket(reinterpret_cast<char*>(&monitor_packet),
                      monitor_packet.header.size);
    return 0;
}

// IDA_EA: 0x81CB738
// IDA_NAME: _ZN23DisPatcher_RemoveFriend4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_RemoveFriend::send(CUser *,ParamBase &)
void DisPatcher_RemoveFriend::send(CUser* user, ParamBase& param) {
    const stRemoveFriendParam& result =
        reinterpret_cast<const stRemoveFriendParam&>(param);
    if (result.result != 0x7fffffff && result.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REMOVE_FRIEND,
            static_cast<unsigned char>(result.result));
    }
}

// IDA_EA: 0x824621C
// IDA_NAME: _ZN23DisPatcher_RemoveFriendD2Ev
// IDA_EA: 0x824624C
// IDA_NAME: _ZN23DisPatcher_RemoveFriendD0Ev
// DEMANGLED: DisPatcher_RemoveFriend::~DisPatcher_RemoveFriend()
DisPatcher_RemoveFriend::~DisPatcher_RemoveFriend() {}

// IDA_EA: 0x8595B14
// IDA_NAME: _ZN23DisPatcher_RemoveFriendC2Ev
// DEMANGLED: DisPatcher_RemoveFriend::DisPatcher_RemoveFriend(void)
DisPatcher_RemoveFriend::DisPatcher_RemoveFriend()
    : IPacketDispatcher<MSG_REMOVE_FRIEND,
                        stRemoveFriendParam,
                        static_cast<ch_state>(3)>() {}

// IDA_EA: 0x81CB1CA
// IDA_NAME: _ZN20DisPatcher_AddFriend4readER9PacketBufR8MSG_BASE
// DEMANGLED: DisPatcher_AddFriend::read(PacketBuf &,MSG_BASE &)
int DisPatcher_AddFriend::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_ADD_FRIEND& request = reinterpret_cast<MSG_ADD_FRIEND&>(message);
    if (packet.get_int(request.character_name_length) != 1) {
        return LineFunc(
            8315,
            "virtual int DisPatcher_AddFriend::read(PacketBuf&, MSG_BASE&)",
            0,
            0);
    }
    if (packet.get_str(request.character_name,
                       sizeof(request.character_name),
                       request.character_name_length) == 1) {
        return 0;
    }
    return LineFunc(
        8322,
        "virtual int DisPatcher_AddFriend::read(PacketBuf&, MSG_BASE&)",
        0,
        0);
}

// IDA_EA: 0x81CB274
// IDA_NAME: _ZN20DisPatcher_AddFriend11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: DisPatcher_AddFriend::check_error(CUser *,MSG_BASE &)
int DisPatcher_AddFriend::check_error(CUser* user, MSG_BASE& message) {
    if (user->get_state() <= 2) {
        return 0x7fffffff;
    }

    MSG_ADD_FRIEND& request = reinterpret_cast<MSG_ADD_FRIEND&>(message);
    // IDA 0x81CB5FA--0x81CB611 checks the address of the in-place request
    // name storage.  This is an observed ABI branch, not a validation of the
    // character string itself.
    if (request.character_name == 0) {
        return 1;
    }
    return std::strcmp(request.character_name, user->getCurCharacName()) == 0;
}

// IDA_EA: 0x81CB2E2
// IDA_NAME: _ZN20DisPatcher_AddFriend7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_AddFriend::process(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_AddFriend::process(CUser* user,
                                  MSG_BASE& message,
                                  ParamBase& param) {
    return process_packet_jpn(user, message, param);
}

// IDA_EA: 0x81CB30A
// IDA_NAME: _ZN20DisPatcher_AddFriend18process_packet_jpnEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: DisPatcher_AddFriend::process_packet_jpn(CUser *,MSG_BASE &,ParamBase &)
int DisPatcher_AddFriend::process_packet_jpn(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    stAddFriendParam& result = reinterpret_cast<stAddFriendParam&>(param);
    result.result = check_error(user, message);
    if (result.result > 0) {
        return 0;
    }
    if (result.result < 0) {
        LogManager::logFormat(
            1,
            "PacketDispatcher_Impl_1.cpp",
            "int DisPatcher_AddFriend::process_packet_jpn(CUser*, MSG_BASE&, ParamBase&)",
            8408,
            "Dispatcher_Impl Check_Error No(%d)",
            result.result);
        return 8409;
    }

    const MSG_ADD_FRIEND& request =
        reinterpret_cast<const MSG_ADD_FRIEND&>(message);
    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "PacketDispatcher_Impl_1.cpp", 8429);
    CStreamGuard guard(stream, true);
    *guard << 0x33e;
    *guard << user->GetUID();
    SIG_CHECK_ADD_FRIEND* const db_request =
        guard->GetInBuffer<SIG_CHECK_ADD_FRIEND>();
    if (!db_request) {
        result.result = 1;
        return 0;
    }

    db_request->check_name =
        !GlobalData::s_pGMAccounts_->isGM(user->get_acc_id());
    std::strncpy(db_request->character_name,
                 request.character_name,
                 sizeof(db_request->character_name));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x81CB4E6
// IDA_NAME: _ZN20DisPatcher_AddFriend4sendEP5CUserR9ParamBase
// DEMANGLED: DisPatcher_AddFriend::send(CUser *,ParamBase &)
void DisPatcher_AddFriend::send(CUser* user, ParamBase& param) {
    const stAddFriendParam& result =
        reinterpret_cast<const stAddFriendParam&>(param);
    if (result.result != 0x7fffffff && result.result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_ADD_FRIEND,
            static_cast<unsigned char>(result.result));
    }
}

// IDA_EA: 0x824626A
// IDA_NAME: _ZN20DisPatcher_AddFriendD2Ev
// IDA_EA: 0x824629A
// IDA_NAME: _ZN20DisPatcher_AddFriendD0Ev
// DEMANGLED: DisPatcher_AddFriend::~DisPatcher_AddFriend()
DisPatcher_AddFriend::~DisPatcher_AddFriend() {}

// IDA_EA: 0x8595AF8
// IDA_NAME: _ZN20DisPatcher_AddFriendC2Ev
// DEMANGLED: DisPatcher_AddFriend::DisPatcher_AddFriend(void)
DisPatcher_AddFriend::DisPatcher_AddFriend()
    : IPacketDispatcher<MSG_ADD_FRIEND,
                        stAddFriendParam,
                        static_cast<ch_state>(3)>() {}
