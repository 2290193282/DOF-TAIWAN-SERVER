#ifndef OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_RUNTIME_H_
#define OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_RUNTIME_H_

#include <stddef.h>
#include <stdint.h>
#include <type_traits>
#include <utility>
#include <vector>

#include "old_game/economy/inventory/inventory.h"
#include "old_game/economy/inventory/item_lock_runtime.h"
#include "old_game/player/core/charac_expand_cdata.h"

class CUser;
class PacketGuard;

enum ENUM_TITLE_BOOK_CATEGORY : int;
enum ENUM_TITLE_BOOK_INFO_TYPE : int;

struct stTitleBook {
    // IDA_EA: 0x848F62A
    // IDA_NAME: _ZN11stTitleBookC2Ev
    // DEMANGLED: stTitleBook::stTitleBook(void)
    stTitleBook();

    uint8_t category_0[70][0x3d];
    uint8_t category_1[200][0x3d];
    uint8_t category_2[120][0x3d];
    uint8_t category_3[60][0x3d];

    // IDA_EA: 0x848F6E4
    // IDA_NAME: _ZN11stTitleBook5clearEv
    // DEMANGLED: stTitleBook::clear(void)
    uint8_t* clear();
};

static_assert(offsetof(stTitleBook, category_1) == 0x10AEu,
              "stTitleBook category_1 offset must stay 0x10ae");
static_assert(offsetof(stTitleBook, category_2) == 0x4056u,
              "stTitleBook category_2 offset must stay 0x4056");
static_assert(offsetof(stTitleBook, category_3) == 0x5CEEu,
              "stTitleBook category_3 offset must stay 0x5cee");
static_assert(sizeof(stTitleBook) == 0x6B3Au,
              "stTitleBook IDA ABI size must stay 0x6b3a");
static_assert(std::is_trivially_destructible<stTitleBook>::value,
              "stTitleBook must keep the old constructor-only item lifetime");

class CTitleBook : public charac_expand::CData {
public:
    // IDA_EA: 0x864103C
    // IDA_NAME: _ZN10CTitleBookC2Ev
    // DEMANGLED: CTitleBook::CTitleBook(void)
    CTitleBook();

    // IDA_EA: 0x86433FE
    // IDA_NAME: _ZN10CTitleBookD2Ev
    // DEMANGLED: CTitleBook::~CTitleBook()
    // IDA_EA: 0x864342E
    // IDA_NAME: _ZN10CTitleBookD0Ev
    // DEMANGLED: CTitleBook::~CTitleBook()
    virtual ~CTitleBook();

    // IDA_EA: 0x86410D6
    // IDA_NAME: _ZN10CTitleBook9_gm_clearEv
    // DEMANGLED: CTitleBook::_gm_clear(void)
    void _gm_clear();

    // IDA_EA: 0x86410F8
    // IDA_NAME: _ZNK10CTitleBook7getDataEPc
    // DEMANGLED: CTitleBook::getData(char *)const
    virtual void getData(char* dest) const;

    // IDA_EA: 0x8641124
    // IDA_NAME: _ZNK10CTitleBook12findItemLockEhR14ENUM_ITEMSPACERi
    // DEMANGLED: CTitleBook::findItemLock(unsigned char,ENUM_ITEMSPACE &,int &)const
    int findItemLock(unsigned char lock,
                     ENUM_ITEMSPACE& item_space,
                     int& slot) const;

    // IDA_EA: 0x86411BA
    // IDA_NAME: _ZN10CTitleBook15getItemLockListERSt6vectorIN9item_lock13stItemLockRefESaIS2_EE
    // DEMANGLED: CTitleBook::getItemLockList(std::vector<item_lock::stItemLockRef,std::allocator<item_lock::stItemLockRef>> &)
    bool getItemLockList(std::vector<item_lock::stItemLockRef>& locks);

