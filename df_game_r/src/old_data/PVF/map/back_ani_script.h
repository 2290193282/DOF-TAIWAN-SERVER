#ifndef OLDSRV_DATAMANAGER_BACK_ANI_SCRIPT_H_
#define OLDSRV_DATAMANAGER_BACK_ANI_SCRIPT_H_

#include <stdint.h>
#include <string>


// IDA unit: 12_pvf_datamanager_scripts/BackAniScript.
class BackAniScript {
public:
    BackAniScript();
    BackAniScript(const BackAniScript& rhs);
    ~BackAniScript();

    BackAniScript* clear();
    BackAniScript& operator=(const BackAniScript& rhs);

    std::string animation_name;  // +0x00 in old i386 libstdc++ layout.
    int32_t layer;               // +0x04, default 1.
    uint8_t flag_08;             // +0x08
    uint8_t flag_09;             // +0x09
    uint8_t flag_0a;             // +0x0a
    int32_t value_0c;            // +0x0c
    int32_t value_10;            // +0x10
};

#endif  // OLDSRV_DATAMANAGER_BACK_ANI_SCRIPT_H_
