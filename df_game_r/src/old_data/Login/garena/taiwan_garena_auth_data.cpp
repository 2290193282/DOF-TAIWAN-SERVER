#include "old_data/Login/garena/taiwan_garena_auth_data.h"

#include <arpa/inet.h>
#include <cstring>
#include <ctime>

#include "common/manager/global_data.h"
#include "old_game/dungeon/instance/dungeon_pvp_packets.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

namespace Taiwan {

typedef char GarenaAuthData_Size_Must_Be_0x68[
    (sizeof(GarenaAuthData) == 0x68) ? 1 : -1];

// IDA_EA: 0x8173080
// IDA_NAME: _ZN6Taiwan20GarenaCheckTimeStampEjjjj
// DEMANGLED: Taiwan::GarenaCheckTimeStamp(unsigned int,unsigned int,unsigned int,unsigned int)
bool GarenaCheckTimeStamp(uint32_t token_time,
                          uint32_t current_time,
                          uint32_t login_time,
                          uint32_t last_play_time) {
    if (token_time + 300u > current_time) {
        return true;
    }
    if (token_time == login_time) {
        return last_play_time + 300u >= current_time;
    }
    return false;
}

// IDA_EA: 0x817305A
// IDA_NAME: _ZN6Taiwan15GarenaCyberCafeEt
// DEMANGLED: Taiwan::GarenaCyberCafe(unsigned short)
bool GarenaCyberCafe(uint16_t gca_type) {
    return (gca_type & 1u) != 0u;
}

// IDA_EA: 0x8471C74
// IDA_NAME: _ZN6Taiwan16GarenaAuthPCRoomEP5CUser
// DEMANGLED: Taiwan::GarenaAuthPCRoom(CUser *)
void GarenaAuthPCRoom(CUser* user) {
    if (!user) {
        return;
    }

    // Packet_Pcroom_Response C2 initializes its header, the two leading
    // dwords, and byte +0x1e.  The old caller then fills these exact wire
    // fields in the 12-byte middle span before handing the whole 0x1f-byte
    // record to the PCRoom proxy.
    Packet_Pcroom_Response response;
    response.dword_0a = static_cast<uint32_t>(user->GetUID());
    response.dword_0e = static_cast<uint32_t>(user->get_acc_id());

    const uint32_t auth_kind = 1u;
    const uint32_t expire_time =
        static_cast<uint32_t>(std::time(0) + 604800);
    std::memcpy(response.raw_12 + 0, &auth_kind, sizeof(auth_kind));
    std::memcpy(response.raw_12 + 4, &expire_time, sizeof(expire_time));
    response.raw_12[8] = 1u;
    response.raw_12[9] = 11u;

    Stream* const stream = GlobalData::stream_pool()->Acquire(
        "DF_PCRoomServerProxy.cpp", 427);
    CStreamGuard guard(stream, true);
    guard << 382;
    guard << user->GetUID();
    guard.put_binary(&response, static_cast<int>(sizeof(response)));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DISPATCH_Q, guard);
}

// IDA_EA: 0x8172FC8
// IDA_NAME: _ZN6Taiwan14GarenaAuthData6getUidEv
// DEMANGLED: Taiwan::GarenaAuthData::getUid(void)
uint32_t GarenaAuthData::getUid() { return ntohl(uid_be); }

// IDA_EA: 0x8172FDE
// IDA_NAME: _ZN6Taiwan14GarenaAuthData8getUnameEv
// DEMANGLED: Taiwan::GarenaAuthData::getUname(void)
char* GarenaAuthData::getUname() { return uname; }

// IDA_EA: 0x8172FEA
// IDA_NAME: _ZN6Taiwan14GarenaAuthData12getTimeStampEv
// DEMANGLED: Taiwan::GarenaAuthData::getTimeStamp(void)
uint32_t GarenaAuthData::getTimeStamp() {
    return patch_time + ntohl(timestamp_be);
}

// IDA_EA: 0x8173008
// IDA_NAME: _ZN6Taiwan14GarenaAuthData10getGcaTypeEv
// DEMANGLED: Taiwan::GarenaAuthData::getGcaType(void)
uint16_t GarenaAuthData::getGcaType() { return ntohs(gca_type_be); }

// IDA_EA: 0x8173022
// IDA_NAME: _ZN6Taiwan14GarenaAuthData13getMobileAuthEv
// DEMANGLED: Taiwan::GarenaAuthData::getMobileAuth(void)
bool GarenaAuthData::getMobileAuth() { return mobile_auth == 1; }

// IDA_EA: 0x8173034
// IDA_NAME: _ZN6Taiwan14GarenaAuthData12setPatchTimeEj
// DEMANGLED: Taiwan::GarenaAuthData::setPatchTime(unsigned int)
GarenaAuthData* GarenaAuthData::setPatchTime(uint32_t value) {
    patch_time = value;
    return this;
}

// IDA_EA: 0x8173042
// IDA_NAME: _ZN6Taiwan14GarenaAuthData12getPatchTimeEv
// DEMANGLED: Taiwan::GarenaAuthData::getPatchTime(void)
uint32_t GarenaAuthData::getPatchTime() { return patch_time; }

// IDA_EA: 0x817304E
// IDA_NAME: _ZN6Taiwan14GarenaAuthData17getGoldMemberTypeEv
// DEMANGLED: Taiwan::GarenaAuthData::getGoldMemberType(void)
int GarenaAuthData::getGoldMemberType() { return gold_member; }

}  // namespace Taiwan
