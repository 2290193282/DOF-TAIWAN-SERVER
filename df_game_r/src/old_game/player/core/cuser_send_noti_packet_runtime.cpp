#include "old_game/player/core/cuser.h"

#include <cstddef>
#include <cstring>

#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "infra/log/format/trace.h"
#include "old_game/progression/quest/quest_runtime.h"
#include "old_game/dungeon/champion_break/champion_break_runtime.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

static_assert(offsetof(PvpResultType, values) +
                      22u * sizeof(PvpResultType::values[0]) ==
                  0x58u,
              "SendNotiPacket PVP exp field offset must stay at +0x58");

}  // namespace

// IDA_EA: 0x867BA5C
// IDA_NAME: _ZN5CUser14SendNotiPacketENS_11eSendTargetE15ENUM_NOTIPACKETi
// DEMANGLED: CUser::SendNotiPacket(
//             CUser::eSendTarget,ENUM_NOTIPACKET,int)
int CUser::SendNotiPacket(eSendTarget target,
                          ENUM_NOTIPACKET packet_type,
                          int value) {
    PacketGuard packet;
    packet.put_header(0, packet_type);

    if (packet_type == ENUM_NOTIPACKET_USERINFO) {
        packet.put_byte(value);
        packet.put_short(1);
        make_basic_info(packet, static_cast<char>(value));
    } else if (packet_type == ENUM_NOTIPACKET_EXP) {
        packet.put_byte(get_charac_level());
        packet.put_int(get_charac_exp());
        packet.put_int(get_charac_party_bonus_exp());
        packet.put_int(get_charac_member_bonus_exp());

        const SkillSlot* const skill_slot = getCurCharacSkillR();
        packet.put_short(
            skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_0));
        packet.put_short(
            skill_slot->get_remain_sp_at_index(ENUM_SKILL_TREE_1));
        packet.put_short(
            skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_0));
        packet.put_short(
            skill_slot->get_remain_sfp_at_index(ENUM_SKILL_TREE_SFP_1));

        packet.put_int(getPVPResultR()->values[22]);
        packet.put_int(get_charac_fatigue_buf_bonus_exp());
        packet.put_int(get_charac_seria_buf_bonus_exp());

        CEventBase* const weekend_event =
            GlobalData::s_event_manager->GetRepeatEvent(87);
        if (weekend_event->IsEventing(0)) {
            packet.put_int(getWeekendBonusExp());
            resetWeekendBonusExp();
        } else {
            packet.put_int(0);
        }

        packet.put_int(getPremiumBonusExp());
        resetPremiumBonusExp();

        // CLIENT_IDA_EVIDENCE: HandleNotiExperienceUpdate reads two more
        // uint32 bonus categories after the premium bonus field.
        packet.put_int(0);
        packet.put_int(champion_break::ConsumeHunterExpBonus(this));
    } else {
        LogManager::logFormat(
            1,
            "user.cpp",
            "bool CUser::SendNotiPacket(CUser::eSendTarget, "
            "ENUM_NOTIPACKET, int)",
            0x76D1,
            "User %s - NotifyID = %d",
            getCurCharacName(),
            static_cast<int>(packet_type));
        return 0;
    }

    packet.finalize(true);
    SendPacket(target, packet);
    return 1;
}

// IDA_EA: 0x86886CE
// IDA_NAME: _ZN5CUser21SendNotiPacketMessageEPKc17ENUM_MESSAGE_TYPE
// DEMANGLED: CUser::SendNotiPacketMessage(char const*,ENUM_MESSAGE_TYPE)
void CUser::SendNotiPacketMessage(const char* text, ENUM_MESSAGE_TYPE type) {
    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(static_cast<int>(type));
    packet.put_short(0);
    packet.put_byte(0);
    packet.put_int(static_cast<int>(std::strlen(text)));
    packet.put_str(text, static_cast<int>(std::strlen(text)));
    packet.finalize(true);
    Send(packet);
}
