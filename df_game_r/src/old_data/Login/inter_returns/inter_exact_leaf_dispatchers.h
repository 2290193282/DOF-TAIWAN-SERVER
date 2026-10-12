#ifndef OLDSRV_LOGIN_INTER_EXACT_LEAF_DISPATCHERS_H_
#define OLDSRV_LOGIN_INTER_EXACT_LEAF_DISPATCHERS_H_

#include "old_data/db/legacy_dispatch/sig_dispatcher.h"

class CUser;

// The five classes below are base-only ISigDispatcher objects in the original
// i386 InterDispatcher table.  Their EAs are retained with each definition.
class Inter_Terminate : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_Terminate();
    virtual ~Inter_Terminate();
};

class Inter_FatigueResetDBReply : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_FatigueResetDBReply();
    virtual ~Inter_FatigueResetDBReply();
};

class Inter_SuicideWhenQueueIsEmpty : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_SuicideWhenQueueIsEmpty();
    virtual ~Inter_SuicideWhenQueueIsEmpty();
};

class Inter_UserRepel : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_UserRepel();
    virtual ~Inter_UserRepel();
};

class Inter_UDP_Incoming_Capcha_Data : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_UDP_Incoming_Capcha_Data();
    virtual ~Inter_UDP_Incoming_Capcha_Data();
};

// IDA table entries 384, 388--392, 394 and 398.  These remain base-only
// receiver objects in the original i386 InterDispatcher table.
class Inter_LoadExpertJob : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_LoadExpertJob();
    virtual ~Inter_LoadExpertJob();
};

class Inter_SendAllUserInfoMiniumForGuildSystem : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_SendAllUserInfoMiniumForGuildSystem();
    virtual ~Inter_SendAllUserInfoMiniumForGuildSystem();
};

class Inter_ItemLimitEdition_Sell_Start : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ItemLimitEdition_Sell_Start();
    virtual ~Inter_ItemLimitEdition_Sell_Start();
};

class Inter_ItemLimitEdition_Sell_End : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ItemLimitEdition_Sell_End();
    virtual ~Inter_ItemLimitEdition_Sell_End();
};

class Inter_ItemLimitEdition_Update : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ItemLimitEdition_Update();
    virtual ~Inter_ItemLimitEdition_Update();
};

class Inter_ItemLimitEdition_Reply : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ItemLimitEdition_Reply();
    virtual ~Inter_ItemLimitEdition_Reply();
};

class Inter_FindFactoryHub : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_FindFactoryHub();
    virtual ~Inter_FindFactoryHub();
};

class Inter_GuildExpLimit : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildExpLimit();
    virtual ~Inter_GuildExpLimit();
};

// IDA table entries 405, 408--409, 411, 416--418 and 423.  The return
// payloads are consumed by the original gameplay/database notification chain;
// none of these receivers owns additional instance state.
class Inter_CancelRestingUser : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_CancelRestingUser();
    virtual ~Inter_CancelRestingUser();
};

class Inter_ApplyTempPowerSide : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ApplyTempPowerSide();
    virtual ~Inter_ApplyTempPowerSide();
};

class Inter_ReloadPowerWarRanker : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_ReloadPowerWarRanker();
    virtual ~Inter_ReloadPowerWarRanker();
};

class Inter_LoadPowerWarInfo : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_LoadPowerWarInfo();
    virtual ~Inter_LoadPowerWarInfo();
};

class Inter_SetFactoryHubUser : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_SetFactoryHubUser();
    virtual ~Inter_SetFactoryHubUser();
};

class Inter_SetFactoryHubUserNotConn : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_SetFactoryHubUserNotConn();
    virtual ~Inter_SetFactoryHubUserNotConn();
};

class Inter_Set_CleanPad_Point : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_Set_CleanPad_Point();
    virtual ~Inter_Set_CleanPad_Point();
};

class Inter_LoadNoCacheData : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_LoadNoCacheData();
    virtual ~Inter_LoadNoCacheData();
};

// IDA table entries 426--432 and 434 are the server-side responses to the
// CMD251--269 guild cargo protocol.  They remain base-only dispatcher objects.
class Inter_GuildCargo : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargo();
    virtual ~Inter_GuildCargo();
};

class Inter_NoticeGuildCargo : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_NoticeGuildCargo();
    virtual ~Inter_NoticeGuildCargo();
};

class Inter_GuildCargoHistory : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoHistory();
    virtual ~Inter_GuildCargoHistory();
};

