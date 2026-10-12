#include "old_data/Login/inter_returns/inter_guild_name_return_dispatchers.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "infra/log/format/log_manager.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D5072
// IDA_NAME: _ZN21Inter_ChangeGuildName12dispatch_sigEP5CUserPci
int Inter_ChangeGuildName::dispatch_sig(CUser* user, char* payload, int) {
    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_CHANGE_GUILD_NAME);
    if (payload[22]) {
        reply.put_byte(0);
        reply.put_byte(static_cast<unsigned char>(payload[22]));
    } else {
        reply.put_byte(1);
        const int guild_name_length = static_cast<int>(std::strlen(payload + 23));
        reply.put_int(guild_name_length);
        reply.put_str(payload + 23, guild_name_length);
        user->ChangeGuildName(payload + 23);
        GlobalData::s_guild_proxy_mgr->GetServerProxy(
            static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()))
            ->ChangeGuildNameInProxy(ReadU32(payload, 14), payload + 23);
    }
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84EDC94 / 0x84F8BF8 / 0x84F8C28
// IDA_NAME: _ZN21Inter_ChangeGuildNameC2Ev / D2Ev / D0Ev
Inter_ChangeGuildName::Inter_ChangeGuildName() : ISigDispatcher() {}
Inter_ChangeGuildName::~Inter_ChangeGuildName() {}

// IDA_EA: 0x84D51DC
// IDA_NAME: _ZN36Inter_NoticeChangeGuildNameToMembers12dispatch_sigEP5CUserPci
int Inter_NoticeChangeGuildNameToMembers::dispatch_sig(CUser* user,
                                                        char* payload,
                                                        int) {
    const uint32_t charac_no = ReadU32(payload, 14);
    if (user->get_charac_no(-1) == static_cast<int>(charac_no)) {
        PacketGuard notify;
        notify.put_header(0, 218);
        const int guild_name_length = static_cast<int>(std::strlen(payload + 22));
        notify.put_int(guild_name_length);
        notify.put_str(payload + 22, guild_name_length);
        notify.finalize(true);
        user->Send(notify);
        user->ChangeGuildName(payload + 22);
    } else {
        LogManager::logFormat(
            1, "InterDispatcher.cpp",
            "virtual int Inter_NoticeChangeGuildNameToMembers::dispatch_sig(CUser*, char*, int)",
            0x36CA,
            "Inter_NoticeChangeGuildNameToMembers::dispatch_sig() : pUser->get_charac_no(%d) != recv->m_uCharId(%d)",
            user->get_charac_no(-1), charac_no);
    }
    return 0;
}

// IDA_EA: 0x84EDCB0 / 0x84F8BAA / 0x84F8BDA
// IDA_NAME: _ZN36Inter_NoticeChangeGuildNameToMembersC2Ev / D2Ev / D0Ev
Inter_NoticeChangeGuildNameToMembers::Inter_NoticeChangeGuildNameToMembers()
    : ISigDispatcher() {}
Inter_NoticeChangeGuildNameToMembers::~Inter_NoticeChangeGuildNameToMembers() {}

// IDA_EA: 0x84D39AE
// IDA_NAME: _ZN34Inter_NoticeGuildNameOnGuildCreate12dispatch_sigEP5CUserPci
int Inter_NoticeGuildNameOnGuildCreate::dispatch_sig(CUser*, char* payload, int) {
    STGuildProxy guild_info;
    std::memcpy(guild_info.guild_name,
                payload + 22,
                sizeof(guild_info.guild_name) - 1u);
    GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(
            static_cast<unsigned char>(payload[45])))
        ->InsertGuildInfoProxy(ReadU32(payload, 18), guild_info);
    return 0;
}

// IDA_EA: 0x84EDB0C / 0x84F908A / 0x84F90BA
// IDA_NAME: _ZN34Inter_NoticeGuildNameOnGuildCreateC2Ev / D2Ev / D0Ev
Inter_NoticeGuildNameOnGuildCreate::Inter_NoticeGuildNameOnGuildCreate()
    : ISigDispatcher() {}
Inter_NoticeGuildNameOnGuildCreate::~Inter_NoticeGuildNameOnGuildCreate() {}
