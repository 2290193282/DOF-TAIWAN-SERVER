#include "old_game/gm/core/dispatcher_gm_command.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stdint.h>

#include "old_game/gm/core/cgm_manager_abi.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "common/manager/monitor_server_proxy_abi.h"
#include "old_game/social/party/party.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/stream.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"
#include "infra/concurrent/timer/timer_queue.h"

namespace {

const char kDispatchSignature[] =
    "virtual int Dispatcher_GM_Command::dispatch_sig(CUser*, PacketBuf&)";

#pragma pack(push, 1)
// IDA 0x822BBD6: Packet_Monitor_Notice_Message.  The one-byte message
// length begins immediately after the 10-byte monitor packet header.
struct PacketMonitorNoticeMessage : public PacketHeader {
    PacketMonitorNoticeMessage() : PacketHeader(0x09dcu, 0x010bu) {
        std::memset(message, 0, sizeof(message));
    }

    uint8_t message_length;
    char message[0x100];
};

// IDA 0x822BC70: Packet_Forbid_Chat_By_Monitor.  Its 0x30-byte wire image
// is the monitor header followed by seconds, character-name length and a
// 30-byte character-name body.
struct PacketForbidChatByMonitor : public PacketHeader {
    PacketForbidChatByMonitor() : PacketHeader(0x09ddu, 0x0030u),
                                  seconds(0),
                                  name_length(0),
                                  name() {}

    int seconds;
    int name_length;
    char name[30];
};
#pragma pack(pop)

static_assert(sizeof(PacketMonitorNoticeMessage) == 0x10bu,
              "monitor notice wire packet must remain 0x10B bytes");
static_assert(sizeof(PacketForbidChatByMonitor) == 0x30u,
              "forbid-chat monitor wire packet must remain 0x30 bytes");

bool ReadSizedString(PacketBuf& packet, char* value, int capacity, int& size) {
    size = 0;
    if (packet.get_int(size) != 1 || size < 0 || size >= capacity) {
        return false;
    }
    std::memset(value, 0, static_cast<size_t>(capacity));
    return size == 0 || packet.get_str(value, capacity, size) == 1;
}

bool IsActiveGameMaster(CUser* user) {
    CGM_Manager* const manager = GlobalData::s_GM_Manager;
    return user && manager &&
           manager->GetCurrentGmMode(user->get_acc_id()) == 1 &&
           manager->GetGmLevel(user->get_acc_id()) > 0;
}

void BuildFailure(PacketGuard& packet) {
    packet.put_header(1, ENUM_CMDPACKET_GM_COMMAND);
    packet.put_byte(1);
    packet.put_byte(99);
    packet.put_byte(1);
    packet.finalize(true);
}

void BuildSuccess(PacketGuard& packet, unsigned char command) {
    packet.put_header(1, ENUM_CMDPACKET_GM_COMMAND);
    packet.put_byte(1);
    packet.put_byte(command);
}

void SendNamedSuccess(CUser* sender,
                      CUser* target,
                      unsigned char command,
                      int name_length,
                      const char* name) {
    PacketGuard packet;
    BuildSuccess(packet, command);
    packet.put_int(name_length);
    packet.put_str(name, name_length);
    packet.finalize(true);
    sender->Send(packet);
    target->Send(packet);
}

int ClampForbidSeconds(int minutes) {
    if (minutes <= 0) {
        return 60;
    }
    if (minutes > 1439) {
        return 86400;
    }
    return minutes * 60;
}

CUser* FindNamedUser(const char* name) {
    GameWorld* const world = G_GameWorld();
    if (!world) {
        return 0;
    }
    return world->find_user_from_world_byaccid(world->GetIDUserInIDMap(name));
}

}  // namespace

// IDA_EA: 0x8595698
// IDA_NAME: _ZN21Dispatcher_GM_CommandC2Ev
// DEMANGLED: Dispatcher_GM_Command::Dispatcher_GM_Command(void)
Dispatcher_GM_Command::Dispatcher_GM_Command()
    : IPacketDispatcher<MSG_BASE, ParamBase, static_cast<ch_state>(0)>() {}

// IDA_EA: 0x823379C
// IDA_NAME: _ZN21Dispatcher_GM_CommandD2Ev
// DEMANGLED: Dispatcher_GM_Command::~Dispatcher_GM_Command()
// IDA_EA: 0x82337CC
// IDA_NAME: _ZN21Dispatcher_GM_CommandD0Ev
// DEMANGLED: Dispatcher_GM_Command::~Dispatcher_GM_Command()
Dispatcher_GM_Command::~Dispatcher_GM_Command() {}

