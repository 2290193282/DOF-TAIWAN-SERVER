#ifndef OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_RUNTIME_H_
#define OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_RUNTIME_H_

#include "old_data/PVF/arad/arad_script_interface.h"
#include "old_data/PVF/arad/arad_singleton.h"
#include "old_game/player/core/cerashop_add_restrict_user_info.h"
#include "infra/network/packet/stream.h"

#include <map>
#include <stdint.h>
#include <vector>

class CUser;

namespace CerashopAddRestrict {

// IDA type: CerashopAddRestrict::RestrictType.  The nested enum name is part
// of the original InfoDaily constructor ABI.
struct RestrictType {
    enum T {
        ACCOUNT = 0,
        CHARACTER = 1,
    };
};

// IDA unit: CerashopAddRestrict::paramDaily, allocation size 0x0c in
// InputBuyInfoDetail @0x80E1196 and LoadProcess @0x80E142A.
struct paramDaily {
    uint32_t ipg_no;     // +0x00
    uint32_t count;      // +0x04
    uint32_t next_date;  // +0x08
};

// IDA unit: CerashopAddRestrict::InfoDaily, allocation size 0x1c in
// ScriptLoader::LoadScriptDaily @0x8ABF4DE.
struct InfoDaily {
    // IDA_EA: 0x8ABF826
    // IDA_NAME: _ZN19CerashopAddRestrict9InfoDailyC2ENS_12RestrictType1TEjjllii
    // DEMANGLED: CerashopAddRestrict::InfoDaily::InfoDaily(
    //   CerashopAddRestrict::RestrictType::T,unsigned int,unsigned int,
    //   long,long,int,int)
    InfoDaily(RestrictType::T type_value,
              uint32_t ipg_no_value,
              uint32_t limit_value,
              long begin_time_value,
              long end_time_value,
              int interval_value,
              int count_value);

    RestrictType::T type;  // +0x00
    uint32_t ipg_no;       // +0x04
    uint32_t limit;        // +0x08
    long begin_time;       // +0x0c
    long end_time;         // +0x10
    int interval;          // +0x14
    int count;             // +0x18
};

// IDA unit: CerashopAddRestrict::DBInputData, C2 @0x80E243C.
struct DBInputData {
    DBInputData();

    paramDaily* parameter;  // +0x00
    InfoDaily* restrict_info;  // +0x04
};

typedef std::map<unsigned int, paramDaily*> ParamDailyMap;
typedef std::map<unsigned int, unsigned int> RestrictValueMap;

// IDA unit: CerashopAddRestrict::ScriptLoader.  Its D2 only destroys the
// map container; ClearScript @0x8ABF704 is an intentional no-op in the
// original binary, so InfoDaily ownership must not be invented here.
class ScriptLoader {
public:
    // IDA_EA: 0x80E2414 / 0x80E2428
    ScriptLoader();
    ~ScriptLoader();

    // IDA_EA: 0x8ABF3CC
    int LoadScript();

    // IDA_EA: 0x8ABF704
    void ClearScript();

    // IDA_EA: 0x8ABF70A / 0x8ABF766
    bool FindIpgNo(unsigned int ipg_no);
    InfoDaily* GetRestrictInfo(unsigned int ipg_no);

private:
    // IDA_EA: 0x8ABF4DE
    int LoadScriptDaily(RestrictType::T type);

    std::map<unsigned int, InfoDaily*> restrict_infos_;  // +0x00
};

// IDA unit: CerashopAddRestrict::Manager.  ScriptInterface is the physical
// vptr-bearing base at +0x00; the empty Singleton base follows it exactly as
// the original C2 construction order at 0x80E09BC.
class Manager : public ARAD::ScriptInterface,
                public ARAD::Singleton<Manager> {
public:
    // IDA_EA: 0x80E09BC / 0x80E0A24
    Manager();
    ~Manager();

    // IDA_EA: 0x80E0E08 / 0x80E0E40
    virtual bool Load();
    virtual int Destroy();

    // IDA_EA: 0x80E0CE2 / 0x80E0D14 / 0x80E0D90 / 0x80E0DA4 / 0x80E0DEC
    static int IsBuyableRestrictItem(CUser* user,
                                     unsigned int ipg_no,
                                     unsigned int count,
                                     int index);
    static int UpdateBuyableRestrictItem(CUser* user,
                                         unsigned int ipg_no,
                                         unsigned int count);
    static int LoadRequestBuyRestrictItem(CUser* user);
    static void ClearBuyRestrictItem(CUser* user);
    static void SendSyncPacket(CUser* user);

    // IDA_EA: 0x80E0BEE / 0x80E0E4A / 0x80E0E68 / 0x80E0F3A / 0x80E129A
    bool CheckBuyableProduct(CUser* user,
                             unsigned int ipg_no,
                             unsigned int count,
                             int index);
    bool FindIpgNo(unsigned int ipg_no);
    void GetItemListForPacket(std::vector<unsigned int>& items,
                              ParamDailyMap& parameters);
    void makeSyncPacket(CUser* user);
    bool InputBuyInfo(CUser* user,
                      unsigned int ipg_no,
                      unsigned int count,
                      DBInputData& input);

    // IDA_EA: 0x80E142A
    static bool LoadProcess(RestrictValueMap& values,
                            ParamDailyMap& parameters,
                            SIG_LOAD_CERASHOP_ADD_RESTRICT_TYPE* load);

private:
    ScriptLoader script_loader_;  // +0x04
};

// IDA_EA: 0x80E1196 / 0x8ABF164 / 0x8ABF33F.
paramDaily* InputBuyInfoDetail(ParamDailyMap& parameters,
                               const InfoDaily* restrict_info,
                               unsigned int ipg_no,
                               unsigned int count);
int readTime(short values[4]);
long uint32Time(short values[4]);

}  // namespace CerashopAddRestrict

namespace ARAD {
template <>
Singleton<CerashopAddRestrict::Manager>::Singleton();
template <>
CerashopAddRestrict::Manager*
Singleton<CerashopAddRestrict::Manager>::Get();
}  // namespace ARAD

#if defined(__i386__)
static_assert(sizeof(CerashopAddRestrict::paramDaily) == 0x0cu,
              "CerashopAddRestrict::paramDaily must remain 0x0c");
static_assert(sizeof(CerashopAddRestrict::InfoDaily) == 0x1cu,
              "CerashopAddRestrict::InfoDaily must remain 0x1c");
static_assert(sizeof(CerashopAddRestrict::DBInputData) == 0x08u,
              "CerashopAddRestrict::DBInputData must remain 0x08");
static_assert(sizeof(CerashopAddRestrict::ScriptLoader) == 0x18u,
              "CerashopAddRestrict::ScriptLoader must remain 0x18");
static_assert(sizeof(CerashopAddRestrict::Manager) == 0x1cu,
              "CerashopAddRestrict::Manager must remain 0x1c");
#endif

#endif  // OLDSRV_OLD_GAME_NPC_CERASHOP_ADD_RESTRICT_RUNTIME_H_
