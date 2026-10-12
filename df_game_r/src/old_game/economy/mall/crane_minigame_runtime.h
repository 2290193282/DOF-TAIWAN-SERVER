#ifndef OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CRANE_MINIGAME_RUNTIME_H_
#define OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CRANE_MINIGAME_RUNTIME_H_

#include <stdint.h>

#include "old_data/PVF/etc/crane_minigame_script.h"

class CUser;

// IDA unit: 99_misc_global/CraneMinigameManager.
// The manager extends the parsed PVF script by the one-byte material rotation
// index at +0x68.  Its inherited script owns all item/mail/material state.
class CraneMinigameManager : public CraneMinigameScript {
public:
    // IDA_EA: 0x80EBC40
    // IDA_NAME: _ZN20CraneMinigameManagerC2Ev
    // DEMANGLED: CraneMinigameManager::CraneMinigameManager(void)
    CraneMinigameManager();

    // IDA_EA: 0x80EBC5A
    // IDA_NAME: _ZN20CraneMinigameManagerD2Ev
    // DEMANGLED: CraneMinigameManager::~CraneMinigameManager()
    ~CraneMinigameManager();

    // IDA_EA: 0x80EBC6E
    // IDA_NAME: _ZN20CraneMinigameManager4initEv
    // DEMANGLED: CraneMinigameManager::init(void)
    int init();

    // IDA_EA: 0x80EBCE0
    // IDA_NAME: _ZN20CraneMinigameManager13isPickSuccessEsi
    // DEMANGLED: CraneMinigameManager::isPickSuccess(short,int)
    int isPickSuccess(int16_t item_index, int item_id);

    // IDA_EA: 0x80EBD44
    // IDA_NAME: _ZN20CraneMinigameManager10getItemCntEi
    // DEMANGLED: CraneMinigameManager::getItemCnt(int)
    int getItemCnt(int item_index);

    // IDA_EA: 0x80EBD7A
    // IDA_NAME: _ZN20CraneMinigameManager8sendItemEP5CUseri
    // DEMANGLED: CraneMinigameManager::sendItem(CUser *,int)
    int sendItem(CUser* user, int item_index);

    // IDA_EA: 0x80EBE06
    // IDA_NAME: _ZN20CraneMinigameManager16startUseMaterialEP5CUseri
    // DEMANGLED: CraneMinigameManager::startUseMaterial(CUser *,int)
    int startUseMaterial(CUser* user, int item_slot);

    // IDA_EA: 0x80EBEBC
    // IDA_NAME: _ZN20CraneMinigameManager27updateCraneItemNeedMaterialEi
    // DEMANGLED: CraneMinigameManager::updateCraneItemNeedMaterial(int)
    int updateCraneItemNeedMaterial(int rotation_seed);

    // IDA_EA: 0x80EBF2E
    // IDA_NAME: _ZN20CraneMinigameManager25SendCraneItemNeedMaterialEP5CUser
    // DEMANGLED: CraneMinigameManager::SendCraneItemNeedMaterial(CUser *)
    void SendCraneItemNeedMaterial(CUser* user);

    uint8_t material_rotation_index_;  // +0x68
};

#if defined(__i386__)
static_assert(sizeof(CraneMinigameManager) == 0x6cu,
              "CraneMinigameManager must retain its 0x6c-byte i386 layout");
#endif

#endif  // OLDSRV_OLD_GAME_MAIL_TRADE_CASH_CRANE_MINIGAME_RUNTIME_H_
