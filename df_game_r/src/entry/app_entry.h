#ifndef OLDSRV_APP_APP_ENTRY_H_
#define OLDSRV_APP_APP_ENTRY_H_


const char* SelectKeyString(int* key_group, int* key_index);
int InitCipherKeyString();
int RegistServerSchedule();

// IDA: regist_signal/signal_handler/regist_signals
// @0x829F1FC/@0x829F28B/@0x829F4C5.  The installed handler is deliberately
// restricted to sig_atomic_t stores; ProcessPendingOSSignals performs the
// shutdown work from MainLoop's normal thread context.
bool regist_signal(int signal_number, void (*handler)(int));
void signal_handler(int signal_number);
bool regist_signals();
void ProcessPendingOSSignals();

// IDA: _Z4InitiPPc / Init(int,char**) @0x82A08F4.
char Init(int argc, char** argv);

// IDA: _Z8MainLoopv @0x829FA2B.
int MainLoop();

// IDA: _Z26ProcessAntiBotInputWrapperv @0x80EDDB0 and
// _Z22ProcessServerStatusLogv @0x80EDE91.
int ProcessAntiBotInputWrapper();
int ProcessServerStatusLog();


#endif  // OLDSRV_APP_APP_ENTRY_H_
