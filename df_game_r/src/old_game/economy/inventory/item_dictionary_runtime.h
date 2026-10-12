#ifndef OLDSRV_OLD_GAME_INVENTORY_ITEM_DICTIONARY_RUNTIME_H_
#define OLDSRV_OLD_GAME_INVENTORY_ITEM_DICTIONARY_RUNTIME_H_

#include <stdint.h>

#include <vector>

#include "old_game/player/core/charac_expand_cdata.h"

class CUser;

// IDA unit: 99_misc_global/CItemDictionary.
// charac_expand::CDataMgr slot 16 owns 40,000 equipment discovery bits.
class CItemDictionary : public charac_expand::CData {
public:
    // IDA_EA: 0x811D5D4
    // IDA_NAME: _ZN15CItemDictionaryC2Ev
    // DEMANGLED: CItemDictionary::CItemDictionary(void)
    CItemDictionary();

    // IDA_EA: 0x811D5F0
    // IDA_NAME: _ZN15CItemDictionaryD2Ev
    // DEMANGLED: CItemDictionary::~CItemDictionary()
    // IDA_EA: 0x811D620
    // IDA_NAME: _ZN15CItemDictionaryD0Ev
    // DEMANGLED: CItemDictionary::~CItemDictionary()
    virtual ~CItemDictionary();

    // IDA_EA: 0x811D79A
    // IDA_NAME: _ZN15CItemDictionary8loadDataEP5CUserPc
    // DEMANGLED: CItemDictionary::loadData(CUser *,char *)
    virtual bool loadData(CUser* user, char* data);

    // IDA_EA: 0x811D7E0
    // IDA_NAME: _ZNK15CItemDictionary7getDataEPc
    // DEMANGLED: CItemDictionary::getData(char *)const
    virtual void getData(char* data) const;

    // IDA_EA: 0x811D80E
    // IDA_NAME: _ZN15CItemDictionary15setGainItemFlagEP5CUserj
    // DEMANGLED: CItemDictionary::setGainItemFlag(CUser *,unsigned int)
    void setGainItemFlag(CUser* user, unsigned int item_id);

    // IDA_EA: 0x811D8FC
    // IDA_NAME: _ZN15CItemDictionary16setItemCheckFlagEP5CUserjj
    // DEMANGLED: CItemDictionary::setItemCheckFlag(CUser *,unsigned int,unsigned int)
    void setItemCheckFlag(CUser* user,
                          unsigned int item_id,
                          unsigned int item_flag);

    // IDA_EA: 0x811DA32
    // IDA_NAME: _ZNK15CItemDictionary16getItemCheckFlagEjj
    // DEMANGLED: CItemDictionary::getItemCheckFlag(unsigned int,unsigned int)const
    bool getItemCheckFlag(unsigned int charac_no,
                          unsigned int item_flag) const;

    // IDA_EA: 0x811DAEE
    // IDA_NAME: _ZNK15CItemDictionary16getGainItemCountEv
    // DEMANGLED: CItemDictionary::getGainItemCount(void)const
    int getGainItemCount() const;

    // IDA_EA: 0x811DB60
    // IDA_NAME: _ZNK15CItemDictionary15getLengthBinaryEv
    // DEMANGLED: CItemDictionary::getLengthBinary(void)const
    int getLengthBinary() const;

    // IDA_EA: 0x811DBA6
    // IDA_NAME: _ZN15CItemDictionary15getGainItemListEv
    // DEMANGLED: CItemDictionary::getGainItemList(void)
    std::vector<int> getGainItemList();

    // IDA_EA: 0x811DCE6
    // IDA_NAME: _ZN15CItemDictionary22sendItemDictionaryInfoEP5CUserj
    // DEMANGLED: CItemDictionary::sendItemDictionaryInfo(CUser *,unsigned int)
    void sendItemDictionaryInfo(CUser* user, unsigned int item_id);

    // IDA_EA: 0x811DD9A
    // IDA_NAME: _ZN15CItemDictionary27sendItemDictionaryInfoTotalEP5CUser
    // DEMANGLED: CItemDictionary::sendItemDictionaryInfoTotal(CUser *)
    void sendItemDictionaryInfoTotal(CUser* user);

protected:
    // IDA_EA: 0x811D63E
    // IDA_NAME: _ZN15CItemDictionary9_saveDataEP5CUser
    // DEMANGLED: CItemDictionary::_saveData(CUser *)
    virtual int _saveData(CUser* user);

    // IDA_EA: 0x811D774
    // IDA_NAME: _ZN15CItemDictionary6_resetEv
    // DEMANGLED: CItemDictionary::_reset(void)
    virtual int _reset();

private:
    uint8_t check_flags_[5000];  // IDA +0x05; CData tail padding is reused.
};

static_assert(sizeof(CItemDictionary) == 0x1390u,
              "CItemDictionary IDA ABI size must stay 0x1390");

#endif  // OLDSRV_OLD_GAME_INVENTORY_ITEM_DICTIONARY_RUNTIME_H_
