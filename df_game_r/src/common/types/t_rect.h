#ifndef OLDSRV_GLOBAL_T_RECT_H_
#define OLDSRV_GLOBAL_T_RECT_H_

#include <stdint.h>


// IDA unit: 99_misc_global/T_Rect. Shared value type used by map/town script
// IDA_EA: ctor/dtor/clear @0x8AAE094/0x8AAE0A8/0x8AAE84A.
// records; no script population or map behavior is owned here.
struct T_Rect {
    T_Rect();
    ~T_Rect();

    T_Rect* clear();

    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;
};


#endif  // OLDSRV_GLOBAL_T_RECT_H_