class Inter_GuildCargoCheckPushItem : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoCheckPushItem();
    virtual ~Inter_GuildCargoCheckPushItem();
};

class Inter_GuildCargoPushItem : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoPushItem();
    virtual ~Inter_GuildCargoPushItem();
};

class Inter_GuildCargoPopItem : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoPopItem();
    virtual ~Inter_GuildCargoPopItem();
};

class Inter_GuildCargoMoveItem : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoMoveItem();
    virtual ~Inter_GuildCargoMoveItem();
};

class Inter_GuildCargoUpgrade : public ISigDispatcher {
public:
    int dispatch_sig(CUser* user, char* payload, int payload_size);
    Inter_GuildCargoUpgrade();
    virtual ~Inter_GuildCargoUpgrade();
};

#if defined(__i386__)
static_assert(sizeof(Inter_Terminate) == 0x04u,
              "Inter_Terminate must retain the original base-only ABI");
static_assert(sizeof(Inter_FatigueResetDBReply) == 0x04u,
              "Inter_FatigueResetDBReply must retain the original base-only ABI");
static_assert(sizeof(Inter_SuicideWhenQueueIsEmpty) == 0x04u,
              "Inter_SuicideWhenQueueIsEmpty must retain the original base-only ABI");
static_assert(sizeof(Inter_UserRepel) == 0x04u,
              "Inter_UserRepel must retain the original base-only ABI");
static_assert(sizeof(Inter_UDP_Incoming_Capcha_Data) == 0x04u,
              "Inter_UDP_Incoming_Capcha_Data must retain the original base-only ABI");
static_assert(sizeof(Inter_LoadExpertJob) == 0x04u,
              "Inter_LoadExpertJob must retain the original base-only ABI");
static_assert(sizeof(Inter_SendAllUserInfoMiniumForGuildSystem) == 0x04u,
              "Inter_SendAllUserInfoMiniumForGuildSystem must retain the original base-only ABI");
static_assert(sizeof(Inter_ItemLimitEdition_Sell_Start) == 0x04u,
              "Inter_ItemLimitEdition_Sell_Start must retain the original base-only ABI");
static_assert(sizeof(Inter_ItemLimitEdition_Sell_End) == 0x04u,
              "Inter_ItemLimitEdition_Sell_End must retain the original base-only ABI");
static_assert(sizeof(Inter_ItemLimitEdition_Update) == 0x04u,
              "Inter_ItemLimitEdition_Update must retain the original base-only ABI");
static_assert(sizeof(Inter_ItemLimitEdition_Reply) == 0x04u,
              "Inter_ItemLimitEdition_Reply must retain the original base-only ABI");
static_assert(sizeof(Inter_FindFactoryHub) == 0x04u,
              "Inter_FindFactoryHub must retain the original base-only ABI");
static_assert(sizeof(Inter_GuildExpLimit) == 0x04u,
              "Inter_GuildExpLimit must retain the original base-only ABI");
static_assert(sizeof(Inter_CancelRestingUser) == 0x04u &&
                  sizeof(Inter_ApplyTempPowerSide) == 0x04u &&
                  sizeof(Inter_ReloadPowerWarRanker) == 0x04u &&
                  sizeof(Inter_LoadPowerWarInfo) == 0x04u &&
                  sizeof(Inter_SetFactoryHubUser) == 0x04u &&
                  sizeof(Inter_SetFactoryHubUserNotConn) == 0x04u &&
                  sizeof(Inter_Set_CleanPad_Point) == 0x04u &&
                  sizeof(Inter_LoadNoCacheData) == 0x04u,
              "Inter 405--423 receivers must retain the original base-only ABI");
static_assert(sizeof(Inter_GuildCargo) == 0x04u &&
                  sizeof(Inter_NoticeGuildCargo) == 0x04u &&
                  sizeof(Inter_GuildCargoHistory) == 0x04u &&
                  sizeof(Inter_GuildCargoCheckPushItem) == 0x04u &&
                  sizeof(Inter_GuildCargoPushItem) == 0x04u &&
                  sizeof(Inter_GuildCargoPopItem) == 0x04u &&
                  sizeof(Inter_GuildCargoMoveItem) == 0x04u &&
                  sizeof(Inter_GuildCargoUpgrade) == 0x04u,
              "Inter guild-cargo receivers must retain the original base-only ABI");
#endif

#endif  // OLDSRV_LOGIN_INTER_EXACT_LEAF_DISPATCHERS_H_
