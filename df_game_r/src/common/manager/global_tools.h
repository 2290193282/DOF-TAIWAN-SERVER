#ifndef OLDSRV_GLOBAL_GLOBAL_TOOLS_H_
#define OLDSRV_GLOBAL_GLOBAL_TOOLS_H_

#include <bitset>
#include <ctime>
#include <stdint.h>
#include <string>
#include <vector>

#include "common/config.h"
#include "old_data/PVF/pvp_mission/datamanager/pvp_mission_system.h"
#include "infra/network/packet/PacketEnum.h"

struct LotteryInfo;
class MySQL;
class CDungeonDataManager;
class PacketGuard;
class CUser;
class CItem;
class PvP_Room;
struct MSG_MAKE_PVP_ROOM;
struct _Additioal_info;

namespace Global {
class CGlobalFunc {
public:
    // IDA_EA: 0x81466F0
    // IDA_NAME: _ZN6Global11CGlobalFunc15Get_State_CheckE8ch_state
    // DEMANGLED: Global::CGlobalFunc::Get_State_Check(ch_state)
    static int Get_State_Check(unsigned int state);
};
}  // namespace Global

// IDA unit: 99_misc_global/Global. This symbol is global namespace in the
// IDA_EA: toTString @0x8ADEAB0/@0x8ADEAC3/@0x8ADEB7D;
// toUnicode @0x8ADF0F6/@0x8ADF1B3; GetEscapeString @0x861CC47;
// GetTenThousandPercentage @0x8550FC9; checkValidCharacName @0x820582B;
// GetLightServerFlag/IsLightServer/SetLightServerFlag
// @0x822AD3A/@0x822AD44/@0x8A8CBA5.
// old binary: _Z9toTStringRKSs.
// IDA_EA: references include AvatarRoulette::getAvatarList @0x8911DB0,
// AvatarRoulette::getStikerList @0x8911DFE, getCharacterJobName @0x88BE60E,
// and getGrowTypeName @0x88BF2B8. The old C++ ABI names this global enum as
// 17ENUM_CHARACTERJOB; enumerator names are not present in the exported IDA
// evidence, so this public baseline keeps only the typed ABI boundary.
enum ENUM_CHARACTERJOB : int;

extern const char* CHARACTER_JOB_TAG_NAMES[
    oldsrv::config::kCharacterJobWithCommonCount]
    __asm__("CHARACTER_JOB_TAG_NAMES");
extern unsigned int CREATECHARACTEREQUIP[
    oldsrv::config::kCharacterJobCount * 2]
    __asm__("CREATECHARACTEREQUIP");
extern int g_new_scriptVersion __asm__("g_new_scriptVersion");
extern int MAX_VILLAGE_NUM __asm__("MAX_VILLAGE_NUM");
unsigned char* GetLightServerFlag();
int IsLightServer();
unsigned char* SetLightServerFlag(bool flag);
unsigned char* GetCollectDungeonStatistic();
int IsCollectDungeonStatistic();
unsigned char* SetCollectDungeonStatistic(bool flag);

// IDA_EA: 0x84849B4 / 0x8485B79.  The statistics timer owns this singleton
// and sends its writes through the primary log DB handle.
CDungeonDataManager* GetInstanceDungeonDataManager();
MySQL* GetMySqLInstance();

// IDA_EA: 0x81ED09A
// IDA_NAME: _Z12EnterPVPRoomP5CUserP8PvP_Room14ENUM_CMDPACKETPKc
// DEMANGLED: EnterPVPRoom(CUser *,PvP_Room *,ENUM_CMDPACKET,char const*)
int EnterPVPRoom(CUser* user,
                 PvP_Room* room,
                 ENUM_CMDPACKET command,
                 const char* password);

// IDA_EA: 0x81F793C
// IDA_NAME: _Z11MakePVPRoomP5CUserR17MSG_MAKE_PVP_ROOM14ENUM_CMDPACKET15PVP_BATTLE_MODE
// DEMANGLED: MakePVPRoom(CUser *,MSG_MAKE_PVP_ROOM &,ENUM_CMDPACKET,PVP_BATTLE_MODE)
int MakePVPRoom(CUser* user,
                MSG_MAKE_PVP_ROOM& message,
                ENUM_CMDPACKET command,
                PVP_BATTLE_MODE battle_mode);

// IDA_EA: 0x849D768
// IDA_NAME: _Z11EnableWorldPK5CItemS1_
// DEMANGLED: EnableWorld(CItem const*,CItem const*)
bool EnableWorld(const CItem* first, const CItem* second);

// IDA_EA: 0x8115BF4
// IDA_NAME: _Z11filterEventi
// DEMANGLED: filterEvent(int)
int filterEvent(int event_id) __asm__("_Z11filterEventi");
const char* toTString(const std::string& source);
const char* toTString(const std::wstring& source);
const char* toTString(const wchar_t* source);
const wchar_t* toUnicode(const std::string& source);
const wchar_t* toUnicode(const char* source);
const char* toMbcs(const char* source);
int getCharacterJob(const char* text);
int checkValiedJobName(const char* text);
char* GetEscapeString(MySQL* mysql, const char* source, char* dest)
    __asm__("_Z15GetEscapeStringP5MySQLPKcPc");
