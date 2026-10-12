#ifndef OLDSRV_SECURITY_SECU_DATA_CONTROL_RUNTIME_H_
#define OLDSRV_SECURITY_SECU_DATA_CONTROL_RUNTIME_H_

#include <stdint.h>

class CUser;

// IDA unit: 99_misc_global/Secu_DataControl.  The original C2 is empty and
// GlobalData::Init allocates exactly one byte for this process-lifetime
// command service.
class Secu_DataControl {
public:
    // IDA_EA: 0x82AA6BA
    // IDA_NAME: _ZN16Secu_DataControlC2Ev
    // DEMANGLED: Secu_DataControl::Secu_DataControl(void)
    Secu_DataControl();

    // IDA_EA: 0x8286A30
    // IDA_NAME: _ZN16Secu_DataControl14SetControlDataEP5CUserhtt
    // DEMANGLED: Secu_DataControl::SetControlData(
    //     CUser *,unsigned char,unsigned short,unsigned short)
    char SetControlData(CUser* user,
                        uint8_t command,
                        uint16_t first_value,
                        uint16_t second_value);

    // IDA_EA: 0x8286E40
    // IDA_NAME: _ZN16Secu_DataControl26UpdateSecurityServiceStateEP5CUserhhPc
    // DEMANGLED: Secu_DataControl::UpdateSecurityServiceState(
    //     CUser *,unsigned char,unsigned char,char *)
    void UpdateSecurityServiceState(CUser* user,
                                    uint8_t service_kind,
                                    uint8_t service_state,
                                    char* source);
};

#if defined(__i386__)
static_assert(sizeof(Secu_DataControl) == 0x01u,
              "Secu_DataControl must retain its old one-byte extent");
#endif

#endif  // OLDSRV_SECURITY_SECU_DATA_CONTROL_RUNTIME_H_
