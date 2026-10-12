#include "old_data/Login/inter_returns/inter_monitor_return_dispatchers.h"
#include "infra/network/packet/PacketEnum.h"

#include <cstring>
#include <ctime>
#include <stdint.h>

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

namespace {

uint32_t ReadU32(const char* payload, int offset) {
    uint32_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

uint16_t ReadU16(const char* payload, int offset) {
    uint16_t value = 0;
    std::memcpy(&value, payload + offset, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84D09AA
// IDA_NAME: _ZN26Inter_Monitor_MegaphoneMsg12dispatch_sigEP5CUserPci
int Inter_Monitor_MegaphoneMsg::dispatch_sig(CUser*, char* payload, int) {
    PacketGuard packet;
    packet.put_header(0, 118);
    packet.put_byte(static_cast<unsigned char>(payload[14]));
    packet.put_byte(static_cast<unsigned char>(payload[11]));
    packet.put_short(ReadU16(payload, 12));
    const int speaker_name_length = static_cast<int>(std::strlen(payload + 15));
    packet.put_int(speaker_name_length);
    packet.put_str(payload + 15, speaker_name_length);
    packet.put_int(static_cast<unsigned char>(payload[45]));
    packet.put_str(payload + 46, static_cast<unsigned char>(payload[45]));
    packet.finalize(true);
    if (G_GameWorld()->IsIntegratedPvPBaseChannel()) {
        G_GameWorld()->send_server_group(packet, payload[10]);
    } else {
        G_GameWorld()->send_all(packet);
    }
    return 0;
}

// IDA_EA: 0x84ED738 / 0x84F99AE / 0x84F99DE
Inter_Monitor_MegaphoneMsg::Inter_Monitor_MegaphoneMsg() : ISigDispatcher() {}
Inter_Monitor_MegaphoneMsg::~Inter_Monitor_MegaphoneMsg() {}

// IDA_EA: 0x84D0BA6
// IDA_NAME: _ZN36Inter_Monitor_RequestBlackListResult12dispatch_sigEP5CUserPci
int Inter_Monitor_RequestBlackListResult::dispatch_sig(CUser* user,
                                                       char* payload,
                                                       int) {
    if (!user->get_state()) return 0;

    PacketGuard reply;
    reply.put_header(1, ENUM_CMDPACKET_REQUEST_BLACKLIST);
    reply.put_byte(1);
    const unsigned char count = static_cast<unsigned char>(payload[14]);
    if (count <= 10u) {
        reply.put_byte(count);
        for (unsigned int index = 0; index < count; ++index) {
            const int offset = 19 + static_cast<int>(40u * index);
            const int name_length = static_cast<int>(std::strlen(payload + offset));
            if (!name_length) {
                LogManager::logFormat(
                    1, "InterDispatcher.cpp",
                    "virtual int Inter_Monitor_RequestBlackListResult::dispatch_sig(CUser*, char*, int)",
                    0x2F6D, "BlackListNameLen Err charno(%d)",
                    user->getCurCharacNo());
            }
            reply.put_int(name_length);
            reply.put_str(payload + offset, name_length);
            const time_t timestamp = static_cast<time_t>(ReadU32(payload, offset + 32));
            std::tm local_time;
            std::tm* const result = localtime_r(&timestamp, &local_time);
            reply.put_byte(static_cast<unsigned char>(result->tm_year));
            reply.put_byte(static_cast<unsigned char>(result->tm_mon));
            reply.put_byte(static_cast<unsigned char>(result->tm_mday));
            user->registerToBlackList(ReadU32(payload, offset - 4));
        }
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x84ED78C / 0x84F9912 / 0x84F9942
Inter_Monitor_RequestBlackListResult::Inter_Monitor_RequestBlackListResult()
    : ISigDispatcher() {}
Inter_Monitor_RequestBlackListResult::~Inter_Monitor_RequestBlackListResult() {}

// IDA_EA: 0x84D0B3A
// IDA_NAME: _ZN28Inter_MonitorNoticeBlackList12dispatch_sigEP5CUserPci
int Inter_MonitorNoticeBlackList::dispatch_sig(CUser* user,
                                               char* payload,
                                               int) {
    if (!user->get_state()) return 0;
    const unsigned char count = static_cast<unsigned char>(payload[18]);
    for (unsigned int index = 0; index < count; ++index) {
        user->registerToBlackList(ReadU32(payload, 19 + static_cast<int>(4u * index)));
    }
    return 0;
}

// IDA_EA: 0x84ED7C4 / 0x84F9960 / 0x84F9990
Inter_MonitorNoticeBlackList::Inter_MonitorNoticeBlackList() : ISigDispatcher() {}
Inter_MonitorNoticeBlackList::~Inter_MonitorNoticeBlackList() {}

// IDA_EA: 0x84D1218
// IDA_NAME: _ZN26Inter_DB_RequestBlackCount12dispatch_sigEP5CUserPci
int Inter_DB_RequestBlackCount::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state()) return 0;
    if (ReadU32(payload, 4) != 0) {
        user->setCurrCharacBlackCount(0);
    } else {
        user->setCurrCharacBlackCount(ReadU32(payload, 8));
    }
    user->EnableCharacInfo(static_cast<CACHE_SECTION>(7));
    if (ReadU32(payload, 12) != 0) {
        PacketGuard reply;
        reply.put_header(0, 236);
        reply.put_int(static_cast<int>(ReadU32(payload, 12)));
        reply.put_int(static_cast<int>(ReadU32(payload, 8)));
        reply.finalize(true);
        user->Send(reply);
    }
    return 0;
}

// IDA_EA: 0x84ED7A8 / 0x84F9828 / 0x84F9858
Inter_DB_RequestBlackCount::Inter_DB_RequestBlackCount() : ISigDispatcher() {}
Inter_DB_RequestBlackCount::~Inter_DB_RequestBlackCount() {}

// IDA_EA: 0x84D1344
// IDA_NAME: _ZN21Inter_WebNoticeSingle12dispatch_sigEP5CUserPci
int Inter_WebNoticeSingle::dispatch_sig(CUser*, char* payload, int) {
    PacketGuard packet;
    packet.put_header(0, 12);
    packet.put_byte(0);
    packet.put_short(0);
    packet.put_byte(0);
    const unsigned char length = static_cast<unsigned char>(payload[10]);
    packet.put_int(length);
    packet.put_str(payload + 11, length);
    packet.finalize(true);
    G_GameWorld()->send_all(packet);
    return 0;
}

// IDA_EA: 0x84ED7E0 / 0x84F97DA / 0x84F980A
Inter_WebNoticeSingle::Inter_WebNoticeSingle() : ISigDispatcher() {}
Inter_WebNoticeSingle::~Inter_WebNoticeSingle() {}

// IDA_EA: 0x84D1448
// IDA_NAME: _ZN22Inter_MemberExpLevelUp12dispatch_sigEP5CUserPci
int Inter_MemberExpLevelUp::dispatch_sig(CUser* user, char* payload, int) {
    if (user->get_state() &&
        user->getCurCharacNo() == static_cast<int>(ReadU32(payload, 14))) {
        user->set_uppermember_exp_level(static_cast<unsigned char>(payload[18]));
    }
    return 0;
}

// IDA_EA: 0x84ED7FC / 0x84F978C / 0x84F97BC
Inter_MemberExpLevelUp::Inter_MemberExpLevelUp() : ISigDispatcher() {}
Inter_MemberExpLevelUp::~Inter_MemberExpLevelUp() {}

// IDA_EA: 0x84D279E
// IDA_NAME: _ZN18Inter_GMRequestMid12dispatch_sigEP5CUserPci
int Inter_GMRequestMid::dispatch_sig(CUser* user, char* payload, int) {
    if (!user->get_state()) return 0;
    PacketGuard reply;
    reply.clear();
    reply.put_header(0, 126);
    reply.put_int(static_cast<int>(ReadU32(payload, 14)));
    reply.put_int(static_cast<int>(ReadU32(payload, 18)));
    const int name_length = static_cast<int>(std::strlen(payload + 22));
    reply.put_int(name_length);
    reply.put_str(payload + 22, name_length);
    reply.finalize(true);
    user->Send(reply);
    return 0;
}

// IDA_EA: 0x84ED8F8 / 0x84F94CE / 0x84F94FE
Inter_GMRequestMid::Inter_GMRequestMid() : ISigDispatcher() {}
Inter_GMRequestMid::~Inter_GMRequestMid() {}
