#include "old_data/Login/inter_returns/inter_task4_basic_dispatchers.h"

#include <cstring>
#include <stdint.h>

#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "core/lifecycle/serve/game_server_demon_manager.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* payload) {
    uint32_t value = 0;
    std::memcpy(&value, payload, sizeof(value));
    return value;
}

uint32_t ReadU32(const char* payload, int offset) {
    return ReadU32(payload + offset);
}

int BoundedCStringLength(const char* text, int maximum_length) {
    int length = 0;
    while (length < maximum_length && text[length]) {
        ++length;
    }
    return length;
}

}  // namespace

// IDA_EA: 0x84E4388
// IDA_NAME: _ZN27Inter_GoblinPadSamePwdCheck12dispatch_sigEP5CUserPci
int Inter_GoblinPadSamePwdCheck::dispatch_sig(CUser* user,
                                              char*,
                                              int) {
    if (user) {
        user->SendCmdErrorPacket(
            ENUM_CMDPACKET_MOUSEREGISTER, static_cast<unsigned char>(139));
    }
    return 0;
}

// IDA_EA: 0x84EEB04 / 0x84F645C / 0x84F648C
Inter_GoblinPadSamePwdCheck::Inter_GoblinPadSamePwdCheck()
    : ISigDispatcher() {}
Inter_GoblinPadSamePwdCheck::~Inter_GoblinPadSamePwdCheck() {}

// IDA_EA: 0x84E4F56
// IDA_NAME: _ZN19Inter_LevelUpReward12dispatch_sigEP5CUserPci
int Inter_LevelUpReward::dispatch_sig(CUser* user,
                                      char* payload,
                                      int payload_size) {
    if (!user || user->get_state() == 0 || !payload || payload_size < 16) {
        return 25254;
    }

    // The original leaf only validates the returned sixteen-byte record and
    // completes normally; it does not enqueue a client packet here.
    char reward_id[4];
    char reward_value[12];
    std::memcpy(reward_id, payload, sizeof(reward_id));
    std::memcpy(reward_value, payload + sizeof(reward_id),
                sizeof(reward_value));
    return 0;
}

// IDA_EA: 0x84EEBC8 / 0x84F6102 / 0x84F6132
Inter_LevelUpReward::Inter_LevelUpReward() : ISigDispatcher() {}
Inter_LevelUpReward::~Inter_LevelUpReward() {}

// IDA_EA: 0x84E3232
// IDA_NAME: _ZN25Inter_SendPeriodicMessage12dispatch_sigEP5CUserPci
int Inter_SendPeriodicMessage::dispatch_sig(CUser*,
                                            char* payload,
                                            int payload_size) {
    if (!payload || payload_size <= 10 || !payload[10]) {
        return 0;
    }

    const int text_length = BoundedCStringLength(payload + 10,
                                                 payload_size - 10);
    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_PERIODIC_MESSAGE);
    packet.put_int(text_length);
    packet.put_str(payload + 10, text_length);
    packet.finalize(true);
    G_GameWorld()->send_all_with_state(packet, 3);
    return 0;
}

// IDA_EA: 0x84EE960 / 0x84F68EE / 0x84F691E
Inter_SendPeriodicMessage::Inter_SendPeriodicMessage() : ISigDispatcher() {}
Inter_SendPeriodicMessage::~Inter_SendPeriodicMessage() {}

// IDA_EA: 0x84E3326
// IDA_NAME: _ZN18Inter_SendTimeSync12dispatch_sigEP5CUserPci
int Inter_SendTimeSync::dispatch_sig(CUser*, char* payload, int payload_size) {
    if (!payload || payload_size < 14) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_TIME_SYNC);
    packet.put_short(ReadU16(payload, 10));
    packet.put_short(ReadU16(payload, 12));
    packet.finalize(true);
    G_GameWorld()->send_all_with_state(packet, 3);
    return 0;
}

// IDA_EA: 0x84EE97C / 0x84F68A0 / 0x84F68D0
Inter_SendTimeSync::Inter_SendTimeSync() : ISigDispatcher() {}
Inter_SendTimeSync::~Inter_SendTimeSync() {}

// IDA_EA: 0x84E5E1E
// IDA_NAME: _ZN20Inter_SendSecuReward12dispatch_sigEP5CUserPci
int Inter_SendSecuReward::dispatch_sig(CUser* user,
                                       char* payload,
                                       int payload_size) {
    if (!user || !payload || payload_size < 4) {
        return 0;
    }
    user->AddSecuRewardItem(ReadU32(payload) ? 0x10u : 0x08u);
    return 0;
}

// IDA_EA: 0x84EEC8C / 0x84F5EE0 / 0x84F5F10
Inter_SendSecuReward::Inter_SendSecuReward() : ISigDispatcher() {}
Inter_SendSecuReward::~Inter_SendSecuReward() {}

// IDA_EA: 0x84E7FE2
// IDA_NAME: _ZN21Inter_LoadPuUserCheck12dispatch_sigEP5CUserPci
int Inter_LoadPuUserCheck::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    if (user && payload && payload_size > 0) {
        user->SetPuUser(payload[0] != 0);
    }
    return 0;
}

// IDA_EA: 0x84EED88 / 0x84F59CE / 0x84F59FE
Inter_LoadPuUserCheck::Inter_LoadPuUserCheck() : ISigDispatcher() {}
Inter_LoadPuUserCheck::~Inter_LoadPuUserCheck() {}

