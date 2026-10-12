#include "old_game/economy/mail/mailbox_param_runtime.h"

// IDA_EA: 0x823109E
// IDA_NAME: _ZN18stMailboxSendParamC2Ev
// DEMANGLED: stMailboxSendParam::stMailboxSendParam(void)
stMailboxSendParam::stMailboxSendParam()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82479FC
// IDA_NAME: _ZN18stMailboxSendParamD2Ev
// DEMANGLED: stMailboxSendParam::~stMailboxSendParam()
stMailboxSendParam::~stMailboxSendParam() {}

// IDA_EA: 0x8247A2C
// IDA_NAME: _ZN18stMailboxSendParamD0Ev
// DEMANGLED: stMailboxSendParam::~stMailboxSendParam() deleting destructor
// The C++ ABI emits this deleting destructor from the virtual destructor
// definition above; it calls D2 and then operator delete, matching the old
// server body.

// IDA_EA: 0x823106E
// IDA_NAME: _ZN18stMailboxOpenParamC2Ev
// DEMANGLED: stMailboxOpenParam::stMailboxOpenParam(void)
stMailboxOpenParam::stMailboxOpenParam()
    : ParamBase(),
      result(0),
      not_loaded_mail_count(0) {}

// IDA_EA: 0x8247A4A
// IDA_NAME: _ZN18stMailboxOpenParamD2Ev
// DEMANGLED: stMailboxOpenParam::~stMailboxOpenParam()
stMailboxOpenParam::~stMailboxOpenParam() {}

// IDA_EA: 0x8247A7A
// IDA_NAME: _ZN18stMailboxOpenParamD0Ev
// DEMANGLED: stMailboxOpenParam::~stMailboxOpenParam() deleting destructor
// Emitted by the virtual destructor definition above.

// IDA_EA: 0x82310C4
// IDA_NAME: _ZN21stMailboxExtractParamC2Ev
// DEMANGLED: stMailboxExtractParam::stMailboxExtractParam(void)
stMailboxExtractParam::stMailboxExtractParam()
    : ParamBase(),
      result(0),
      unobserved_08(0) {}

// IDA_EA: 0x82479AE
// IDA_NAME: _ZN21stMailboxExtractParamD2Ev
// DEMANGLED: stMailboxExtractParam::~stMailboxExtractParam()
stMailboxExtractParam::~stMailboxExtractParam() {}

// IDA_EA: 0x82479DE
// IDA_NAME: _ZN21stMailboxExtractParamD0Ev
// DEMANGLED: stMailboxExtractParam::~stMailboxExtractParam() deleting destructor
// Emitted by the virtual destructor definition above.

// IDA_EA: 0x8231E30
// IDA_NAME: _ZN17stOpenCeraPackageC2Ev
// DEMANGLED: stOpenCeraPackage::stOpenCeraPackage(void)
stOpenCeraPackage::stOpenCeraPackage()
    : ParamBase(),
      result(0) {}

// IDA_EA: 0x82464DA
// IDA_NAME: _ZN17stOpenCeraPackageD2Ev
// DEMANGLED: stOpenCeraPackage::~stOpenCeraPackage()
stOpenCeraPackage::~stOpenCeraPackage() {}

// IDA_EA: 0x824650A
// IDA_NAME: _ZN17stOpenCeraPackageD0Ev
// DEMANGLED: stOpenCeraPackage::~stOpenCeraPackage() deleting destructor
// Emitted by the virtual destructor definition above.