// IDA_EA: 0x81FA19E
// IDA_NAME: _ZN21Dispatcher_GM_Command12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: Dispatcher_GM_Command::dispatch_sig(CUser *,PacketBuf &)
int Dispatcher_GM_Command::dispatch_sig(CUser* user, PacketBuf& packet) {
    if (!user) {
        return LineFunc(35069, kDispatchSignature, 0, 0);
    }
    if (!user->getCurCharacR()) {
        return LineFunc(35074, kDispatchSignature, 0, 0);
    }

    unsigned char command = 0;
    if (packet.get_byte(command) != 1) {
        return LineFunc(35086, kDispatchSignature, 0, 0);
    }

    PacketGuard failure;
    BuildFailure(failure);

    CGM_Manager* const manager = GlobalData::s_GM_Manager;
    if (command == 0) {
        if (manager && manager->GetGmLevel(user->get_acc_id()) > 0) {
            user->SetGameMasterMode(
                manager->GetCurrentGmMode(user->get_acc_id()) == 0);
            manager->TurnGmMode(user->get_acc_id());
        }
        return 0;
    }
    if (!IsActiveGameMaster(user)) {
        return 0;
    }

    switch (command) {
    case 1: {
        char message[256];
        int message_length = 0;
        if (!ReadSizedString(packet, message, sizeof(message), message_length)) {
            return 0;
        }
        if (message_length != 0) {
            PacketGuard notice;
            BuildSuccess(notice, command);
            notice.put_int(message_length);
            notice.put_str(message, message_length);
            notice.finalize(true);
            G_GameWorld()->send_all(notice);
        }
        break;
    }
    case 2: {
        char message[256];
        int message_length = 0;
        if (!ReadSizedString(packet, message, sizeof(message), message_length)) {
            return 0;
        }
        if (message_length != 0) {
            PacketGuard notice;
            BuildSuccess(notice, command);
            notice.put_int(message_length);
            notice.put_str(message, message_length);
            notice.finalize(true);
            G_GameWorld()->send_to_area(user->getCurCharacVill(),
                                        user->get_area(false), notice);
        }
        break;
    }
    case 3: {
        char message[256];
        int message_length = 0;
        if (!ReadSizedString(packet, message, sizeof(message), message_length)) {
            return 0;
        }
        if (message_length != 0 && GlobalData::s_monitor_proxy_mgr) {
            PacketMonitorNoticeMessage notice;
            notice.message_length = static_cast<uint8_t>(message_length);
            std::memcpy(notice.message, message,
                        static_cast<size_t>(message_length));
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(user->GetServerGroup()));
            if (proxy) {
                proxy->SendTcpPacket(reinterpret_cast<char*>(&notice),
                                     sizeof(notice));
            }
        }
        break;
    }
    case 4: {
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target) {
            user->Send(failure);
            break;
        }
        SendNamedSuccess(user, target, command, name_length, name);
        target->DisConnSig(static_cast<DISCONN_SIG>(20), true, 0);
        break;
    }
    case 5:
    case 6: {
        int minutes = 0;
        if (packet.get_int(minutes) != 1) {
            return 0;
        }
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target) {
            user->Send(failure);
            break;
        }
        const int seconds = ClampForbidSeconds(minutes);
        if (command == 5) {
            G_GameWorld()->DisableChat(name, seconds);
            G_TimerQueue()->InsertTimer(0, target->GetUID(), 93, seconds,
                                        target->GetLoginTick(),
                                        target->getCurCharacNo());
        } else if (target->get_state() == 3) {
            G_GameWorld()->DisableMove(name, seconds);
            G_TimerQueue()->InsertTimer(0, target->GetUID(), 94, seconds,
                                        target->GetLoginTick(),
                                        target->getCurCharacNo());
        } else {
            break;
        }

        PacketGuard success;
        BuildSuccess(success, command);
        success.put_int(minutes);
        success.put_int(name_length);
        success.put_str(name, name_length);
        success.finalize(true);
        user->Send(success);
        target->Send(success);
        if (command == 5 && GlobalData::s_monitor_proxy_mgr) {
            PacketForbidChatByMonitor monitor_packet;
            monitor_packet.seconds = seconds;
            monitor_packet.name_length = name_length;
            std::memcpy(monitor_packet.name, name,
                        static_cast<size_t>(name_length));
            CMonitorServerProxy* const proxy =
                GlobalData::s_monitor_proxy_mgr->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(target->GetServerGroup()));
            if (proxy) {
                proxy->SendTcpPacket(reinterpret_cast<char*>(&monitor_packet),
                                     sizeof(monitor_packet));
            }
        }
        break;
    }
    case 7:
        user->SetInvisible();
        if (user->get_state() == 3) {
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
        }
        break;
    case 8:
        user->SetVisible();
        if (user->get_state() == 3) {
            user->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                 static_cast<ENUM_NOTIPACKET>(2), 0);
        }
        break;
    case 9: {
        int level = 0;
        if (packet.get_int(level) != 1) {
            return 0;
        }
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target) {
            user->Send(failure);
            break;
        }
        const int delta = level - target->get_charac_level();
        int changed = 0;
        for (int step = 0; step < std::abs(delta); ++step) {
            const int next_level = delta > 0
                                       ? target->get_charac_level() + 1
                                       : target->get_charac_level() - 1;
            changed = target->SetCharacLevel(std::min(200, next_level));
        }
        if (changed) {
            if (target->get_state() == 3) {
                target->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                       static_cast<ENUM_NOTIPACKET>(37), 0);
                target->SendNotiPacket(static_cast<CUser::eSendTarget>(0),
                                       static_cast<ENUM_NOTIPACKET>(2), 0);
                target->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                       static_cast<ENUM_NOTIPACKET>(2), 1);
            } else if (target->get_state() == 5) {
                target->SendNotiPacket(static_cast<CUser::eSendTarget>(1),
                                       static_cast<ENUM_NOTIPACKET>(37), 0);
                target->SendNotiPacket(static_cast<CUser::eSendTarget>(2),
                                       static_cast<ENUM_NOTIPACKET>(2), 1);
            }
            PacketGuard success;
            BuildSuccess(success, command);
            success.put_int(level);
            success.put_int(name_length);
            success.put_str(name, name_length);
            success.finalize(true);
            user->Send(success);
            target->Send(success);
        }
        break;
    }
    case 10: {
        int count = 0;
        unsigned short item_id = 0;
        if (packet.get_int(count) != 1 || packet.get_short(item_id) != 1) {
            return 0;
        }
        // The 140828 client response handler consumes the granted count as
        // u16.  Reject values that cannot be represented before AddItem so a
        // successful mutation can never produce a truncated reply.
        if (count <= 0 || count > 0xffff) {
            user->Send(failure);
            break;
        }
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target) {
            user->Send(failure);
            break;
        }
        ENUM_ITEMSPACE item_space = static_cast<ENUM_ITEMSPACE>(0);
        const int slot = target->AddItem(
            static_cast<int>(item_id), count, static_cast<eItemAddReason>(6),
            item_space, 0);
        if (slot >= 0) {
            target->SendUpdateItemList(static_cast<CUser::eSendTarget>(1),
                                       item_space, slot);
            PacketGuard success;
            BuildSuccess(success, command);
            success.put_short(static_cast<uint16_t>(count));
            success.put_short(item_id);
            success.put_int(name_length);
            success.put_str(name, name_length);
            success.finalize(true);
            user->Send(success);
        }
        break;
    }
    case 11: {
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        CParty* const party = target ? target->GetParty() : 0;
        if (!party) {
            user->Send(failure);
            break;
        }
        party->set_charac_live(target, static_cast<ENUM_USER_DUNGEON_STATE>(1));
        SendNamedSuccess(user, target, command, name_length, name);
        break;
    }
    case 12: {
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target) {
            user->Send(failure);
            break;
        }
        target->RecoverFatigue(0);
        target->SendFatigue();
        SendNamedSuccess(user, target, command, name_length, name);
        break;
    }
    case 13: {
        int days = 0;
        if (packet.get_int(days) != 1) {
            return 0;
        }
        char name[30];
        int name_length = 0;
        if (!ReadSizedString(packet, name, sizeof(name), name_length)) {
            return 0;
        }
        CUser* const target = FindNamedUser(name);
        if (!target || !GlobalData::s_stream_pool || !GlobalData::s_msgq_mgr) {
            user->Send(failure);
            break;
        }
        CStreamGuard stream_guard(
            GlobalData::s_stream_pool->Acquire("PacketDispatcher_Impl_1.cpp",
                                                35624),
            true);
        stream_guard << 147;
        stream_guard << user->GetUID();
        SIG_FORBID_USER_TO_PLAY* const request =
            stream_guard.GetInBuffer<SIG_FORBID_USER_TO_PLAY>();
        std::memset(request, 0, sizeof(*request));
        request->charac_no = static_cast<uint32_t>(target->getCurCharacNo());
        request->forbid = 1;
        request->duration_days = days <= 0 ? 1 : (days > 99 ? 100 : days);
        GlobalData::s_msgq_mgr->put(MsgQueueMgr::DB_Q, stream_guard);

        PacketGuard success;
        BuildSuccess(success, command);
        success.put_int(days);
        success.put_int(name_length);
        success.put_str(name, name_length);
        success.finalize(true);
        user->Send(success);
        target->Send(success);
        break;
    }
    default:
        break;
    }
    return 0;
}

// IDA_EA: 0x8233788
// IDA_NAME: _ZN21Dispatcher_GM_Command4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_GM_Command::read(PacketBuf &,MSG_BASE &)
int Dispatcher_GM_Command::read(PacketBuf& packet, MSG_BASE& message) {
    (void)packet;
    (void)message;
    return 0;
}

// IDA_EA: 0x8233792
// IDA_NAME: _ZN21Dispatcher_GM_Command7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_GM_Command::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_GM_Command::process(CUser* user,
                                   MSG_BASE& message,
                                   ParamBase& param) {
    (void)user;
    (void)message;
    (void)param;
    return 0;
}
