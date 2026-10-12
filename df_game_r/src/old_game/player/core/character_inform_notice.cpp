#include "old_game/player/core/character_inform_notice.h"

#include "common/manager/global_data.h"
#include "old_data/db/character/charac_inform_notice_payload.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_guard.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/packet/stream_charac_inform_notice.h"
#include "infra/network/packet/stream_guard.h"
#include "infra/network/packet/stream_pool.h"
#include "infra/concurrent/task_queue/msg_queue_mgr.h"

#include <cstring>

typedef char CCharacInformNotice_Size_Must_Be_0x18[
    (sizeof(CCharacInformNotice) == 0x18) ? 1 : -1];

// IDA_EA: 0x80E4790
// IDA_NAME: _ZN19CCharacInformNoticeC2Ev
// DEMANGLED: CCharacInformNotice::CCharacInformNotice(void)
CCharacInformNotice::CCharacInformNotice() {
    _reset();
}

// IDA_EA: 0x80E47B8
// IDA_NAME: _ZN19CCharacInformNoticeD2Ev
// DEMANGLED: CCharacInformNotice::~CCharacInformNotice()
// IDA_EA: 0x80E47E8
// IDA_NAME: _ZN19CCharacInformNoticeD0Ev
// DEMANGLED: CCharacInformNotice::~CCharacInformNotice()
CCharacInformNotice::~CCharacInformNotice() {}

// IDA_EA: 0x80E4806
// IDA_NAME: _ZN19CCharacInformNotice22handleInformNoticeFlagEj
// DEMANGLED: CCharacInformNotice::handleInformNoticeFlag(unsigned int)
void CCharacInformNotice::handleInformNoticeFlag(uint32_t flag_index) {
    const uint32_t bit = flag_index & 7u;
    if (bit > 7u) {
        return;
    }
    const uint32_t byte_index = flag_index >> 3;
    if (byte_index > 0x0Fu) {
        return;
    }
    if (!checkInformNoticeFlag(bit, byte_index)) {
        return;
    }
    flags_[byte_index] = static_cast<uint8_t>(
        flags_[byte_index] + (1u << bit));
    alter();
}

// IDA_EA: 0x80E4892
// IDA_NAME: _ZN19CCharacInformNotice21checkInformNoticeFlagEjj
// DEMANGLED: CCharacInformNotice::checkInformNoticeFlag(unsigned int,unsigned int)
bool CCharacInformNotice::checkInformNoticeFlag(uint32_t bit,
                                                uint32_t byte_index) {
    if (bit > 7u) {
        return false;
    }
    if (byte_index <= 0x0Fu) {
        return (((flags_[byte_index] >> bit) & 1u) == 0);
    }
    return false;
}

// IDA_EA: 0x80E48EC
// IDA_NAME: _ZN19CCharacInformNotice19getInformNoticeFlagEi
// DEMANGLED: CCharacInformNotice::getInformNoticeFlag(int)
int CCharacInformNotice::getInformNoticeFlag(int byte_index) {
    return flags_[byte_index];
}

// IDA_EA: 0x80E48FC
// IDA_NAME: _ZN19CCharacInformNotice24getInformNoticeFlagIndexERSt6vectorIhSaIhEE
// DEMANGLED: CCharacInformNotice::getInformNoticeFlagIndex(std::vector<unsigned char,std::allocator<unsigned char>> &)
bool CCharacInformNotice::getInformNoticeFlagIndex(
    std::vector<uint8_t>& indexes) {
    indexes.clear();
    for (int byte_index = 0; byte_index <= 15; ++byte_index) {
        const uint8_t flags = static_cast<uint8_t>(
            getInformNoticeFlag(byte_index));
        for (int bit = 0; bit <= 7; ++bit) {
            if (((flags >> bit) & 1u) != 0) {
                indexes.push_back(static_cast<uint8_t>(
                    8 * byte_index + bit));
            }
        }
    }
    return false;
}

// IDA_EA: 0x80E4994
// IDA_NAME: _ZN19CCharacInformNotice20sendInformNoticeFlagEP5CUser
// DEMANGLED: CCharacInformNotice::sendInformNoticeFlag(CUser *)
void CCharacInformNotice::sendInformNoticeFlag(CUser* user) {
    std::vector<uint8_t> indexes;
    getInformNoticeFlagIndex(indexes);

    PacketGuard packet;
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET,
                      ENUM_NOTIPACKET_INFORM_NOTICE_2ND);
    packet.put_byte(static_cast<int>(indexes.size()));
    for (std::vector<uint8_t>::size_type index = 0; index < indexes.size();
         ++index) {
        packet.put_byte(indexes[index]);
    }
    packet.finalize(true);
    user->Send(packet);
}

// IDA_EA: 0x80E4AB8
// IDA_NAME: _ZN19CCharacInformNotice6_resetEv
// DEMANGLED: CCharacInformNotice::_reset(void)
int CCharacInformNotice::_reset() {
    return static_cast<int>(reinterpret_cast<uintptr_t>(
        std::memset(flags_, 0, sizeof(flags_))));
}

// IDA_EA: 0x80E4ADE
// IDA_NAME: _ZN19CCharacInformNotice8loadDataEP5CUserPc
// DEMANGLED: CCharacInformNotice::loadData(CUser *,char *)
bool CCharacInformNotice::loadData(CUser* user, char* src) {
    _reset();
    std::memcpy(flags_, src, sizeof(flags_));
    sendInformNoticeFlag(user);
    return true;
}

// IDA_EA: 0x80E4B2C
// IDA_NAME: _ZNK19CCharacInformNotice7getDataEPc
// DEMANGLED: CCharacInformNotice::getData(char *)const
void CCharacInformNotice::getData(char* dest) const {
    std::memcpy(dest, flags_, sizeof(flags_));
}

// IDA_EA: 0x80E4B52
// IDA_NAME: _ZN19CCharacInformNotice9_saveDataEP5CUser
// DEMANGLED: CCharacInformNotice::_saveData(CUser *)
int CCharacInformNotice::_saveData(CUser* user) {
    Stream* stream =
        GlobalData::stream_pool()->Acquire("CharacInformNotice.cpp", 119);
    CStreamGuard guard(stream, true);
    *guard << 0x311;
    *guard << user->GetUID();

    SIG_SAVE_CHARAC_INFORM_NOTICE* payload =
        guard.GetInBuffer<SIG_SAVE_CHARAC_INFORM_NOTICE>();
    if (!payload) {
        return 0;
    }

    std::memset(payload, 0, sizeof(*payload));
    payload->charac_no =
        static_cast<uint32_t>(user->get_charac_no(-1));
    std::memcpy(payload->flags, flags_, sizeof(flags_));
    GlobalData::msgq_mgr()->put(MsgQueueMgr::DB_Q, guard);
    return 1;
}
