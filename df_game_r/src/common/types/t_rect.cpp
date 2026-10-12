#include "common/types/t_rect.h"


// IDA_EA: 0x8AAE094
// IDA_NAME: _ZN6T_RectC2Ev
// DEMANGLED: T_Rect::T_Rect(void)
T_Rect::T_Rect() {
    clear();
}

// IDA_EA: 0x8AAE0A8
// IDA_NAME: _ZN6T_Rect5clearEv
// DEMANGLED: T_Rect::clear(void)
T_Rect* T_Rect::clear() {
    left = 0;
    top = 0;
    right = 0;
    bottom = 0;
    return this;
}

// IDA_EA: 0x8AAE84A
// IDA_NAME: _ZN6T_RectD2Ev
// DEMANGLED: T_Rect::~T_Rect()
T_Rect::~T_Rect() {
}
