#include "old_game/world/region/world_params.h"

#include <new>


typedef char stMoveMapParam_Size_Must_Be_0x08[
    (sizeof(stMoveMapParam) == 0x08) ? 1 : -1];
typedef char stSetAreaParam_Size_Must_Be_0x0c[
    (sizeof(stSetAreaParam) == 0x0c) ? 1 : -1];

// IDA_EA: 0x8230D26
// IDA_NAME: _ZN14stMoveMapParamC2Ev
// DEMANGLED: stMoveMapParam::stMoveMapParam(void)
stMoveMapParam::stMoveMapParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x8248014
// IDA_NAME: _ZN14stMoveMapParamD2Ev
// DEMANGLED: stMoveMapParam::~stMoveMapParam()
stMoveMapParam::~stMoveMapParam() {}

// IDA_EA: 0x8248044
// IDA_NAME: _ZN14stMoveMapParamD0Ev
// DEMANGLED: stMoveMapParam::~stMoveMapParam() deleting destructor
void stMoveMapParam_deleting_destructor(stMoveMapParam* self) {
    if (self) {
        self->~stMoveMapParam();
        operator delete(self);
    }
}

// IDA_EA: 0x8230E26
// IDA_NAME: _ZN14stSetAreaParamC2Ev
// DEMANGLED: stSetAreaParam::stSetAreaParam(void)
stSetAreaParam::stSetAreaParam()
    : ParamBase(),
      result(0),
      area(0),
      forbidden_area(0) {}

// IDA_EA: 0x8247E8E
// IDA_NAME: _ZN14stSetAreaParamD2Ev
// DEMANGLED: stSetAreaParam::~stSetAreaParam()
stSetAreaParam::~stSetAreaParam() {}

// IDA_EA: 0x8247EBE
// IDA_NAME: _ZN14stSetAreaParamD0Ev
// DEMANGLED: stSetAreaParam::~stSetAreaParam() deleting destructor
void stSetAreaParam_deleting_destructor(stSetAreaParam* self) {
    if (self) {
        self->~stSetAreaParam();
        operator delete(self);
    }
}