char* AddFieldString(bool& first,
                     char* dest,
                     int flags,
                     int mask,
                     const char* format,
                     ...)
    __asm__("_Z14AddFieldStringRbPciiPKcz");
// IDA_EA: 0x83FF968
// IDA_NAME: _Z15getList2inQueryPjPc
// DEMANGLED: getList2inQuery(unsigned int*, char*)
char* getList2inQuery(unsigned int* buf, char* dest)
    __asm__("_Z15getList2inQueryPjPc");
// IDA_EA: 0x83FFA02
// IDA_NAME: _Z15getList2inQueryRKSt6vectorIiSaIiEEPc
// DEMANGLED: getList2inQuery(std::vector<int,std::allocator<int>> const&,char *)
char* getList2inQuery(const std::vector<int>& values, char* dest)
    __asm__("_Z15getList2inQueryRKSt6vectorIiSaIiEEPc");
int trim(std::string& text);
char* trim(char* text);
// IDA_EA: 0x829EFAE
// IDA_NAME: _Z16checkSpecialCharPc
// DEMANGLED: checkSpecialChar(char *)
int checkSpecialChar(char* text) __asm__("_Z16checkSpecialCharPc");
// IDA_EA: 0x829F01F
// IDA_NAME: _Z20checkSpecialCharNamePc
// DEMANGLED: checkSpecialCharName(char *)
int checkSpecialCharName(char* text) __asm__("_Z20checkSpecialCharNamePc");
int LineFunc(int line, const char* function, int error_no, unsigned int mid);
// IDA_EA: 0x822A98C
// IDA_NAME: _Z22getAvatarEmblemTypeIdxt
int getAvatarEmblemTypeIdx(unsigned short type_mask);
// IDA_EA: 0x822A9E5
// IDA_NAME: _Z39getAvatarEmblemTypeIdxForCompountEmblemtPiRi
// DEMANGLED: getAvatarEmblemTypeIdxForCompountEmblem(unsigned short,int *,int &)
int getAvatarEmblemTypeIdxForCompountEmblem(unsigned short type_mask,
                                            int* type_counts,
                                            int& total_count);
// IDA_EA: 0x8332E4E
// IDA_NAME: _Z12getItemIndexRKSt6vectorISt4pairIiiESaIS1_EEi
// DEMANGLED: getItemIndex(std::vector<std::pair<int,int>> const&,int)
//
// CompoundAvatar.etc stores already-cumulative weights in the pair's second
// member.  This process-global helper returns the first item whose boundary
// is strictly greater than the sampled value, or zero when no boundary wins.
int getItemIndex(const std::vector<std::pair<int, int> >& items,
                 int sampled_weight);
// IDA_EA: 0x8A9AFA2
// IDA_NAME: _Z22getSkillSpendSPOnStealii
// Stealable skills pay the original percentage of their computed SP/SFP
// consumption, floored to one point.
int getSkillSpendSPOnSteal(int spend, int penalty_percent);
int GetTenThousandPercentage(int value, int deduction_rate);
int GetEquipmentTypeStringToEnum(std::string& text);
// IDA_EA: 0x89BC76E
// IDA_NAME: _Z27getEquipmentTypeToEnumValueRSs
// DEMANGLED: getEquipmentTypeToEnumValue(std::string &)
int getEquipmentTypeToEnumValue(std::string& text)
    __asm__("_Z27getEquipmentTypeToEnumValueRSs");
int getCharacterJobToEnumValue(std::string& text);
int getGrowTypeIndex(const char* text);
// IDA_EA: 0x86B192D
// IDA_NAME: _Z16get_str_datetimePci
// DEMANGLED: get_str_datetime(char *,int)
int get_str_datetime(char* text, int length)
    __asm__("_Z16get_str_datetimePci");
double* GetMarketConditionRate(double base_amount,
                               double current_amount,
                               double* threshold_rates,
                               int previous_state,
                               int& out_state,
                               double& out_rate)
    __asm__("_Z22GetMarketConditionRateddPdiRiRd");
int MakeNotiSetDurability(PacketGuard& packet, int durability_rate)
    __asm__("_Z21MakeNotiSetDurabilityR11PacketGuardi");
// IDA_EA: 0x842AE3F
// IDA_NAME: _Z23saveSecurityCardHistoryP5MySQLj26ENUM_SECURITYCARD_MOD_TYPE
// DEMANGLED: saveSecurityCardHistory(MySQL *,unsigned int,ENUM_SECURITYCARD_MOD_TYPE)
// B keeps integer modify-type call sites today; the old enum name is preserved
// by the fixed asm symbol until concrete enumerators are needed.
int saveSecurityCardHistory(MySQL* db,
                            unsigned int account_id,
                            int modify_type)
    __asm__("_Z23saveSecurityCardHistoryP5MySQLj26ENUM_SECURITYCARD_MOD_TYPE");
