#include "common/manager/user.h"

#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/dispatcher/packet_register_gm_mid.h"
#include "infra/network/transport/network_endpoint.h"
#include "infra/security/policy/pi_sender_manager.h"
#include "infra/security/dynamic_abi/antibot_adapter.h"
#include "infra/security/policy/packet_integrity.h"
#include "old_game/world/region/game_world.h"

#include <stdint.h>
#include <string.h>


const char* GetPacketName(int packet_class, unsigned short packet_id);

int OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO(
    CUser* user, PacketBuf* packet)
    __asm__("OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO");

// CUser/CUserCharacInfo gameplay identity accessors moved to
// old_game/userinfo/cuser_identity_command_runtime.cpp.

// IDA_EA: 0x86945D4
// IDA_NAME: _ZN22Packet_Register_GM_MIDC2Ev
// DEMANGLED: Packet_Register_GM_MID::Packet_Register_GM_MID(void)
Packet_Register_GM_MID::Packet_Register_GM_MID()
    : type(0x1F44u),
      size(0x000Eu),
      checksum(0),
      reserved(0),
      charac_no(0) {
}

// CUser/CUserCharacInfo gameplay name/level and command-result transitions
// moved to old_game/userinfo/cuser_identity_command_runtime.cpp.

// IDA_EA: 0x864809C
// IDA_NAME: OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO
// DEMANGLED: CUser::AntibotSend(PacketBuf&)
int OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO(
    CUser* user, PacketBuf* packet) {
    const SC_Header* const sc_header =
        reinterpret_cast<const SC_Header*>(packet->get_packet(0));
    const unsigned short packet_id = sc_header->type;
    bool dproto_wrap_required = false;
    bool dproto_wrap_failed = false;

    IAntibotInterface* checker = GlobalData::antibot_checker();
    if (packet->isEncryptRequired() && checker != 0) {
        if (AntibotCheckerIsCmdInDownList(
                checker,
                static_cast<uint32_t>(user->getAntibotKey()),
                static_cast<int>(packet_id)) != 0) {
            dproto_wrap_required = true;
        }
    }

    if (dproto_wrap_required) {
        PacketGuard dproto_packet;
        const int packet_len = packet->get_len();
        const int dproto_capacity = packet_len + 500;
        uint32_t dproto_len = static_cast<uint32_t>(dproto_capacity);
        char* dproto_payload =
            static_cast<char*>(__builtin_alloca(dproto_capacity));
        memset(dproto_payload, 0, static_cast<size_t>(dproto_capacity));

        dproto_packet.clear();
        dproto_packet.put_header(0, ENUM_NOTIPACKET_DPROTO);
        const int wrap_result =
            AntibotCheckerRecvBufferForEncrypt(
                checker,
                static_cast<uint32_t>(user->getAntibotKey()),
                reinterpret_cast<uint8_t*>(packet->get_packet(0)),
                static_cast<uint32_t>(packet_len),
                reinterpret_cast<uint8_t*>(dproto_payload),
                &dproto_len);
        if (wrap_result != 0) {
            cMyTrace trace("bool CUser::AntibotSend(PacketBuf&)", 2055, 0);
            trace("[RecvBufferForEncrypt] result : %d, PacketID : %d, Key : %d, Length : %d",
                  wrap_result,
                  packet_id,
                  user->getAntibotKey(),
                  packet_len);
            dproto_wrap_failed = true;
        } else {
            GameWorld* const world = G_GameWorld();
            if (world && world->GetChannelType() == 19 &&
                (user->GetPvpIndex() >= 0 || user->get_state() == 5)) {
                cMyTrace trace(
                    "OldSrv_CUser_AntibotSend_wrap_encrypt_required_as_NOTI_DPROTO",
                    __LINE__,
                    5);
                trace("[ZOMBIE_DPROTO][INNER] class=%u opcode=%u plain_len=%d wrapped_len=%u state=%d pvp_index=%d",
                      static_cast<unsigned int>(
                          static_cast<unsigned char>(packet->get_packet(0)[0])),
                      static_cast<unsigned int>(packet_id),
                      packet_len,
                      static_cast<unsigned int>(dproto_len),
                      user->get_state(),
                      user->GetPvpIndex());
            }
            dproto_packet.put_binary(dproto_payload,
                                     static_cast<int>(dproto_len));
            dproto_packet.finalize(false);
            return user->Send(dproto_packet);
        }
    }

    PacketBuf final_packet;
    char* packet_bytes = packet->get_packet(0);
    int packet_len = packet->get_len();
    if ((!dproto_wrap_required && packet->isEncryptRequired()) ||
        dproto_wrap_failed) {
        final_packet.copy(*packet);
        final_packet.encrypt_packet();
        packet_bytes = final_packet.get_packet(0);
        packet_len = final_packet.get_len();
        final_packet.encFinalize();
    }

    _NS_PI_2ND_SendHash_SC(packet_bytes,
                           static_cast<uint16_t>(packet_len),
                           user);
    const bool is_initial_hello =
        static_cast<unsigned char>(packet_bytes[0]) == 0u && packet_id == 1u;
    if (is_initial_hello) {
        // Metadata only: no cipher key, PI value, address, or packet bytes.
        LogManager::logFormat(
            1, "hello_wire_diag.cpp",
            "int CUser::AntibotSend(PacketBuf&)", 0,
            "[DIAG0627][HELLO_WIRE] fd=%d wire_len=%d encrypted=%d dproto=%d",
            user->network_.get_socket(), packet_len,
            packet->isEncryptRequired() ? 1 : 0,
            dproto_wrap_required ? 1 : 0);
    }
    const int sent = user->network_.send_packet(packet_bytes, packet_len);
    if (is_initial_hello) {
        LogManager::logFormat(
            1, "hello_wire_diag.cpp",
            "int CUser::AntibotSend(PacketBuf&)", 0,
            "[DIAG0627][HELLO_WRITE] fd=%d requested=%d result=%d queued=%d",
            user->network_.get_socket(), packet_len, sent,
            user->network_.GetRemainSendSize());
    }
    if (sent >= 0) {
        return 1;
    }

    cMyTrace trace("bool CUser::AntibotSend(PacketBuf&)", 2100, 0);
    trace("User %s - send_packet() fail length(ID:%d,LEN:%d),",
          user->getCurCharacName(),
          packet_id,
          packet_len);
    if (packet_id != ENUM_NOTIPACKET_DISCONNECT_REASON) {
        user->DisConnSig(static_cast<DISCONN_SIG>(6), false, 0);
    }
    return 0;
}
