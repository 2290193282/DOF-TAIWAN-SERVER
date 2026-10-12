#include "old_data/Login/quest_authen_data.h"

typedef char _Quest_Authen_Data_Size_Must_Be_0x18[
    (sizeof(_Quest_Authen_Data) == 0x18) ? 1 : -1];

// IDA_EA: 0x814AA16
// IDA_NAME: _ZN18_Quest_Authen_Data5resetEv
// DEMANGLED: _Quest_Authen_Data::reset(void)
_Quest_Authen_Data* _Quest_Authen_Data::reset() {
    quest_id = 0;
    clear_count = 0;
    auth_time = 0;
    auth_flag = 0;
    auth_value_10 = 0;
    auth_value_14 = 0;
    return this;
}

// IDA_EA: 0x8694EF2
// IDA_NAME: _ZN18_Quest_Authen_DataC2Ev
// DEMANGLED: _Quest_Authen_Data::_Quest_Authen_Data(void)
_Quest_Authen_Data::_Quest_Authen_Data() { reset(); }
