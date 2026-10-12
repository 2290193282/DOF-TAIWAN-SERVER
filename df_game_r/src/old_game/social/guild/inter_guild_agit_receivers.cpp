#include "old_data/db/taiwan/inter_slots_357_382.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

CGuildServerProxy* GetGuildProxyFor(CUser* user) {
    return GlobalData::s_guild_proxy_mgr->GetServerProxy(
        static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
}

}  // namespace

// IDA_EA: 0x84DA98E
// IDA_NAME: _ZN27Inter_NoticeCreateGuildAgit12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeCreateGuildAgit::dispatch_sig(CUser *,char *,int)
int Inter_NoticeCreateGuildAgit::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)payload_size;
    const SIG_NOTICE_CREATE_GUILD_AGIT& input =
        *reinterpret_cast<const SIG_NOTICE_CREATE_GUILD_AGIT*>(payload);
    user->SetGuildAgitFlag(true);

    if (static_cast<int>(input.charac_no) == user->get_charac_no(-1)) {
        PacketGuard response;
        response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                            ENUM_CMDPACKET_CREATE_GUILD_AGIT);
        response.put_byte(1);
        response.finalize(true);
        user->Send(response);
    }

    PacketGuard notify;
    notify.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_CREATE_GUILD_AGIT);
    notify.finalize(true);
    user->Send(notify);
    GetGuildProxyFor(user)->ChangeGuildAgitFlagInProxy(
        static_cast<unsigned int>(user->get_charac_guildkey()), 1u);
    return 0;
}

// IDA_EA: 0x84EE180 / 0x84F7F7A / 0x84F7FAA
// IDA_NAME: _ZN27Inter_NoticeCreateGuildAgitC2Ev /
//           _ZN27Inter_NoticeCreateGuildAgitD2Ev /
//           _ZN27Inter_NoticeCreateGuildAgitD0Ev
Inter_NoticeCreateGuildAgit::Inter_NoticeCreateGuildAgit() {}
Inter_NoticeCreateGuildAgit::~Inter_NoticeCreateGuildAgit() {}

// IDA_EA: 0x84DAB18
// IDA_NAME: _ZN27Inter_NoticeDeleteGuildAgit12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeDeleteGuildAgit::dispatch_sig(CUser *,char *,int)
int Inter_NoticeDeleteGuildAgit::dispatch_sig(CUser* user,
                                              char* payload,
                                              int payload_size) {
    (void)payload_size;
    const SIG_NOTICE_DELETE_GUILD_AGIT& input =
        *reinterpret_cast<const SIG_NOTICE_DELETE_GUILD_AGIT*>(payload);
    if (input.error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DELETE_GUILD_AGIT,
                                 input.error);
    } else {
        user->SetGuildAgitFlag(false);
        if (static_cast<int>(input.charac_no) == user->get_charac_no(-1)) {
            PacketGuard response;
            response.put_header(ENUM_PACKETCLASS_CMDPACKET,
                                ENUM_CMDPACKET_DELETE_GUILD_AGIT);
            response.put_byte(1);
            response.finalize(true);
            user->Send(response);
        }

        PacketGuard notify;
        notify.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                          ENUM_NOTIPACKET_DELETE_GUILD_AGIT);
        notify.finalize(true);
        user->Send(notify);
        GetGuildProxyFor(user)->ChangeGuildAgitFlagInProxy(
            static_cast<unsigned int>(user->get_charac_guildkey()), 0u);
    }
    return 0;
}

// IDA_EA: 0x84EE19C / 0x84F7F2C / 0x84F7F5C
// IDA_NAME: _ZN27Inter_NoticeDeleteGuildAgitC2Ev /
//           _ZN27Inter_NoticeDeleteGuildAgitD2Ev /
//           _ZN27Inter_NoticeDeleteGuildAgitD0Ev
Inter_NoticeDeleteGuildAgit::Inter_NoticeDeleteGuildAgit() {}
Inter_NoticeDeleteGuildAgit::~Inter_NoticeDeleteGuildAgit() {}

// IDA_EA: 0x84DACDA
// IDA_NAME: _ZN25Inter_NoticeLoadGuildAgit12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeLoadGuildAgit::dispatch_sig(CUser *,char *,int)
int Inter_NoticeLoadGuildAgit::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;
    const SIG_NOTICE_LOAD_GUILD_AGIT& input =
        *reinterpret_cast<const SIG_NOTICE_LOAD_GUILD_AGIT*>(payload);
    user->SetGuildAgitDBInfo(
        *reinterpret_cast<STGuildAgitDBInfo*>(payload + 22));

    PacketGuard response;
    response.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                        ENUM_NOTIPACKET_GUILD_AGIT_INFO);
    response.put_byte(static_cast<char>(input.guild_agit_flag));
    response.finalize(true);
    user->Send(response);
    return 0;
}

// IDA_EA: 0x84EE1B8 / 0x84F7EDE / 0x84F7F0E
// IDA_NAME: _ZN25Inter_NoticeLoadGuildAgitC2Ev /
//           _ZN25Inter_NoticeLoadGuildAgitD2Ev /
//           _ZN25Inter_NoticeLoadGuildAgitD0Ev
Inter_NoticeLoadGuildAgit::Inter_NoticeLoadGuildAgit() {}
Inter_NoticeLoadGuildAgit::~Inter_NoticeLoadGuildAgit() {}

// IDA_EA: 0x84DAD96
// IDA_NAME: _ZN21Inter_NoticeGuildMemo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_NoticeGuildMemo::dispatch_sig(CUser *,char *,int)
int Inter_NoticeGuildMemo::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    (void)payload_size;
    if (user->get_state()) {
        const SIG_NOTICE_GUILD_MEMO& input =
            *reinterpret_cast<const SIG_NOTICE_GUILD_MEMO*>(payload);
        if (static_cast<int>(input.charac_no) == user->get_charac_no(-1)) {
            PacketGuard response;
            response.put_header(ENUM_PACKETCLASS_NOTIPACKET, 197);
            const int memo_length = static_cast<int>(std::strlen(input.guild_memo));
            response.put_int(memo_length);
            response.put_str(input.guild_memo, memo_length);
            const int notice_length =
                static_cast<int>(std::strlen(input.guild_notice));
            response.put_int(notice_length);
            response.put_str(input.guild_notice, notice_length);
            response.finalize(true);
            user->Send(response);
        }
    }
    return 0;
}

// IDA_EA: 0x84EE12C / 0x84F7E90 / 0x84F7EC0
// IDA_NAME: _ZN21Inter_NoticeGuildMemoC2Ev /
//           _ZN21Inter_NoticeGuildMemoD2Ev /
//           _ZN21Inter_NoticeGuildMemoD0Ev
Inter_NoticeGuildMemo::Inter_NoticeGuildMemo() {}
Inter_NoticeGuildMemo::~Inter_NoticeGuildMemo() {}
