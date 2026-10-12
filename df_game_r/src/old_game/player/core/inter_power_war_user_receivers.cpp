#include "old_data/db/taiwan/inter_slots_357_382.h"

#include <cstring>

#include "common/manager/global_data.h"
#include "common/manager/global_lifecycle.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/player/core/cuser.h"
#include "old_game/pvp/battlefield/power_war_runtime.h"
#include "infra/network/packet/packet_guard.h"
#include "common/manager/system_time.h"

namespace {

// Both old replies put their first 32-bit player field at a non-word-aligned
// wire offset.  Copying preserves that ABI without introducing an alignment
// assumption into the i386 receiver.
int32_t ReadWireInt32(const char* source) {
    int32_t value = 0;
    std::memcpy(&value, source, sizeof(value));
    return value;
}

uint32_t ReadWireUInt32(const char* source) {
    uint32_t value = 0;
    std::memcpy(&value, source, sizeof(value));
    return value;
}

}  // namespace

// IDA_EA: 0x84DAF30
// IDA_NAME: _ZN25Inter_PowerWarProcessInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PowerWarProcessInfo::dispatch_sig(CUser *,char *,int)
int Inter_PowerWarProcessInfo::dispatch_sig(CUser* user,
                                            char* payload,
                                            int payload_size) {
    (void)payload_size;
    const uint32_t charac_no = ReadWireUInt32(payload + 14);
    if (static_cast<int>(charac_no) == user->get_charac_no(-1)) {
        user->SetPowerWarProcessInfoLastUpdateTime(
            GlobalData::s_systemTime_.getCurSec());

        PacketGuard response;
        response.put_header(0, 201);
        response.put_int(ReadWireInt32(payload + 18));
        response.put_int(ReadWireInt32(payload + 22));
        response.put_int(ReadWireInt32(payload + 26));
        response.put_int(ReadWireInt32(payload + 30));
        response.put_int(ReadWireInt32(payload + 34));
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x84EE148 / 0x84F7DF4 / 0x84F7E24
// IDA_NAME: _ZN25Inter_PowerWarProcessInfoC2Ev /
//           _ZN25Inter_PowerWarProcessInfoD2Ev /
//           _ZN25Inter_PowerWarProcessInfoD0Ev
Inter_PowerWarProcessInfo::Inter_PowerWarProcessInfo() {}
Inter_PowerWarProcessInfo::~Inter_PowerWarProcessInfo() {}

// IDA_EA: 0x84DB070
// IDA_NAME: _ZN21Inter_PowerWarEndInfo12dispatch_sigEP5CUserPci
// DEMANGLED: Inter_PowerWarEndInfo::dispatch_sig(CUser *,char *,int)
int Inter_PowerWarEndInfo::dispatch_sig(CUser* user,
                                        char* payload,
                                        int payload_size) {
    (void)payload_size;
    const uint32_t charac_no = ReadWireUInt32(payload + 14);
    if (static_cast<int>(charac_no) == user->get_charac_no(-1)) {
        PacketGuard response;
        response.put_header(0, 223);
        response.put_byte(payload[18]);
        response.put_int(ReadWireInt32(payload + 19));
        response.put_int(ReadWireInt32(payload + 23));
        response.put_int(ReadWireInt32(payload + 27));
        response.put_int(ReadWireInt32(payload + 31));
        response.put_int(ReadWireInt32(payload + 35));
        if (GlobalData::s_power_manager->GetWinnerSide() ==
            static_cast<unsigned char>(user->getPowerSide())) {
            response.put_int(G_CDataManager()->get_power_war_basic_point());
        } else {
            response.put_int(0);
        }
        response.finalize(true);
        user->Send(response);
    }
    return 0;
}

// IDA_EA: 0x84EE164 / 0x84F7DA6 / 0x84F7DD6
// IDA_NAME: _ZN21Inter_PowerWarEndInfoC2Ev /
//           _ZN21Inter_PowerWarEndInfoD2Ev /
//           _ZN21Inter_PowerWarEndInfoD0Ev
Inter_PowerWarEndInfo::Inter_PowerWarEndInfo() {}
Inter_PowerWarEndInfo::~Inter_PowerWarEndInfo() {}
