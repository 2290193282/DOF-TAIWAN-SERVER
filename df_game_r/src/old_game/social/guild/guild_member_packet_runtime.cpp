#include "old_game/social/guild/guild_member_packet_runtime.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/log/format/trace.h"

void FunctionGuildMember::RefreshGuildBenefits(CUser* user) {
    if (!user || !user->getCurCharacR() || user->get_state() <= 2) return;
    // Same publication path as Inter_MonitorNoticeGuildMemberInfo: NOTI70
    // refreshes the guild window, while full USERINFO carries passive skills.
    user->send_skill_info();
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(1);
    packet.put_short(1);
    user->make_basic_info(packet, 1);
    packet.finalize(true);
    user->Send(packet);
}

bool FunctionGuildMember::SyncCurrentMemberGrade(CUser* user, uint32_t charac_no,
                                                uint32_t guild_key, int count,
                                                const ST_Guild_Mem_Info* members) {
    if (!user || !user->getCurCharacR() || user->get_state() <= 2 ||
        !guild_key || static_cast<uint32_t>(user->get_charac_guildkey()) != guild_key ||
        static_cast<uint32_t>(user->getCurCharacNo()) != charac_no ||
        !members || count < 0 || count > 255) return false;
    const char* const name = user->getCurCharacName();
    const ST_Guild_Mem_Info* self = 0;
    for (int i = 0; i < count; ++i) {
        if (!std::memchr(members[i].character_name, 0, sizeof(members[i].character_name))) return false;
        if (members[i].job == user->get_charac_job() &&
            std::strcmp(members[i].character_name, name) == 0) {
            if (self) return false; // ambiguous roster: do not change authority
            self = &members[i];
        }
    }
    // The member-list byte after job is packed grow type (client 0x569030
    // splits low 4/high 3 bits).  Guild rank is the later +0x3a byte.
    if (!self || user->getGuildMemberGrade() == self->master_grade) return false;
    const int previous = user->getGuildMemberGrade();
    user->setGuildMemberGrade(self->master_grade);
    PacketGuard notice;
    notice.put_header(0, ENUM_NOTIPACKET_GUILD_MEMBER_INFO);
    notice.put_byte(self->master_grade);
    notice.finalize(true);
    user->Send(notice);
    RefreshGuildBenefits(user);
    LogManager::logFormat(1, "GuildMembers.cpp", __FUNCTION__, __LINE__,
        "[GUILD][MEMBER_GRADE_SYNC] char=%u guild=%u grade=%d->%u",
        charac_no, guild_key, previous, static_cast<unsigned int>(self->master_grade));
    return true;
}

// IDA_EA: 0x84CA37A
// IDA_NAME: _ZN19FunctionGuildMember26MakePacketGuildMembersInfoER11PacketGuardiP17ST_Guild_Mem_Info
// DEMANGLED: FunctionGuildMember::MakePacketGuildMembersInfo(PacketGuard &,int,ST_Guild_Mem_Info *)
bool FunctionGuildMember::MakePacketGuildMembersInfo(PacketGuard& packet,
                                                     int count,
                                                     ST_Guild_Mem_Info* members) {
    packet.put_byte(count);
    const int current_second = GlobalData::system_time()->getCurSec();

    // The original is a `while (count-- != 0)` loop.  Its caller supplies an
    // unsigned-byte count, and the returned value is always the final false.
    while (count-- != 0) {
        const int character_name_length =
            static_cast<int>(std::strlen(members->character_name));
        packet.put_int(character_name_length);
        packet.put_str(members->character_name, character_name_length);

        const int member_message_length =
            static_cast<int>(std::strlen(members->member_message));
        packet.put_int(member_message_length);
        packet.put_str(members->member_message, member_message_length);
        packet.put_short(members->level);
        packet.put_byte(members->job);
        packet.put_byte(members->guild_member_grade);
        packet.put_byte(members->channel_no);
        packet.put_byte(members->state);
        packet.put_byte(members->block_flag);
        packet.put_byte(members->master_grade);
        if (members->logout_time &&
            members->logout_time <= static_cast<uint32_t>(current_second)) {
            packet.put_int(current_second - members->logout_time);
        } else {
            packet.put_int(0);
        }
        ++members;
    }
    return false;
}
