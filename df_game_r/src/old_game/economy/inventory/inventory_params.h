#ifndef OLDSRV_06_USERINFO_STAT_INVENTORY_INVENTORY_PARAMS_H_
#define OLDSRV_06_USERINFO_STAT_INVENTORY_INVENTORY_PARAMS_H_
// Header evidence marker: active declarations in this file are backed by
// IDA_EA 0x8230AE4 in the companion source.


#include <stdint.h>

#include "infra/network/packet/message_base.h"


// IDA unit: 06_userinfo_stat_inventory/stOnUseEquipParam.
struct stOnUseEquipParam : public ParamBase {
    stOnUseEquipParam();
    virtual ~stOnUseEquipParam();

    int result;
    uint8_t slot;
    uint8_t padding_09[3];
};

// IDA unit: 06_userinfo_stat_inventory/stOnGetUserInfoParam.
struct stOnGetUserInfoParam : public ParamBase {
    stOnGetUserInfoParam();
    virtual ~stOnGetUserInfoParam();

    uint16_t target_id;
    uint8_t detail_type;
    uint8_t padding_07;
};

// IDA unit: 06_userinfo_stat_inventory/stRepairEquipParam.
struct stRepairEquipParam : public ParamBase {
    stRepairEquipParam();
    virtual ~stRepairEquipParam();

    int result;
    uint8_t inven_type;
    uint8_t padding_09;
    uint16_t slot;
    uint16_t repair_cost;
    uint8_t padding_0e[2];
};

// IDA unit: 06_userinfo_stat_inventory/stImageCommunicationEquipmentUse.
struct stImageCommunicationEquipmentUse : public ParamBase {
    stImageCommunicationEquipmentUse();
    virtual ~stImageCommunicationEquipmentUse();

    int result;
    int money_cost;
    int cur_money;
};


#endif  // OLDSRV_06_USERINFO_STAT_INVENTORY_INVENTORY_PARAMS_H_
