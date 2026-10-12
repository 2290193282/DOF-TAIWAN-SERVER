#ifndef OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PARAMS_H_
#define OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PARAMS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8230D26 in the companion source.


#include <stdint.h>

#include "infra/network/packet/message_base.h"


// IDA unit: 05_world_enter_village/stMoveMapParam.
struct stMoveMapParam : public ParamBase {
    stMoveMapParam();
    virtual ~stMoveMapParam();

    int result;
};

void stMoveMapParam_deleting_destructor(stMoveMapParam* self);

// IDA unit: 05_world_enter_village/stSetAreaParam.
struct stSetAreaParam : public ParamBase {
    stSetAreaParam();
    virtual ~stSetAreaParam();

    int result;
    uint8_t area;
    uint8_t forbidden_area;
    uint8_t pad_0a[2];
};

void stSetAreaParam_deleting_destructor(stSetAreaParam* self);


#endif  // OLDSRV_05_WORLD_ENTER_VILLAGE_WORLD_PARAMS_H_