    // IDA_EA: 0x864125A
    // IDA_NAME: _ZN10CTitleBook17deleteTimeoutItemERKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CTitleBook::deleteTimeoutItem(std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    void deleteTimeoutItem(
        const std::vector<std::pair<int, int> >& matched_items);

    // IDA_EA: 0x864165E
    // IDA_NAME: _ZN10CTitleBook8loadDataEP5CUserPc
    // DEMANGLED: CTitleBook::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* src);

    // IDA_EA: 0x86416A4
    // IDA_NAME: _ZN10CTitleBook13getTitleSlotRE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::getTitleSlotR(ENUM_TITLE_BOOK_CATEGORY,int)
    Inven_Item getTitleSlotR(ENUM_TITLE_BOOK_CATEGORY category, int slot);

    // IDA_EA: 0x86416D8
    // IDA_NAME: _ZN10CTitleBook12getTitleSlotE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::getTitleSlot(ENUM_TITLE_BOOK_CATEGORY,int)
    Inven_Item* getTitleSlot(ENUM_TITLE_BOOK_CATEGORY category, int slot);

    // IDA_EA: 0x8641704
    // IDA_NAME: _ZN10CTitleBook8sendListEv
    // DEMANGLED: CTitleBook::sendList(void)
    bool sendList();

    // IDA_EA: 0x8641750
    // IDA_NAME: _ZN10CTitleBook17sendListOtherUserEP5CUser25ENUM_TITLE_BOOK_INFO_TYPE
    // DEMANGLED: CTitleBook::sendListOtherUser(CUser *,ENUM_TITLE_BOOK_INFO_TYPE)
    bool sendListOtherUser(CUser* user, ENUM_TITLE_BOOK_INFO_TYPE type);

    // IDA_EA: 0x86417AA
    // IDA_NAME: _ZN10CTitleBook17_sendCategoryDataEP5CUsert24ENUM_TITLE_BOOK_CATEGORY25ENUM_TITLE_BOOK_INFO_TYPE
    // DEMANGLED: CTitleBook::_sendCategoryData(CUser *,unsigned short,ENUM_TITLE_BOOK_CATEGORY,ENUM_TITLE_BOOK_INFO_TYPE)
    void _sendCategoryData(CUser* user,
                           unsigned short unique_id,
                           ENUM_TITLE_BOOK_CATEGORY category,
                           ENUM_TITLE_BOOK_INFO_TYPE type);

    // IDA_EA: 0x8641994
    // IDA_NAME: _ZN10CTitleBook18_getOwnerTitleInfoER24ENUM_TITLE_BOOK_CATEGORYRiP10Inven_Item
    // DEMANGLED: CTitleBook::_getOwnerTitleInfo(ENUM_TITLE_BOOK_CATEGORY &,int &,Inven_Item *)
    int _getOwnerTitleInfo(ENUM_TITLE_BOOK_CATEGORY& category,
                           int& slot,
                           Inven_Item* out);

    // IDA_EA: 0x8641A6A
    // IDA_NAME: _ZN10CTitleBook12_putItemDataER11PacketGuardsR10Inven_Item
    // DEMANGLED: CTitleBook::_putItemData(PacketGuard &,short,Inven_Item &)
    int _putItemData(PacketGuard& packet, short slot, Inven_Item& item);

    // IDA_EA: 0x8641B84
    // IDA_NAME: _ZN10CTitleBook21_getCategoryItemCountE24ENUM_TITLE_BOOK_CATEGORY
    // DEMANGLED: CTitleBook::_getCategoryItemCount(ENUM_TITLE_BOOK_CATEGORY)
    int _getCategoryItemCount(ENUM_TITLE_BOOK_CATEGORY category);

    // IDA_EA: 0x8641BDC
    // IDA_NAME: _ZN10CTitleBook17_checkInsertTitleE24ENUM_TITLE_BOOK_CATEGORYiP10Inven_Item
    // DEMANGLED: CTitleBook::_checkInsertTitle(ENUM_TITLE_BOOK_CATEGORY,int,Inven_Item *)
    bool _checkInsertTitle(ENUM_TITLE_BOOK_CATEGORY category,
                           int slot,
                           Inven_Item* item);

    // IDA_EA: 0x8641C52
    // IDA_NAME: _ZN10CTitleBook17_checkMovetoInvenE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::_checkMovetoInven(ENUM_TITLE_BOOK_CATEGORY,int)
    static bool _checkMovetoInven(ENUM_TITLE_BOOK_CATEGORY category,
                                  int slot);

    // IDA_EA: 0x8641D16
    // IDA_NAME: _ZN10CTitleBook12_insertTitleE24ENUM_TITLE_BOOK_CATEGORYiP10Inven_Item
    // DEMANGLED: CTitleBook::_insertTitle(ENUM_TITLE_BOOK_CATEGORY,int,Inven_Item *)
    Inven_Item* _insertTitle(ENUM_TITLE_BOOK_CATEGORY category,
                             int slot,
                             Inven_Item* item);

    // IDA_EA: 0x8641D50
    // IDA_NAME: _ZN10CTitleBook12_deleteTitleE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::_deleteTitle(ENUM_TITLE_BOOK_CATEGORY,int)
    Inven_Item* _deleteTitle(ENUM_TITLE_BOOK_CATEGORY category, int slot);

    // IDA_EA: 0x8641D84
    // IDA_NAME: _ZN10CTitleBook9_getTitleE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::_getTitle(ENUM_TITLE_BOOK_CATEGORY,int)
    Inven_Item* _getTitle(ENUM_TITLE_BOOK_CATEGORY category, int slot);

    // IDA_EA: 0x8641DB0
    // IDA_NAME: _ZN10CTitleBook10_getTitleRE24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::_getTitleR(ENUM_TITLE_BOOK_CATEGORY,int)
    Inven_Item _getTitleR(ENUM_TITLE_BOOK_CATEGORY category, int slot);

    // IDA_EA: 0x8641DF2
    // IDA_NAME: _ZN10CTitleBook6_emptyEv
    // DEMANGLED: CTitleBook::_empty(void)
    int _empty();

    // IDA_EA: 0x8641E60
    // IDA_NAME: _ZN10CTitleBook12putTitleOnlyEP5CUseri
    // DEMANGLED: CTitleBook::putTitleOnly(CUser *,int)
    static int putTitleOnly(CUser* user, int title_reward);

    // IDA_EA: 0x8641E88
    // IDA_NAME: _ZN10CTitleBook12putTitleOnlyEP5CUseriR24ENUM_TITLE_BOOK_CATEGORYRi
    // DEMANGLED: CTitleBook::putTitleOnly(CUser *,int,ENUM_TITLE_BOOK_CATEGORY &,int &)
    static int putTitleOnly(CUser* user,
                            int title_reward,
                            ENUM_TITLE_BOOK_CATEGORY& category,
                            int& slot);

    // IDA_EA: 0x8641FAC
    // IDA_NAME: _ZN10CTitleBook10checkErrorEP5CUser24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::checkError(CUser *,ENUM_TITLE_BOOK_CATEGORY,int)
    static int checkError(CUser* user,
                          ENUM_TITLE_BOOK_CATEGORY category,
                          int slot);

    // IDA_EA: 0x8642058
    // IDA_NAME: _ZN10CTitleBook8putTitleEP5CUser14ENUM_ITEMSPACEii24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::putTitle(CUser *,ENUM_ITEMSPACE,int,int,ENUM_TITLE_BOOK_CATEGORY,int)
    static int putTitle(CUser* user,
                        ENUM_ITEMSPACE item_space,
                        int source_slot,
                        int item_id,
                        ENUM_TITLE_BOOK_CATEGORY category,
                        int title_slot);

    // IDA_EA: 0x86425C4
    // IDA_NAME: _ZN10CTitleBook8getTitleEP5CUser24ENUM_TITLE_BOOK_CATEGORYii14ENUM_ITEMSPACERi
    // DEMANGLED: CTitleBook::getTitle(CUser *,ENUM_TITLE_BOOK_CATEGORY,int,int,ENUM_ITEMSPACE,int &)
    static int getTitle(CUser* user,
                        ENUM_TITLE_BOOK_CATEGORY category,
                        int title_slot,
                        int item_id,
                        ENUM_ITEMSPACE item_space,
                        int& output_slot);

    // IDA_EA: 0x8642BAA
    // IDA_NAME: _ZN10CTitleBook11_getFitSlotEiR24ENUM_TITLE_BOOK_CATEGORYRi
    // DEMANGLED: CTitleBook::_getFitSlot(int,ENUM_TITLE_BOOK_CATEGORY &,int &)
    static int _getFitSlot(int title_reward,
                           ENUM_TITLE_BOOK_CATEGORY& category,
                           int& slot);

    // IDA_EA: 0x8642D42
    // IDA_NAME: _ZN10CTitleBook13_checkFitSlotEi24ENUM_TITLE_BOOK_CATEGORYi
    // DEMANGLED: CTitleBook::_checkFitSlot(int,ENUM_TITLE_BOOK_CATEGORY,int)
    static bool _checkFitSlot(int title_reward,
                              ENUM_TITLE_BOOK_CATEGORY category,
                              int slot);

    // IDA_EA: 0x8642EC2
    // IDA_NAME: _ZN10CTitleBook14isAbleItemLockEm
    // DEMANGLED: CTitleBook::isAbleItemLock(unsigned long)
    static bool isAbleItemLock(unsigned long item_id);

protected:
    // IDA_EA: 0x86414A8
    // IDA_NAME: _ZN10CTitleBook9_saveDataEP5CUser
    // DEMANGLED: CTitleBook::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x86410B6
    // IDA_NAME: _ZN10CTitleBook6_resetEv
    // DEMANGLED: CTitleBook::_reset(void)
    virtual int _reset();

private:
    // IDA_EA: 0x864306E
    // IDA_NAME: _ZN10CTitleBook14_isMatchedItemEiRKSt6vectorISt4pairIiiESaIS2_EE
    // DEMANGLED: CTitleBook::_isMatchedItem(int,std::vector<std::pair<int,int>,std::allocator<std::pair<int,int>>> const&)
    int _isMatchedItem(
        int item_id,
        const std::vector<std::pair<int, int> >& matched_items);

    CUser* owner_;                    // IDA +0x0008
    stTitleBook title_book_;          // IDA +0x000c
    Inven_Item* category_slots_[4];   // IDA +0x6b48
};

static_assert(sizeof(CTitleBook) == 0x6B58u,
              "CTitleBook IDA ABI size must stay 0x6b58");

#endif  // OLDSRV_OLD_GAME_USERINFO_TITLE_BOOK_RUNTIME_H_
