#include "old_game/social/guild/guild_double_check_dispatchers.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_game/social/guild/guild_url_validation.h"
#include "infra/security/policy/packet_dispatcher_guard.h"
#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"


// IDA_EA: 0x85963B8
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleC2Ev
// DEMANGLED: Dispatcher_CheckGuildNameDouble::Dispatcher_CheckGuildNameDouble(void)
Dispatcher_CheckGuildNameDouble::Dispatcher_CheckGuildNameDouble() {
}

// IDA_EA: 0x8242442
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleD2Ev
// DEMANGLED: Dispatcher_CheckGuildNameDouble::~Dispatcher_CheckGuildNameDouble()
// IDA_EA: 0x8242472
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDoubleD0Ev
// DEMANGLED: Dispatcher_CheckGuildNameDouble::~Dispatcher_CheckGuildNameDouble()
Dispatcher_CheckGuildNameDouble::~Dispatcher_CheckGuildNameDouble() {
}

// IDA_EA: 0x82084E8
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser* user,
                                                  PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            41833,
            "virtual int Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    int text_length = 0;
    if (packet.get_int(text_length) != 1) {
        return LineFunc(
            41854,
            "virtual int Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (static_cast<unsigned int>(text_length) > 0x16u) {
        return LineFunc(
            41857,
            "virtual int Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char* const guild_name = user->getCreateGuildName();
    if (!guild_name) {
        return LineFunc(
            41862,
            "virtual int Dispatcher_CheckGuildNameDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_str(guild_name, 23, text_length) != 1) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE, 106);
        return 0;
    }
    if (!checkValidCharacName(guild_name)) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHECK_GUILD_NAME_DOUBLE, 108);
        return 0;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "PacketDispatcher_Impl_1.cpp", 41885);
    CStreamGuard guard(stream, true);
    *guard << 0xEB;
    *guard << user->GetUID();
    SIG_DOUBLE_CHECK_GUILD_NAME* const request =
        guard->GetInBuffer<SIG_DOUBLE_CHECK_GUILD_NAME>();
    std::memset(request, 0, sizeof(*request));
    std::memcpy(request, guild_name, static_cast<size_t>(text_length));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x82339DE
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CheckGuildNameDouble::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CheckGuildNameDouble::read(PacketBuf& packet,
                                          MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82339E8
// IDA_NAME: _ZN31Dispatcher_CheckGuildNameDouble7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CheckGuildNameDouble::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CheckGuildNameDouble::process(CUser* user,
                                             MSG_BASE& message,
                                             ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}

// IDA_EA: 0x85963D4
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleC2Ev
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::Dispatcher_CheckGuildAddressDouble(void)
Dispatcher_CheckGuildAddressDouble::Dispatcher_CheckGuildAddressDouble() {
}

// IDA_EA: 0x82423F4
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleD2Ev
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::~Dispatcher_CheckGuildAddressDouble()
// IDA_EA: 0x8242424
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDoubleD0Ev
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::~Dispatcher_CheckGuildAddressDouble()
Dispatcher_CheckGuildAddressDouble::~Dispatcher_CheckGuildAddressDouble() {
}

// IDA_EA: 0x82087A4
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser* user,
                                                     PacketBuf& packet) {
    if (user->get_state() <= 2 || !user->getCurCharacR()) {
        return LineFunc(
            41903,
            "virtual int Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    int text_length = 0;
    if (packet.get_int(text_length) != 1) {
        return LineFunc(
            41918,
            "virtual int Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }

    char* const guild_address = user->getCreateGuildAddr();
    if (!guild_address) {
        return LineFunc(
            41922,
            "virtual int Dispatcher_CheckGuildAddressDouble::dispatch_sig(CUser*, PacketBuf&)",
            0,
            0);
    }
    if (packet.get_str(guild_address, 13, text_length) != 1) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE, 110);
        return 0;
    }
    if (check_url(guild_address) != 1) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHECK_GUILD_ADDREASS_DOUBLE, 112);
        return 0;
    }

    Stream* const stream = GlobalData::s_stream_pool->Acquire(
        "PacketDispatcher_Impl_1.cpp", 41937);
    CStreamGuard guard(stream, true);
    *guard << 0xEC;
    *guard << user->GetUID();
    SIG_DOUBLE_CHECK_GUILD_ADDRESS* const request =
        guard->GetInBuffer<SIG_DOUBLE_CHECK_GUILD_ADDRESS>();
    std::memset(request, 0, sizeof(*request));
    std::memcpy(request, guild_address, static_cast<size_t>(text_length));
    GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, guard);
    return 0;
}

// IDA_EA: 0x82339F2
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::read(PacketBuf &,MSG_BASE &)
int Dispatcher_CheckGuildAddressDouble::read(PacketBuf& packet,
                                             MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x82339FC
// IDA_NAME: _ZN34Dispatcher_CheckGuildAddressDouble7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_CheckGuildAddressDouble::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_CheckGuildAddressDouble::process(CUser* user,
                                                MSG_BASE& message,
                                                ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