// IDA_EA: 0x84E4052
// IDA_NAME: _ZN25Inter_GuildAttendanceInfo12dispatch_sigEP5CUserPci
int Inter_GuildAttendanceInfo::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    if (!user || user->get_state() <= 2 || !payload || payload_size < 47 ||
        static_cast<int>(ReadU32(payload, 10)) != user->getCurCharacNo()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(1, ENUM_CMDPACKET_GUILD_ATTENDANCE_INFO);
    packet.put_byte(1);
    packet.put_byte(static_cast<unsigned char>(payload[18]));
    packet.put_int(static_cast<int>(ReadU32(payload, 19)));
    packet.put_int(static_cast<int>(ReadU32(payload, 23)));
    packet.put_int(static_cast<int>(ReadU32(payload, 43)));
    packet.put_int(static_cast<int>(ReadU32(payload, 35)));
    packet.put_int(static_cast<int>(ReadU32(payload, 39)));
    packet.put_int(static_cast<int>(ReadU32(payload, 27)));
    packet.put_int(static_cast<int>(ReadU32(payload, 31)));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA78 / 0x84F65E2 / 0x84F6612
Inter_GuildAttendanceInfo::Inter_GuildAttendanceInfo() : ISigDispatcher() {}
Inter_GuildAttendanceInfo::~Inter_GuildAttendanceInfo() {}

// IDA_EA: 0x84E41E4
// IDA_NAME: _ZN28Inter_AchieveGuildAttendance12dispatch_sigEP5CUserPci
int Inter_AchieveGuildAttendance::dispatch_sig(CUser* user,
                                               char* payload,
                                               int payload_size) {
    if (!user || user->get_state() <= 2 || !payload || payload_size < 26 ||
        static_cast<int>(ReadU32(payload, 10)) != user->getCurCharacNo()) {
        return 0;
    }

    PacketGuard packet;
    packet.put_header(0, ENUM_NOTIPACKET_ACHIEVE_GUILD_ATTENDANCE);
    packet.put_int(static_cast<int>(ReadU32(payload, 18)));
    packet.put_int(static_cast<int>(ReadU32(payload, 22)));
    packet.finalize(true);
    user->Send(packet);
    return 0;
}

// IDA_EA: 0x84EEA94 / 0x84F6594 / 0x84F65C4
Inter_AchieveGuildAttendance::Inter_AchieveGuildAttendance()
    : ISigDispatcher() {}
Inter_AchieveGuildAttendance::~Inter_AchieveGuildAttendance() {}

// IDA_EA: 0x84E7B84
// IDA_NAME: _ZN27Inter_EmergencyPatchMessage12dispatch_sigEP5CUserPci
int Inter_EmergencyPatchMessage::dispatch_sig(CUser*,
                                              char* payload,
                                              int payload_size) {
    if (!payload || payload_size < 11) {
        return 0;
    }
    GameServerDemonManager* const manager = GameserverDemonManagerInstance();
    if (manager->IsProhibitConnectState()) {
        return 0;
    }

    const unsigned char notice_length = static_cast<unsigned char>(payload[10]);
    if (payload_size < 11 + static_cast<int>(notice_length)) {
        return 0;
    }
    manager->SetUserProhibitFlag(true);
    manager->SetNoticeMessage(notice_length, payload + 11);
    manager->Notice10MinShutDownMessage();
    return 0;
}

// IDA_EA: 0x84EEE30 / 0x84F5AEA / 0x84F5B1A
Inter_EmergencyPatchMessage::Inter_EmergencyPatchMessage()
    : ISigDispatcher() {}
Inter_EmergencyPatchMessage::~Inter_EmergencyPatchMessage() {}

// IDA_EA: 0x84E8064
// IDA_NAME: _ZN26Inter_UpdateServerSnapShot12dispatch_sigEP5CUserPci
int Inter_UpdateServerSnapShot::dispatch_sig(CUser*, char*, int) {
    G_GameWorld()->UpdateServerSnapShot();
    return 0;
}

// IDA_EA: 0x84EEE68 / 0x84F5932 / 0x84F5962
Inter_UpdateServerSnapShot::Inter_UpdateServerSnapShot() : ISigDispatcher() {}
Inter_UpdateServerSnapShot::~Inter_UpdateServerSnapShot() {}

// IDA_EA: 0x84E8906
// IDA_NAME: _ZN25Inter_UpdateMiniCraneSeed12dispatch_sigEP5CUserPci
int Inter_UpdateMiniCraneSeed::dispatch_sig(CUser*,
                                            char* payload,
                                            int payload_size) {
    if (payload && payload_size >= 14) {
        G_GameWorld()->ResetCraneItemNeedMaterial(
            static_cast<int>(ReadU32(payload, 10)));
    }
    return 0;
}

// IDA_EA: 0x84EEEA0 / 0x84F5896 / 0x84F58C6
Inter_UpdateMiniCraneSeed::Inter_UpdateMiniCraneSeed() : ISigDispatcher() {}
Inter_UpdateMiniCraneSeed::~Inter_UpdateMiniCraneSeed() {}

// IDA_EA: 0x84E8F68
// IDA_NAME: _ZN26Inter_SelectAllowProxyUser12dispatch_sigEP5CUserPci
int Inter_SelectAllowProxyUser::dispatch_sig(CUser* user,
                                             char* payload,
                                             int) {
    if (!user || !payload) {
        return 28191;
    }
    user->SendCmdErrorPacket(ENUM_CMDPACKET_SET_UDP_IP_PORT, 7);
    user->DisConnSig(static_cast<DISCONN_SIG>(30), true, 0);
    return 0;
}

// IDA_EA: 0x84EEF2C / 0x84F56DE / 0x84F570E
Inter_SelectAllowProxyUser::Inter_SelectAllowProxyUser() : ISigDispatcher() {}
Inter_SelectAllowProxyUser::~Inter_SelectAllowProxyUser() {}
