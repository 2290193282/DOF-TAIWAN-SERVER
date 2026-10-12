#include "old_game/social/guild/guild_join_inter_receivers.h"

#include <cstring>

#include "old_data/db/legacy_dispatch/db_final_refill_exact27.h"
#include "old_game/social/guild/guild_server_proxy_abi.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"


// IDA_EA: 0x84E3790
// IDA_NAME: _ZN20Inter_CheckJoinGuild12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_CheckJoinGuild::dispatch_sig(CUser *,char *,int)
int Inter_CheckJoinGuild::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_CHECK_JOIN_GUILD_DB_REPLY* const reply =
        reinterpret_cast<const SIG_CHECK_JOIN_GUILD_DB_REPLY*>(payload);
    if (reply->result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_CHECK_JOIN_GUILD,
            static_cast<unsigned char>(reply->result));
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_CHECK_JOIN_GUILD);
    packet.put_byte(1);

    int text_size = static_cast<int>(std::strlen(reply->guild_name));
    packet.put_int(text_size);
    packet.put_str(reply->guild_name, text_size);

    text_size = static_cast<int>(std::strlen(reply->master_name));
    packet.put_int(text_size);
    packet.put_str(reply->master_name, text_size);
    packet.put_int(reply->guild_level);
    packet.put_int(reply->member_count);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EE9EC
// IDA_NAME: _ZN20Inter_CheckJoinGuildC2Ev
// DEMANGLED: Inter_CheckJoinGuild::Inter_CheckJoinGuild(void)
Inter_CheckJoinGuild::Inter_CheckJoinGuild() {
}

// IDA_EA: 0x84F6768
// IDA_NAME: _ZN20Inter_CheckJoinGuildD2Ev
// DEMANGLED: Inter_CheckJoinGuild::~Inter_CheckJoinGuild()
// IDA_EA: 0x84F6798
// IDA_NAME: _ZN20Inter_CheckJoinGuildD0Ev
// DEMANGLED: Inter_CheckJoinGuild::~Inter_CheckJoinGuild()
Inter_CheckJoinGuild::~Inter_CheckJoinGuild() {
}

// IDA_EA: 0x84E3ACE
// IDA_NAME: _ZN19Inter_JoinGuildInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_JoinGuildInfo::dispatch_sig(CUser *,char *,int)
int Inter_JoinGuildInfo::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_JOIN_GUILD_INFO* const reply =
        reinterpret_cast<const SIG_JOIN_GUILD_INFO*>(payload);
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_JOIN_GUILD_INFO);
    packet.put_byte(reply->found);
    if (reply->found != 0) {
        packet.put_int(static_cast<int>(reply->guild_id));

        int text_size = static_cast<int>(std::strlen(reply->guild_name));
        packet.put_int(text_size);
        packet.put_str(reply->guild_name, text_size);

        text_size = static_cast<int>(std::strlen(reply->master_name));
        packet.put_int(text_size);
        packet.put_str(reply->master_name, text_size);
        packet.put_int(reply->guild_level);
        packet.put_int(reply->member_count);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA24
// IDA_NAME: _ZN19Inter_JoinGuildInfoC2Ev
// DEMANGLED: Inter_JoinGuildInfo::Inter_JoinGuildInfo(void)
Inter_JoinGuildInfo::Inter_JoinGuildInfo() {
}

// IDA_EA: 0x84F66CC
// IDA_NAME: _ZN19Inter_JoinGuildInfoD2Ev
// DEMANGLED: Inter_JoinGuildInfo::~Inter_JoinGuildInfo()
// IDA_EA: 0x84F66FC
// IDA_NAME: _ZN19Inter_JoinGuildInfoD0Ev
// DEMANGLED: Inter_JoinGuildInfo::~Inter_JoinGuildInfo()
Inter_JoinGuildInfo::~Inter_JoinGuildInfo() {
}

// IDA_EA: 0x84E3C60
// IDA_NAME: _ZN19Inter_GuildJoinList12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_GuildJoinList::dispatch_sig(CUser *,char *,int)
int Inter_GuildJoinList::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    (void)payload_size;

    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_GUILD_JOIN_LIST* const reply =
        reinterpret_cast<const SIG_GUILD_JOIN_LIST*>(payload);
    const int count = static_cast<int>(reply->count);

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GUILD_JOIN_LIST);
    packet.put_byte(1);
    packet.put_int(count);
    for (int index = 0; index < count; ++index) {
        const SIG_GUILD_JOIN_LIST_ENTRY& entry = reply->entries[index];
        packet.put_int(static_cast<int>(entry.charac_no));

        const int text_size =
            static_cast<int>(std::strlen(entry.charac_name));
        packet.put_int(text_size);
        packet.put_str(entry.charac_name, text_size);

        // The old word-byte extraction emits level before sex:
        // SBYTE2(word + 9), SHIBYTE(word + 9), SBYTE1(word + 10),
        // SLOBYTE(word + 10).
        packet.put_byte(static_cast<int>(entry.job));
        packet.put_byte(static_cast<int>(entry.grow_type));
        packet.put_byte(static_cast<int>(entry.level));
        packet.put_byte(static_cast<int>(entry.sex));

        const int memo_size = static_cast<int>(std::strlen(entry.memo));
        packet.put_int(memo_size);
        packet.put_str(entry.memo, memo_size);
        packet.put_int(entry.elapsed_seconds);
    }
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA5C
// IDA_NAME: _ZN19Inter_GuildJoinListC2Ev
// DEMANGLED: Inter_GuildJoinList::Inter_GuildJoinList(void)
Inter_GuildJoinList::Inter_GuildJoinList() {
}

