#ifndef OLDSRV_GLOBAL_ARAD_PRINT_ERROR_H_
#define OLDSRV_GLOBAL_ARAD_PRINT_ERROR_H_


class CUser;

namespace ARAD {
namespace PRINT_ERROR {

// IDA_EA: 0x8197BA4
// IDA_NAME: _ZN4ARAD11PRINT_ERROR7isEmptyEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::isEmpty(char *,CUser *,unsigned short)
void isEmpty(char* context, CUser* user, unsigned short slot);

// IDA_EA: 0x8197BFD
// IDA_NAME: _ZN4ARAD11PRINT_ERROR13CheckItemLockEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::CheckItemLock(char *,CUser *,unsigned short)
void CheckItemLock(char* context, CUser* user, unsigned short slot);

// IDA_EA: 0x8197C56
// IDA_NAME: _ZN4ARAD11PRINT_ERROR11delete_itemEPcP5CUsert
// DEMANGLED: ARAD::PRINT_ERROR::delete_item(char *,CUser *,unsigned short)
void delete_item(char* context, CUser* user, unsigned short slot);

// IDA_EA: 0x8197CAF
// IDA_NAME: _ZN4ARAD11PRINT_ERROR9find_itemEPcP5CUseri
// DEMANGLED: ARAD::PRINT_ERROR::find_item(char *,CUser *,int)
void find_item(char* context, CUser* user, int item_id);

}  // namespace PRINT_ERROR
}  // namespace ARAD


#endif  // OLDSRV_GLOBAL_ARAD_PRINT_ERROR_H_