// IDA_EA: 0x842AF12
// IDA_NAME: _Z19turnOffSecurityFlagP5MySQLj
// DEMANGLED: turnOffSecurityFlag(MySQL *,unsigned int)
int turnOffSecurityFlag(MySQL* db, unsigned int account_id)
    __asm__("_Z19turnOffSecurityFlagP5MySQLj");
// IDA_EA: 0x842AE90
// IDA_NAME: _Z18turnOnSecurityFlagP5MySQLj
// DEMANGLED: turnOnSecurityFlag(MySQL *,unsigned int)
int turnOnSecurityFlag(MySQL* db, unsigned int account_id)
    __asm__("_Z18turnOnSecurityFlagP5MySQLj");
// IDA_EA: 0x86B216D
// IDA_NAME: _Z28generateSecurityCardQuestionPc
// DEMANGLED: generateSecurityCardQuestion(char *)
unsigned int generateSecurityCardQuestion(char* p_src)
    __asm__("_Z28generateSecurityCardQuestionPc");
bool BitsetToStr(const std::bitset<256>& bits, std::string& text);
bool BitsetToStr(const std::bitset<256>& bits, unsigned char* text);
bool StrToBitset(unsigned char* text, std::bitset<256>& bits);
bool createLotteryInfo(LotteryInfo* info, std::vector<int>* values)
    __asm__("_Z17createLotteryInfoP11LotteryInfoPSt6vectorIiSaIiEE");
// IDA_EA: 0x846C0A8
// IDA_NAME: _Z22CheckDailyScheduleTimeill
// DEMANGLED: CheckDailyScheduleTime(int,long,long)
bool CheckDailyScheduleTime(int tm_hour, long last_time, long cur_time)
    __asm__("_Z22CheckDailyScheduleTimeill");
// IDA_EA: 0x846C144
// IDA_NAME: _Z23ComputeDaysScheduleTimeill
// DEMANGLED: ComputeDaysScheduleTime(int,long,long)
int ComputeDaysScheduleTime(int tm_hour, long last_time, long cur_time)
    __asm__("_Z23ComputeDaysScheduleTimeill");
// IDA_EA: 0x846C2E9
// IDA_NAME: _Z19getPrevScheduleTimeil
// DEMANGLED: getPrevScheduleTime(int,long)
long getPrevScheduleTime(int tm_hour, long timestamp)
    __asm__("_Z19getPrevScheduleTimeil");

namespace global {
namespace tools {

// IDA_EA: 0x8145E64
// IDA_NAME: _ZN6global5tools20InitializeCharacStat14initCharacStatER15_Additioal_infojiiii
// DEMANGLED: global::tools::InitializeCharacStat::initCharacStat(_Additioal_info &,unsigned int,int,int,int,int)
struct InitializeCharacStat {
    static int initCharacStat(_Additioal_info& stat,
                              unsigned int charac_no,
                              int job,
                              int level,
                              int first_grow_type,
                              int second_grow_type);
};

}  // namespace tools
}  // namespace global


// IDA unit: 99_misc_global/Global.
char* Char2Hex(unsigned char value, char* output);
int Hex2Char(const char* input, unsigned char& output);
void PrintBackTrace();
const char* NumberToString(unsigned int value, int slot);
const char* NumberToString(unsigned long long value, int slot);
unsigned int StringToNumber(const char* text);
std::time_t GetCurrentResetBaseTime(std::time_t timer, int reset_hour);
bool checkTimeToday(std::time_t timer, int reset_hour);
int checkValidCharacName(const char* text);
// IDA_EA: 0x8275315
// IDA_NAME: _Z21checkNewMemberHacklogjP5CUserj
// DEMANGLED: checkNewMemberHacklog(unsigned int,CUser *,unsigned int)
void checkNewMemberHacklog(unsigned int registration_date,
                           CUser* user,
                           unsigned int registration_seconds);
// IDA_EA: 0x86B1D25
// IDA_NAME: _Z18get_seed_from_datev
// DEMANGLED: get_seed_from_date(void)
int get_seed_from_date();
// IDA_EA: 0x889209D
// IDA_NAME: _Z18checkGetItemPacketiii
// DEMANGLED: checkGetItemPacket(int,int,int)
int checkGetItemPacket(int backup_seed, int first, int second);
// IDA_EA: 0x82138F2
// IDA_NAME: _Z27IsAuctionServerDisconnectedh
// DEMANGLED: IsAuctionServerDisconnected(unsigned char)
int IsAuctionServerDisconnected(uint8_t payment_type)
    __asm__("_Z27IsAuctionServerDisconnectedh");
// IDA_EA: 0x8213E15 / 0x8213E40.
// CMD186 calls these exact global integer predicates for its price and
// stackable-item overflow validation.  They remain named link edges here.
int IsPriceError(int first_price, int second_price)
    __asm__("_Z12IsPriceErrorii");
int IsValidMultiplyResult(int first_value, int multiplier)
    __asm__("_Z21IsValidMultiplyResultii");


#endif  // OLDSRV_GLOBAL_GLOBAL_TOOLS_H_
