#ifndef OLDSRV_DATAMANAGER_INTERACTIVE_DIALOG_SCRIPT_H_
#define OLDSRV_DATAMANAGER_INTERACTIVE_DIALOG_SCRIPT_H_

#include <stdint.h>

#include <string>
#include <vector>


// IDA_EA: 0x83D7FA4, 0x83DCA8E, 0x8A0098A, 0x8A009F2, 0x8A02B96.
// IDA unit: 12_pvf_datamanager_scripts/InteractiveDialogScript.
class InteractiveDialogScript {
public:
    InteractiveDialogScript();
    InteractiveDialogScript(const InteractiveDialogScript& other);
    ~InteractiveDialogScript();

    int clear();
    InteractiveDialogScript& operator=(const InteractiveDialogScript& other);

    int32_t value_00;
    int32_t value_04;
    int32_t value_08;
    std::vector<int> value_0C;
    uint8_t value_18;
    int32_t value_1C;
    uint8_t value_20;
    std::string value_24;
    std::vector<int> value_28;
};

#endif  // OLDSRV_DATAMANAGER_INTERACTIVE_DIALOG_SCRIPT_H_
