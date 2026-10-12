#ifndef OLDSRV_APSYSTEM_ACTION_POINT_PACKET_DISPATCHERS_H_
#define OLDSRV_APSYSTEM_ACTION_POINT_PACKET_DISPATCHERS_H_

#include <stddef.h>
#include <stdint.h>

#include "old_data/db/legacy_dispatch/action_point_payload.h"
#include "infra/network/packet/PacketHeader.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"


class CUser;
class PacketBuf;

namespace APSystem {

// The same 11 bytes are parsed by CMD552 and written by SendActionClear.
// In the incoming path +0x04 is passed to ClearActionAndSendtoUser; in the
// outgoing notification it remains zero and +0x08/+0x0a carry the result.
struct __attribute__((packed)) _MSG_ACTION_POINT_ACTION_CLEAR {
    _MSG_ACTION_POINT_ACTION_CLEAR();

    _ActionGroupIndex action_group_index;  // +0x00
    int action_value_or_reserved;          // +0x04
    uint16_t action_index;                 // +0x08
    bool clear_state;                      // +0x0a
};

static_assert(sizeof(_MSG_ACTION_POINT_ACTION_CLEAR) == 0x0bu,
              "AP clear packet body must preserve 11 bytes");
static_assert(offsetof(_MSG_ACTION_POINT_ACTION_CLEAR, action_group_index) ==
                  0x00u &&
                  offsetof(_MSG_ACTION_POINT_ACTION_CLEAR,
                           action_value_or_reserved) == 0x04u &&
                  offsetof(_MSG_ACTION_POINT_ACTION_CLEAR, action_index) ==
                      0x08u &&
                  offsetof(_MSG_ACTION_POINT_ACTION_CLEAR, clear_state) ==
                      0x0au,
              "AP clear packet field layout must preserve the 0627 ABI");

struct __attribute__((packed)) _MSG_ACTION_POINT_GET_REWARD_ITEM {
    bool result;
    _SIG_LOAD_ACTION_POINT::_TodayRewardItem reward_item;
};

static_assert(sizeof(_MSG_ACTION_POINT_GET_REWARD_ITEM) == 0x05u,
              "AP reward packet body must preserve five bytes");
static_assert(offsetof(_MSG_ACTION_POINT_GET_REWARD_ITEM, reward_item) == 0x01u,
              "AP reward packet item field must stay at +0x01");

class CUserProc {
public:
    // IDA_EA: 0x812238A
    // IDA_NAME: _ZN8APSystem9CUserProc33SetTodayActionAndCheckMedalRewardEP5CUser
    static void SetTodayActionAndCheckMedalReward(CUser* user);
    static void ClearActionAndSendtoUser(CUser* user,
                                         _ActionGroupIndex action_group_index,
                                         int action_value,
                                         ENUM_PACKETCLASS packet_class);
    // IDA_EA: 0x8122398
    // IDA_NAME: _ZN8APSystem9CUserProc19SendActionListStateEP5CUser
    static void SendActionListState(CUser* user);
    static void SendActionClear(CUser& user,
                                uint16_t action_index,
                                _ActionGroupIndex action_group_index,
                                bool clear_state,
                                ENUM_PACKETCLASS packet_class);
    // IDA_EA: 0x81224A0
    // IDA_NAME: _ZN8APSystem9CUserProc19SendActionListStateER5CUser
    static void SendActionListState(CUser& user);
    static void SendGetRewardItem(
        CUser& user,
        _SIG_LOAD_ACTION_POINT::_TodayRewardItem reward_item,
        bool result);
    static void GetTodayRewardAndSendPostal(
        CUser* user,
        _SIG_LOAD_ACTION_POINT::_TodayRewardItem reward_item);
    // IDA_EA: 0x8122564
    // IDA_NAME: _ZN8APSystem9CUserProc28CheckAndSendToMessageAllUserEjjPKc
    // DEMANGLED: APSystem::CUserProc::CheckAndSendToMessageAllUser(
    //     unsigned int,unsigned int,char const*)
    static void CheckAndSendToMessageAllUser(
        uint32_t previous_action_point,
        uint32_t action_point,
        const char* character_name);

    // IDA_EA: 0x812256C
    // IDA_NAME: _ZN8APSystem9CUserProc17UpdateActionPointEP5CUser
    // DEMANGLED: APSystem::CUserProc::UpdateActionPoint(CUser *)
    static void UpdateActionPoint(CUser* user);

    // IDA_EA: 0x8122572
    // IDA_NAME: _ZN8APSystem9CUserProc20SendToMessageAllUserEPKcS2_
    // DEMANGLED: APSystem::CUserProc::SendToMessageAllUser(
    //     char const*,char const*)
    static void SendToMessageAllUser(const char* message_argument,
                                     const char* format);

    // IDA_EA: 0x81225D2
    // IDA_NAME: _ZN8APSystem9CUserProc8GM_ResetEP5CUser
    // DEMANGLED: APSystem::CUserProc::GM_Reset(CUser *)
    static void GM_Reset(CUser* user);

    // IDA_EA: 0x81225EE
    // IDA_NAME: _ZN8APSystem9CUserProc6GM_SetEP5CUserj
    // DEMANGLED: APSystem::CUserProc::GM_Set(CUser *,unsigned int)
    static void GM_Set(CUser* user, uint32_t action_point);
};

class Dispatcher_ClearAction
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_ClearAction();
    virtual ~Dispatcher_ClearAction();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

class Dispatcher_GetTodayReward
    : public IPacketDispatcher<MSG_BASE,
                               ParamBase,
                               static_cast<ch_state>(0)> {
public:
    Dispatcher_GetTodayReward();
    virtual ~Dispatcher_GetTodayReward();

    virtual int dispatch_sig(CUser* user, PacketBuf& packet);
    virtual int read(PacketBuf& packet, MSG_BASE& message);
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ClearAction) == 0x04u,
              "CMD552 dispatcher must retain its i386 base-only ABI");
static_assert(sizeof(Dispatcher_GetTodayReward) == 0x04u,
              "CMD553 dispatcher must retain its i386 base-only ABI");
#endif

}  // namespace APSystem

#endif  // OLDSRV_APSYSTEM_ACTION_POINT_PACKET_DISPATCHERS_H_
