#ifndef OLDSRV_DATAMANAGER_EMOTICON_SCRIPT_H_
#define OLDSRV_DATAMANAGER_EMOTICON_SCRIPT_H_

#include <stdint.h>

#include <string>


// IDA_EA: 0x80E602E, 0x80E607A.
namespace EmoticonType {

// IDA unit: 12_pvf_datamanager_scripts/EmoticonType_EmoticonScript.
struct EmoticonScript {
    EmoticonScript();
    ~EmoticonScript();

    int32_t index;
    int32_t value_04;
    int32_t value_08;
    std::string value_0C;
    std::string value_10;
};

}  // namespace EmoticonType

#endif  // OLDSRV_DATAMANAGER_EMOTICON_SCRIPT_H_
