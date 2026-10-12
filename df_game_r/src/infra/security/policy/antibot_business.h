#ifndef OLDSRV_SECURITY_ANTIBOT_BUSINESS_H_
#define OLDSRV_SECURITY_ANTIBOT_BUSINESS_H_

#include <stdint.h>

enum EPUNISHMODE : int;
struct tagAntibotPunishInfo;
struct tagTSafeModePkg;

class IBusiness {
public:
    // IDA_EA: 0x82A7C24
    // IDA_NAME: _ZN9IBusinessC2Ev
    // DEMANGLED: IBusiness::IBusiness(void)
    IBusiness();

    // IDA vtable off_8C1BAD0: slots +0x00, +0x04, and +0x08 are pure.
    // There is no virtual destructor before these callbacks.
    virtual int SendBuffertoClient(int destination,
                                  int unused_1,
                                  int unused_2,
                                  unsigned int serial,
                                  unsigned char* buffer,
                                  short length) = 0;
    virtual int PunishClient(int unused_1,
                             int unused_2,
                             unsigned int destination,
                             EPUNISHMODE mode,
                             tagAntibotPunishInfo* punish_info,
                             char* unused_3) = 0;
    virtual int ProcessProtectMsg(tagTSafeModePkg* message) = 0;
};

class CBusinessImpl : public IBusiness {
public:
    // IDA_EA: 0x82A7C32
    // IDA_NAME: _ZN13CBusinessImplC2Ev
    // DEMANGLED: CBusinessImpl::CBusinessImpl(void)
    CBusinessImpl();

    // IDA_EA: 0x82A7C4E
    // IDA_NAME: _ZN13CBusinessImplD2Ev
    // DEMANGLED: CBusinessImpl::~CBusinessImpl()
    ~CBusinessImpl();

    // IDA_EA: 0x80DE240
    // IDA_NAME: _ZN13CBusinessImpl18SendBuffertoClientEiiijPhs
    // DEMANGLED: CBusinessImpl::SendBuffertoClient(int,int,int,unsigned int,unsigned char *,short)
    virtual int SendBuffertoClient(int destination,
                                  int unused_1,
                                  int unused_2,
                                  unsigned int serial,
                                  unsigned char* buffer,
                                  short length);

    // IDA_EA: 0x80DE3EE
    // IDA_NAME: _ZN13CBusinessImpl12PunishClientEiij11EPUNISHMODEP20tagAntibotPunishInfoPc
    // DEMANGLED: CBusinessImpl::PunishClient(int,int,unsigned int,EPUNISHMODE,tagAntibotPunishInfo *,char *)
    virtual int PunishClient(int unused_1,
                             int unused_2,
                             unsigned int destination,
                             EPUNISHMODE mode,
                             tagAntibotPunishInfo* punish_info,
                             char* unused_3);

    // IDA_EA: 0x80DE4EC
    // IDA_NAME: _ZN13CBusinessImpl17ProcessProtectMsgEP15tagTSafeModePkg
    // DEMANGLED: CBusinessImpl::ProcessProtectMsg(tagTSafeModePkg *)
    virtual int ProcessProtectMsg(tagTSafeModePkg* message);
};

#endif  // OLDSRV_SECURITY_ANTIBOT_BUSINESS_H_
