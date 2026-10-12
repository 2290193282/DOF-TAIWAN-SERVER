#include "infra/security/policy/secu_data_control_runtime.h"

#include <algorithm>
#include <cstring>
#include <stdint.h>

#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/security/policy/hack_analyzer_public_abi.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"

class CRijndael;

extern char* encryptAES_(CRijndael* crypt, char* source, char* destination)
    __asm__("_Z11encryptAES_P9CRijndaelPcS1_");

// IDA_EA: 0x82AA6BA
// IDA_NAME: _ZN16Secu_DataControlC2Ev
// DEMANGLED: Secu_DataControl::Secu_DataControl(void)
Secu_DataControl::Secu_DataControl() {}

// IDA_EA: 0x8286A30
// IDA_NAME: _ZN16Secu_DataControl14SetControlDataEP5CUserhtt
// DEMANGLED: Secu_DataControl::SetControlData(
//     CUser *,unsigned char,unsigned short,unsigned short)
char Secu_DataControl::SetControlData(CUser* user,
                                      uint8_t command,
                                      uint16_t first_value,
                                      uint16_t second_value) {
    char result = static_cast<char>(second_value);
    if (!user) {
        return result;
    }

    if (command == 0u) {
        const int hack_type = static_cast<int>(first_value);
        const int count = static_cast<int>(second_value);
        if (hack_type > 100 && hack_type <= 4000 && count <= 9) {
            cMyTrace trace(
                "void Secu_DataControl::SetControlData(CUser*, unsigned char, short unsigned int, short unsigned int)",
                30, 0);
            trace("[SecuCommand] (mid:%s) Hacktype : %d     Cnt : %d ",
                  NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                 0),
                  hack_type, count);
            for (int index = 0; index < count; ++index) {
                result = user->getHackAnalyzer()->addServerHackCnt(
                    user, static_cast<WongWork::ENUM_HACKTYPE>(hack_type),
                    1u, 0u, 0u);
            }
        }
        result = user->getHackAnalyzer()->addServerHackCnt(
            user, static_cast<WongWork::ENUM_HACKTYPE>(0x262), 1u, 0u, 0u);
        return result;
    }

    if (command == 1u) {
        result = static_cast<char>(user->isGMUser() ^ 1);
        if (result == 0) {
            const int bounded_point = std::min(
                std::max(static_cast<int>(first_value), 0), 10000);
            cMyTrace trace(
                "void Secu_DataControl::SetControlData(CUser*, unsigned char, short unsigned int, short unsigned int)",
                50, 0);
            trace("[SecuCommand] (mid:%s) CleanPad Set Point : %d",
                  NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                 0),
                  bounded_point);
            user->getMCAPManagerW().setPoint(
                static_cast<uint16_t>(bounded_point));
            result = static_cast<char>(user->getMCAPManagerW().getPoint() >
                                       1000u);
            if (result != 0) {
                const unsigned int point = user->getMCAPManagerW().getPoint();
                result = static_cast<char>(reinterpret_cast<uintptr_t>(
                    user->setStdDropRate(100 - static_cast<int>(point / 100u))));
            }
        }
        return result;
    }

    if (command == 2u) {
        result = static_cast<char>(user->isGMUser() ^ 1);
        if (result == 0) {
            cMyTrace trace(
                "void Secu_DataControl::SetControlData(CUser*, unsigned char, short unsigned int, short unsigned int)",
                64, 0);
            trace("[SecuCommand] (mid:%s) CleanPad Open!",
                  NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                 0));
            result = static_cast<char>(user->reqHumanCertify4ClearMap(false));
        }
        return result;
    }

    if (command == 3u) {
        result = static_cast<char>(user->isGMUser() ^ 1);
        if (result == 0) {
            const int bounded_rate = std::min(
                std::max(static_cast<int>(first_value), 0), 100);
            cMyTrace trace(
                "void Secu_DataControl::SetControlData(CUser*, unsigned char, short unsigned int, short unsigned int)",
                74, 0);
            trace("[SecuCommand] (mid:%s) DropRate : %d",
                  NumberToString(static_cast<unsigned int>(user->get_acc_id()),
                                 0),
                  bounded_rate);
            result = static_cast<char>(reinterpret_cast<uintptr_t>(
                user->setStdDropRate(bounded_rate)));
        }
    }
    return result;
}

// IDA_EA: 0x8286E40
// IDA_NAME: _ZN16Secu_DataControl26UpdateSecurityServiceStateEP5CUserhhPc
// DEMANGLED: Secu_DataControl::UpdateSecurityServiceState(
//     CUser *,unsigned char,unsigned char,char *)
void Secu_DataControl::UpdateSecurityServiceState(CUser* user,
                                                  uint8_t service_kind,
                                                  uint8_t service_state,
                                                  char* source) {
    if (!user) {
        return;
    }

    bool state_changed = false;
    if (service_kind == 0u) {
        WongWork::CSecurityCard* const card = user->getSecurityCard();
        if (card && card->isActivate()) {
            card->init();
            state_changed = true;
        }
    } else if (service_kind == 1u) {
        Sanicova::CPad* const pad = user->getPad();
        if (pad && pad->isActivate()) {
            if (pad->isCertified()) {
                // The original changes this reply field but deliberately does
                // not send a packet unless card/pad state was reset below.
                service_state = 1u;
            } else {
                char encrypted_password[36];
                char plain_password[36];
                std::memset(encrypted_password, 0,
                            sizeof(encrypted_password));
                std::memset(plain_password, 0, sizeof(plain_password));
                std::strncpy(plain_password, source, 8u);
                cMyTrace trace(
                    "void Secu_DataControl::UpdateSecurityServiceState(CUser*, unsigned char, unsigned char, char*)",
                    160, 0);
                trace("[Secu_Web_Conn] (%s)  ", source);
                encryptAES_(GlobalData::s_pcryptRijndael_MousePassword_,
                            plain_password, encrypted_password);
                pad->setFailCnt(0);
                pad->setPassword(encrypted_password);
                state_changed = true;
            }
        }
    }

    if (state_changed) {
        PacketGuard packet;
        packet.put_header(0, 345);
        packet.put_byte(static_cast<int>(service_kind));
        packet.put_byte(static_cast<int>(service_state));
        packet.finalize(true);
        user->Send(packet);
    }
}
