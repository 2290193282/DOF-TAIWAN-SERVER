#include "old_game/activity/reward/advance_altar.h"

#include "infra/concurrent/timer/daily_schedule_manager.h"
#include "common/types/compressed_blob.h"
#include "old_game/activity/reward/event_manager_abi.h"
#include "common/manager/global_data.h"
#include "old_data/db/core/mysql.h"
#include "old_game/player/core/cuser.h"

#include <cstdio>
#include <cstring>

namespace advancealtar {

char g_ZipStageBuffer[0x800];
char g_StrStageBuffer[0x800];
char g_ZipSlotBuffer[0x800];
char g_StrSlotBuffer[0x800];
char g_ZipBuyItemBuffer[0x800];
char g_StrBuyItemBuffer[0x800];
char g_ZipRewardBuffer[0x800];
char g_StrRewardBuffer[0x800];

// IDA_EA: 0x889900A
// IDA_NAME: _ZN12advancealtar15StageDifficulty17getNextDifficultyENS0_1TE
// DEMANGLED: advancealtar::StageDifficulty::getNextDifficulty(advancealtar::StageDifficulty::T)
StageDifficulty::T StageDifficulty::getNextDifficulty(
    StageDifficulty::T difficulty) {
    switch (difficulty) {
        case EASY:
            return MEDIUM;
        case MEDIUM:
            return HARD;
        case NONE:
            return EASY;
        default:
            return difficulty;
    }
}

// IDA_EA: 0x8899038
// IDA_NAME: _ZN12advancealtar15StageDifficulty13getDifficultyERKSsRNS0_1TE
// DEMANGLED: advancealtar::StageDifficulty::getDifficulty(std::string const&,advancealtar::StageDifficulty::T &)
bool StageDifficulty::getDifficulty(const std::string& text,
                                    StageDifficulty::T& difficulty) {
    if (text == "easy") {
        difficulty = EASY;
        return true;
    }
    if (text == "medium") {
        difficulty = MEDIUM;
        return true;
    }
    if (text == "hard") {
        difficulty = HARD;
        return true;
    }
    return false;
}

// IDA_EA: 0x812CFEA
// IDA_NAME: _ZN12advancealtar18isOpenAdvanceAltarEv
// DEMANGLED: advancealtar::isOpenAdvanceAltar(void)
bool isOpenAdvanceAltar() {
    return GlobalData::s_DailyScheduleManager->GetScheduleState(
               DAILY_SCHEDULE_ADVANCE_ALTAR, 10) == 1;
}

// IDA_EA: 0x812CFA6
// IDA_NAME: _ZN12advancealtar9isOnEventEv
// DEMANGLED: advancealtar::isOnEvent(void)
bool isOnEvent() {
    return GlobalData::s_event_manager->GetRepeatEvent(119)->IsEventing(0) ==
           1;
}

// IDA_EA: 0x8898FF0
// IDA_NAME: _ZN12advancealtar16isValidRidableIdENS_9RidableId1TE
// DEMANGLED: advancealtar::isValidRidableId(advancealtar::RidableId::T)
bool isValidRidableId(RidableId::T ridable_id) {
    return ridable_id == RidableId::FIRST;
}

// IDA_EA: 0x812CF54
// IDA_NAME: _ZN12advancealtar14getMemberNamesEP5CUserPc
// DEMANGLED: advancealtar::getMemberNames(CUser *,char *)
void getMemberNames(CUser* user, char* buffer) {
    if (user && buffer) {
        const char* const name = user->getCurCharacName();
        std::sprintf(buffer, "\"%s\",\"%s\",\"N/A\",\"N/A\",\"N/A\"",
                     name, name);
    }
}

// IDA_EA: 0x813DC48
// IDA_NAME: _ZN12advancealtar18convertZipToStringEP5MySQLiPcjS2_j
// DEMANGLED: advancealtar::convertZipToString(MySQL *,int,char *,unsigned int,char *,unsigned int)
bool convertZipToString(MySQL* db_handle,
                        int column,
                        char* zip_buffer,
                        unsigned int zip_capacity,
                        char* text_buffer,
                        unsigned int text_capacity) {
    const int binary_length = db_handle->get_binary_length(column);
    if (!binary_length) {
        return true;
    }
    if (zip_capacity <= static_cast<unsigned int>(binary_length)) {
        return false;
    }

    std::memset(zip_buffer, 0, zip_capacity);
    if (!db_handle->get_binary(column, zip_buffer, binary_length)) {
        return false;
    }

    unsigned int unpacked_length = 0;
    std::memcpy(&unpacked_length, zip_buffer, sizeof(unpacked_length));
    unpacked_length &= 0x3fffffffu;
    if (text_capacity <= unpacked_length) {
        return false;
    }

    std::memset(text_buffer, 0, text_capacity);
    return uncompress_zip(text_buffer,
                          unpacked_length,
                          zip_buffer,
                          static_cast<unsigned int>(binary_length)) == 1;
}

// IDA_EA: 0x813DD3E
// IDA_NAME: _ZN12advancealtar18convertStringToZipEPcjPKcjRj
// DEMANGLED: advancealtar::convertStringToZip(char *,unsigned int,char const*,unsigned int,unsigned int &)
bool convertStringToZip(char* zip_buffer,
                        unsigned int zip_capacity,
                        const char* text,
                        unsigned int text_length,
                        unsigned int& zip_length) {
    if (!text_length) {
        zip_length = 0;
        return true;
    }

    zip_length = zip_capacity;
    std::memset(zip_buffer, 0, zip_capacity);
    return compress_zip(zip_buffer, zip_length, text, text_length) == 1;
}

}  // namespace advancealtar
