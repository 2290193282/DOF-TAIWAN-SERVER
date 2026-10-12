#include "old_game/player/core/character_view_data.h"

#include <cstring>


typedef char CCharacterView_Size_Must_Be_0x01[
    (sizeof(CCharacterView) == 0x01) ? 1 : -1];
typedef char STGuildMemberCharacData_Size_Must_Be_0x21[
    (sizeof(STGuildMemberCharacData) == 0x21) ? 1 : -1];
typedef char STPowerWarCharacData_Size_Must_Be_0x0c[
    (sizeof(STPowerWarCharacData) == 0x0c) ? 1 : -1];

// IDA_EA: 0x822FBDA
// IDA_NAME: _ZN14CCharacterView20enableSaveCharacViewEv
// DEMANGLED: CCharacterView::enableSaveCharacView(void)
CCharacterView* CCharacterView::enableSaveCharacView() {
    save_charac_view = 1;
    return this;
}

// IDA_EA: 0x82A675E
// IDA_NAME: _ZN14CCharacterView16isSaveCharacViewEv
// DEMANGLED: CCharacterView::isSaveCharacView(void)
int CCharacterView::isSaveCharacView() {
    return save_charac_view;
}

// IDA_EA: 0x8697018
// IDA_NAME: _ZN14CCharacterView5resetEv
// DEMANGLED: CCharacterView::reset(void)
int CCharacterView::reset() {
    disableSaveCharacView();
    return 0;
}

// IDA_EA: 0x869702C
// IDA_NAME: _ZN14CCharacterView21disableSaveCharacViewEv
// DEMANGLED: CCharacterView::disableSaveCharacView(void)
CCharacterView* CCharacterView::disableSaveCharacView() {
    save_charac_view = 0;
    return this;
}

// IDA_EA: 0x8697038
// IDA_NAME: _ZN14CCharacterViewC2Ev
// DEMANGLED: CCharacterView::CCharacterView(void)
CCharacterView::CCharacterView() {}

// IDA_EA: 0x869703E
// IDA_NAME: _ZN14CCharacterViewD2Ev
// DEMANGLED: CCharacterView::~CCharacterView()
CCharacterView::~CCharacterView() {}

// IDA_EA: 0x822B802
// IDA_NAME: _ZN23STGuildMemberCharacDataC2Ev
// DEMANGLED: STGuildMemberCharacData::STGuildMemberCharacData(void)
STGuildMemberCharacData::STGuildMemberCharacData() {
    raw_00[0] = 0xff;
    raw_00[1] = 0xff;
    raw_00[2] = 0;
    std::memset(raw_00 + 3, 0, 0x1e);
}

// IDA_EA: 0x86959DC
// IDA_NAME: _ZN20STPowerWarCharacDataC2Ev
// DEMANGLED: STPowerWarCharacData::STPowerWarCharacData(void)
STPowerWarCharacData::STPowerWarCharacData()
    : word_00(0),
      byte_02(0),
      byte_03(1),
      dword_04(0),
      dword_08(0) {}
