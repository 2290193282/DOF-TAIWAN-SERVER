#include "old_game/player/core/cuser.h"

#include "common/manager/global_data.h"
#include "common/manager/statistic_proxy_abi.h"
#include "infra/network/packet/PacketHeader.h"

#include <stdint.h>

#pragma pack(push, 1)
// IDA unit: 99_misc_global/MoneyLogPacket.
struct MoneyLogPacket {
    // IDA_EA: 0x8694602
    // IDA_NAME: _ZN14MoneyLogPacketC2Ev
    // DEMANGLED: MoneyLogPacket::MoneyLogPacket(void)
    MoneyLogPacket();

    PacketHeader header;
    uint32_t charac_no;
    uint32_t plus_money;
    uint32_t minus_money;
    uint32_t account_id;
    uint32_t tick;
};
#pragma pack(pop)

static_assert(sizeof(MoneyLogPacket) == 0x1eu,
              "MoneyLogPacket must remain the old 30-byte UDP payload");

// IDA_EA: 0x8694602
// IDA_NAME: _ZN14MoneyLogPacketC2Ev
// DEMANGLED: MoneyLogPacket::MoneyLogPacket(void)
MoneyLogPacket::MoneyLogPacket()
    : header(0x2723u, 0x1eu),
      charac_no(0),
      plus_money(0),
      minus_money(0),
      account_id(0),
      tick(0) {}

// IDA_EA: 0x868F472
// IDA_NAME: _ZN5CUser12SendMoenyLogEv
// DEMANGLED: CUser::SendMoenyLog(void)
int CUser::SendMoenyLog() {
    if (getCurCharacR()) {
        for (unsigned int index = 0;
             index <= 6u && money_log_index_ >= index;
             ++index) {
            const CharacMoneyLog& log = charac_money_logs_[index];
            if (!log.plus_money && !log.minus_money) {
                continue;
            }

            MoneyLogPacket packet;
            packet.charac_no = static_cast<uint32_t>(getCurCharacNo());
            packet.plus_money = log.plus_money;
            packet.minus_money = log.minus_money;
            packet.account_id = static_cast<uint32_t>(get_acc_id());
            packet.tick = log.tick;

            GlobalData::s_statistic_proxy_mgr
                ->GetServerProxy(
                    static_cast<ENUM_SERVER_GROUP>(GetServerGroup()))
                ->SendPacket(reinterpret_cast<char*>(&packet),
                             static_cast<int>(sizeof(packet)));
        }
    }
    resetMoneyLog();
    return 0;
}
