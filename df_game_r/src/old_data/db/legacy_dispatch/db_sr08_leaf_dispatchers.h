#ifndef OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCHERS_H_
#define OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCHERS_H_

#include <map>

#include "old_data/db/core/db_dispatcher.h"
#include "old_data/db/legacy_dispatch/db_sr08_leaf_dispatch_payloads.h"

class DB_DeletePvPBuddy : public DBDispatcher { public: DB_DeletePvPBuddy(); virtual ~DB_DeletePvPBuddy(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_InsertPvPBuddy : public DBDispatcher { public: DB_InsertPvPBuddy(); virtual ~DB_InsertPvPBuddy(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_LoadPvPBuddy : public DBDispatcher { public: DB_LoadPvPBuddy(); virtual ~DB_LoadPvPBuddy(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_LoadPvPLiveEventData : public DBDispatcher { public: DB_LoadPvPLiveEventData(); virtual ~DB_LoadPvPLiveEventData(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdateDungeonDataStatisticLogdb : public DBDispatcher { public: DB_UpdateDungeonDataStatisticLogdb(); virtual ~DB_UpdateDungeonDataStatisticLogdb(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdatePvPBuddy : public DBDispatcher { public: DB_UpdatePvPBuddy(); virtual ~DB_UpdatePvPBuddy(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdatePvPLiveEventData : public DBDispatcher { public: DB_UpdatePvPLiveEventData(); virtual ~DB_UpdatePvPLiveEventData(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };

class DB_InsertDungeonEntrance : public DBDispatcher { public: DB_InsertDungeonEntrance(); virtual ~DB_InsertDungeonEntrance(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_InsertDungeonEntrance_hour : public DBDispatcher { public: DB_InsertDungeonEntrance_hour(); virtual ~DB_InsertDungeonEntrance_hour(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_InsertDungeonExp : public DBDispatcher { public: DB_InsertDungeonExp(); virtual ~DB_InsertDungeonExp(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_InsertLevelDungeonPlay : public DBDispatcher { public: DB_InsertLevelDungeonPlay(); virtual ~DB_InsertLevelDungeonPlay(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdateMemberPlayInfoPcRoom : public DBDispatcher { public: DB_UpdateMemberPlayInfoPcRoom(); virtual ~DB_UpdateMemberPlayInfoPcRoom(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_SaveProperDungeonClearCount : public DBDispatcher { public: DB_SaveProperDungeonClearCount(); virtual ~DB_SaveProperDungeonClearCount(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };

class DB_TestTradeRestrict : public DBDispatcher { public: DB_TestTradeRestrict(); virtual ~DB_TestTradeRestrict(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static void makeRequest(unsigned int charac_no); };
class DB_GenCeraTicket : public DBDispatcher { public: DB_GenCeraTicket(); virtual ~DB_GenCeraTicket(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdateMemberDoubtTrade : public DBDispatcher { public: DB_UpdateMemberDoubtTrade(); virtual ~DB_UpdateMemberDoubtTrade(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_UpdateGoblinPadAdvicePwdRewardTime : public DBDispatcher { public: DB_UpdateGoblinPadAdvicePwdRewardTime(); virtual ~DB_UpdateGoblinPadAdvicePwdRewardTime(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static void makeRequest(int user_idx, unsigned int member_id); };
class DB_UpdateFatigueAttendance : public DBDispatcher { public: DB_UpdateFatigueAttendance(); virtual ~DB_UpdateFatigueAttendance(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static void makeRequest(unsigned int member_id, const std::map<unsigned char, short>& fatigue_by_day); };
class DB_SaveConditionEventInfo : public DBDispatcher { public: DB_SaveConditionEventInfo(); virtual ~DB_SaveConditionEventInfo(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_Insert0712WinterEventCoupon : public DBDispatcher { public: DB_Insert0712WinterEventCoupon(); virtual ~DB_Insert0712WinterEventCoupon(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static Stream* makeRequest(unsigned int member_id, unsigned int charac_no, unsigned int coupon_count); static Stream* _makeRequest(ENUM_INTERNALPACKET signal, unsigned int member_id, unsigned int charac_no, unsigned int coupon_count); };
class DB_GoblinPadUpdateReward : public DBDispatcher { public: DB_GoblinPadUpdateReward(); virtual ~DB_GoblinPadUpdateReward(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static void makeRequest(unsigned int member_id, char modification_type, bool reward, char* ip_info); };
class DB_InsertArchieveEventLog : public DBDispatcher { public: DB_InsertArchieveEventLog(); virtual ~DB_InsertArchieveEventLog(); virtual bool dispatch(int signal, int user_idx, Stream* stream); static void makeRequest(unsigned int member_id, unsigned int charac_no, SIG_INSERT_ACHIEVE_EVENT::ENUM_ACHIEVE_TYPE achieve_type); };
class DB_LogCeraShopError : public DBDispatcher { public: DB_LogCeraShopError(); virtual ~DB_LogCeraShopError(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };
class DB_StatisticCeraShopGiftLog : public DBDispatcher { public: DB_StatisticCeraShopGiftLog(); virtual ~DB_StatisticCeraShopGiftLog(); virtual bool dispatch(int signal, int user_idx, Stream* stream); };

#endif  // OLDSRV_OLD_DATA_DB_LEGACY_DISPATCH_DB_SR08_LEAF_DISPATCHERS_H_
