#ifndef OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_H_
#define OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_H_

// Canonical B-owned PVF event declarations.

#include <stdint.h>

#include <string>


// IDA_EA: 0x8585510, 0x8A00A86, 0x8A00ABA, 0x8A02F24, 0x8A02FEA.
// IDA unit: 12_pvf_datamanager_scripts/PresentResponceScript.
class PresentResponceScript {
public:
    PresentResponceScript();
    PresentResponceScript(const PresentResponceScript& other);
    ~PresentResponceScript();

    PresentResponceScript* clear();
    PresentResponceScript& operator=(const PresentResponceScript& other);

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    std::string value_0C;
};

#endif  // OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_H_
