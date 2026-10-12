#ifndef OLDSRV_GLOBAL_MONEY_LOG_ABI_H_
#define OLDSRV_GLOBAL_MONEY_LOG_ABI_H_


class CUser;

// IDA_EA: 0x84FEBD9
// IDA_NAME: _Z20write_log_gain_money15eMoneyAddReasoniP5CUser
// DEMANGLED: write_log_gain_money(eMoneyAddReason,int,CUser*)
void write_log_gain_money(int reason, int money, CUser* user)
    __asm__("_Z20write_log_gain_money15eMoneyAddReasoniP5CUser");

// IDA_EA: 0x84FED9B
// IDA_NAME: _Z19write_log_use_money15eMoneySubReasoniP5CUser
// DEMANGLED: write_log_use_money(eMoneySubReason,int,CUser*)
char* write_log_use_money(int reason, int money, CUser* user)
    __asm__("_Z19write_log_use_money15eMoneySubReasoniP5CUser");


#endif  // OLDSRV_GLOBAL_MONEY_LOG_ABI_H_
