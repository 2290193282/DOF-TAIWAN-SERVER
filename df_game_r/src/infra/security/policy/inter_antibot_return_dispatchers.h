#ifndef OLDSRV_SECURITY_INTER_ANTIBOT_RETURN_DISPATCHERS_H_
#define OLDSRV_SECURITY_INTER_ANTIBOT_RETURN_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// InterDispatcher slots 594--596 are the database/anti-bot bridge.  They
// receive the three messages emitted by CBusinessImpl and re-enter the live
// C2S anti-bot state machine through the original S2C notifications.
class Inter_Antibot_SendBufferClient : public ISigDispatcher {
public:
    Inter_Antibot_SendBufferClient();
    virtual ~Inter_Antibot_SendBufferClient();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Antibot_PunishClient : public ISigDispatcher {
public:
    Inter_Antibot_PunishClient();
    virtual ~Inter_Antibot_PunishClient();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

class Inter_Antibot_ProcessProtectMsg : public ISigDispatcher {
public:
    Inter_Antibot_ProcessProtectMsg();
    virtual ~Inter_Antibot_ProcessProtectMsg();
    virtual int dispatch_sig(CUser* user, char* payload, int payload_size);
};

#if defined(__i386__)
static_assert(sizeof(Inter_Antibot_SendBufferClient) == 0x04u,
              "Inter_Antibot_SendBufferClient must retain base-only ABI");
static_assert(sizeof(Inter_Antibot_PunishClient) == 0x04u,
              "Inter_Antibot_PunishClient must retain base-only ABI");
static_assert(sizeof(Inter_Antibot_ProcessProtectMsg) == 0x04u,
              "Inter_Antibot_ProcessProtectMsg must retain base-only ABI");
#endif

#endif  // OLDSRV_SECURITY_INTER_ANTIBOT_RETURN_DISPATCHERS_H_
