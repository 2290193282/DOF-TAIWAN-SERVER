#ifndef OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_EXCEPTION_H_
#define OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_EXCEPTION_H_

// Canonical B-owned PVF event declarations.

#include <stdint.h>
#include <string>


// IDA unit: 12_pvf_datamanager_scripts/PresentResponceScriptException.
class PresentResponceScriptException {
public:
    PresentResponceScriptException();
    ~PresentResponceScriptException();
    PresentResponceScriptException(
        const PresentResponceScriptException& other);

    PresentResponceScriptException& operator=(
        const PresentResponceScriptException& other);
    PresentResponceScriptException* clear();

    int32_t value_00;
    std::string value_04;
};

#endif  // OLDSRV_DATAMANAGER_PRESENT_RESPONSE_SCRIPT_EXCEPTION_H_
