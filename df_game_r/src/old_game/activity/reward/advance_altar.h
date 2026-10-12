#ifndef OLDSRV_GLOBAL_ADVANCE_ALTAR_H_
#define OLDSRV_GLOBAL_ADVANCE_ALTAR_H_

#include <string>

class MySQL;
class CUser;

namespace advancealtar {

struct StageDifficulty {
    enum T {
        NONE = -1,
        EASY = 0,
        MEDIUM = 1,
        HARD = 2
    };

    static T getNextDifficulty(T difficulty);
    static bool getDifficulty(const std::string& text, T& difficulty);
};

namespace RidableId {
enum T {
    FIRST = 1
};
}  // namespace RidableId

namespace AdvanceAltarSlotType {
enum T {
    UNIT = 1,
    SKILL = 2
};
}  // namespace AdvanceAltarSlotType

// IDA_EA: 0x8133DB0 consumes the value as the first field of the
// `star-,%d,%d` character-history record.  CMD573 uses value zero; keeping
// the named enum preserves the original typed HistoryLog ABI.
namespace SubStarType {
enum T {
    BUY_ITEM = 0
};
}  // namespace SubStarType

// IDA_EA: 0x8133D56 / 0x81333B2.  The reset-star transaction records a
// positive-star history event with type 2.  Keeping the original nested enum
// is also required by CharacAdvanceAltarManager::resetStar's i386 ABI.
namespace AddStarType {
enum T {
    RESET_STAR = 2
};
}  // namespace AddStarType

// IDA_EA: 0x812CFEA
// IDA_NAME: _ZN12advancealtar18isOpenAdvanceAltarEv
// DEMANGLED: advancealtar::isOpenAdvanceAltar(void)
bool isOpenAdvanceAltar();

// IDA_EA: 0x812CFA6.  Event 119 is the original event-only reward branch
// selector used by Advance Altar achievement mail.
bool isOnEvent();

// IDA_EA: 0x8898FF0
// IDA_NAME: _ZN12advancealtar16isValidRidableIdENS_9RidableId1TE
// DEMANGLED: advancealtar::isValidRidableId(advancealtar::RidableId::T)
bool isValidRidableId(RidableId::T ridable_id);

// IDA_EA: 0x812CF54
// IDA_NAME: _ZN12advancealtar14getMemberNamesEP5CUserPc
// DEMANGLED: advancealtar::getMemberNames(CUser *,char *)
void getMemberNames(CUser* user, char* buffer);

bool convertZipToString(MySQL* db_handle,
                        int column,
                        char* zip_buffer,
                        unsigned int zip_capacity,
                        char* text_buffer,
                        unsigned int text_capacity);
bool convertStringToZip(char* zip_buffer,
                        unsigned int zip_capacity,
                        const char* text,
                        unsigned int text_length,
                        unsigned int& zip_length);

extern char g_ZipStageBuffer[0x800];
extern char g_StrStageBuffer[0x800];
extern char g_ZipSlotBuffer[0x800];
extern char g_StrSlotBuffer[0x800];
extern char g_ZipBuyItemBuffer[0x800];
extern char g_StrBuyItemBuffer[0x800];
extern char g_ZipRewardBuffer[0x800];
extern char g_StrRewardBuffer[0x800];

}  // namespace advancealtar

#endif  // OLDSRV_GLOBAL_ADVANCE_ALTAR_H_
