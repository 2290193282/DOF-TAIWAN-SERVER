#include "common/manager/arad_print_error.h"

#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"


namespace ARAD {
namespace PRINT_ERROR {

// IDA_EA: 0x8197BA4
// IDA_NAME: _ZN4ARAD11PRINT_ERROR7isEmptyEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::isEmpty(char *,CUser *,unsigned short)
void isEmpty(char* context, CUser* user, unsigned short slot) {
    LogManager::logFormat(
        1,
        "localjapan/Arad_Function_Set.cpp",
        "void ARAD::PRINT_ERROR::isEmpty(char*, CUser*, short unsigned int)",
        0x29D,
        "%s : isEmpty() [slot:%d] [charac_no:%d]",
        context,
        slot,
        user->getCurCharacNo());
}

// IDA_EA: 0x8197BFD
// IDA_NAME: _ZN4ARAD11PRINT_ERROR13CheckItemLockEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::CheckItemLock(char *,CUser *,unsigned short)
void CheckItemLock(char* context, CUser* user, unsigned short slot) {
    LogManager::logFormat(
        1,
        "localjapan/Arad_Function_Set.cpp",
        "void ARAD::PRINT_ERROR::CheckItemLock(char*, CUser*, short unsigned int)",
        0x2A2,
        "%s : CheckItemLock() fail [slot:%d] [charac_no:%d]",
        context,
        slot,
        user->getCurCharacNo());
}

// IDA_EA: 0x8197C56
// IDA_NAME: _ZN4ARAD11PRINT_ERROR11delete_itemEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::delete_item(char *,CUser *,unsigned short)
void delete_item(char* context, CUser* user, unsigned short slot) {
    LogManager::logFormat(
        1,
        "localjapan/Arad_Function_Set.cpp",
        "void ARAD::PRINT_ERROR::delete_item(char*, CUser*, short unsigned int)",
        0x2A7,
        "%s : delete_item() [slot:%d] [charac_no:%d]",
        context,
        slot,
        user->getCurCharacNo());
}

// IDA_EA: 0x8197CAF
// IDA_NAME: _ZN4ARAD11PRINT_ERROR9find_itemEPcP5CUseri
// DEMANGLED: ARAD::PRINT_ERROR::find_item(char *,CUser *,int)
void find_item(char* context, CUser* user, int item_id) {
    LogManager::logFormat(
        1,
        "localjapan/Arad_Function_Set.cpp",
        "void ARAD::PRINT_ERROR::find_item(char*, CUser*, int)",
        0x2ac,
        "%s : find_item() fail [item_id:%d] [charac_no:%d]",
        context,
        item_id,
        user->getCurCharacNo());
}

}  // namespace PRINT_ERROR
}  // namespace ARAD