// IDA_EA: 0x84F667E
// IDA_NAME: _ZN19Inter_GuildJoinListD2Ev
// DEMANGLED: Inter_GuildJoinList::~Inter_GuildJoinList()
// IDA_EA: 0x84F66AE
// IDA_NAME: _ZN19Inter_GuildJoinListD0Ev
// DEMANGLED: Inter_GuildJoinList::~Inter_GuildJoinList()
Inter_GuildJoinList::~Inter_GuildJoinList() {
}

// IDA_EA: 0x84E3924
// IDA_NAME: _ZN22Inter_RequestJoinGuild12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_RequestJoinGuild::dispatch_sig(CUser *,char *,int)
int Inter_RequestJoinGuild::dispatch_sig(CUser* user,
                                         char* payload,
                                         int payload_size) {
    (void)payload_size;
    if (user->get_state() <= 2) {
        return 0;
    }

    const SIG_REQUEST_JOIN_GUILD_DB_REPLY* const reply =
        reinterpret_cast<const SIG_REQUEST_JOIN_GUILD_DB_REPLY*>(payload);
    if (reply->result != 0) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_REQUEST_JOIN_GUILD,
            static_cast<unsigned char>(reply->result));
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_REQUEST_JOIN_GUILD);
    packet.put_byte(1);
    packet.put_int(static_cast<int>(reply->guild_id));

    int text_size = static_cast<int>(std::strlen(reply->guild_name));
    packet.put_int(text_size);
    packet.put_str(reply->guild_name, text_size);

    text_size = static_cast<int>(std::strlen(reply->master_name));
    packet.put_int(text_size);
    packet.put_str(reply->master_name, text_size);
    packet.put_int(reply->guild_level);
    packet.put_int(reply->member_count);
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA08
// IDA_NAME: _ZN22Inter_RequestJoinGuildC2Ev
// DEMANGLED: Inter_RequestJoinGuild::Inter_RequestJoinGuild(void)
Inter_RequestJoinGuild::Inter_RequestJoinGuild() {}

// IDA_EA: 0x84F671A
// IDA_NAME: _ZN22Inter_RequestJoinGuildD2Ev
// DEMANGLED: Inter_RequestJoinGuild::~Inter_RequestJoinGuild()
// IDA_EA: 0x84F674A
// IDA_NAME: _ZN22Inter_RequestJoinGuildD0Ev
// DEMANGLED: Inter_RequestJoinGuild::~Inter_RequestJoinGuild()
Inter_RequestJoinGuild::~Inter_RequestJoinGuild() {}

// IDA_EA: 0x84E34BA
// IDA_NAME: _ZN24Inter_AddGuildFundResult12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_AddGuildFundResult::dispatch_sig(CUser *,char *,int)
int Inter_AddGuildFundResult::dispatch_sig(CUser* user,
                                           char* payload,
                                           int payload_size) {
    if (user->get_state() <= 2 ||
        payload_size < static_cast<int>(sizeof(Packet_Add_Guild_Fund_Result))) {
        return 0;
    }

    const Packet_Add_Guild_Fund_Result& result =
        *reinterpret_cast<const Packet_Add_Guild_Fund_Result*>(payload);
    if (result.charac_no != static_cast<unsigned int>(user->getCurCharacNo())) {
        return 0;
    }
    if (result.result_code != 0) {
        user->getCurCharacInvenW()->gain_money(
            result.refund_amount,
            static_cast<eMoneyAddReason>(29),
            true,
            0);
        user->SendCmdErrorPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND,
                                 result.result_code);
    } else {
        user->SendCmdOkPacket(ENUM_CMDPACKET_DONATE_GUILD_FUND);
    }
    return 0;
}

// IDA_EA: 0x84EE9B4
// IDA_NAME: _ZN24Inter_AddGuildFundResultC2Ev
// DEMANGLED: Inter_AddGuildFundResult::Inter_AddGuildFundResult(void)
Inter_AddGuildFundResult::Inter_AddGuildFundResult() {}

// IDA_EA: 0x84F6804
// IDA_NAME: _ZN24Inter_AddGuildFundResultD2Ev
// DEMANGLED: Inter_AddGuildFundResult::~Inter_AddGuildFundResult()
// IDA_EA: 0x84F6834
// IDA_NAME: _ZN24Inter_AddGuildFundResultD0Ev
// DEMANGLED: Inter_AddGuildFundResult::~Inter_AddGuildFundResult()
Inter_AddGuildFundResult::~Inter_AddGuildFundResult() {}
