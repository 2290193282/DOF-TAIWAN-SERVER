#ifndef OLDSRV_OLD_GAME_USERINFO_CHANGE_EMOTION_PACKET_DISPATCHER_H_
#define OLDSRV_OLD_GAME_USERINFO_CHANGE_EMOTION_PACKET_DISPATCHER_H_

#include <stdint.h>

#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"

class CUser;
class PacketBuf;

// The original process catches this named fixed-width enum.  Its enumerators
// are not recovered in this slice, so retain only the proven named ABI and
// use the two observed numeric values locally.
enum ENUM_ERROR : int;

// CMD258 raw template storage is not constructed.  This named type describes
// the 15-byte read/process ABI only; it intentionally has no C2 identity.
#pragma pack(push, 1)
struct MSG_CHANGE_EMOTION : public MSG_BASE {
    int16_t emotion;
};
#pragma pack(pop)

static_assert(sizeof(MSG_CHANGE_EMOTION) == 0x0fu,
              "CMD258 must retain the 13-byte C2S header plus emotion short");

struct stChangeEmotionParam : public ParamBase {
    // IDA_EA: 0x8231468
    // IDA_NAME: _ZN20stChangeEmotionParamC2Ev
    // DEMANGLED: stChangeEmotionParam::stChangeEmotionParam(void)
    stChangeEmotionParam();

    // IDA_EA: 0x82472AC
    // IDA_NAME: _ZN20stChangeEmotionParamD2Ev
    // DEMANGLED: stChangeEmotionParam::~stChangeEmotionParam()
    // IDA_EA: 0x82472DC
    // IDA_NAME: _ZN20stChangeEmotionParamD0Ev
    // DEMANGLED: stChangeEmotionParam::~stChangeEmotionParam()
    virtual ~stChangeEmotionParam();

    int16_t emotion;
};

#if defined(__i386__)
static_assert(sizeof(stChangeEmotionParam) == 0x08u,
              "CMD258 parameter must retain the i386 ParamBase/short ABI");
#endif

class Dispatcher_ChangeEmotion
    : public IPacketDispatcher<MSG_CHANGE_EMOTION,
                               stChangeEmotionParam,
                               static_cast<ch_state>(3)> {
public:
    // IDA_EA: 0x8596D58
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotionC2Ev
    // DEMANGLED: Dispatcher_ChangeEmotion::Dispatcher_ChangeEmotion(void)
    Dispatcher_ChangeEmotion();

    // IDA_EA: 0x8245724
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotionD2Ev
    // DEMANGLED: Dispatcher_ChangeEmotion::~Dispatcher_ChangeEmotion()
    // IDA_EA: 0x8245754
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotionD0Ev
    // DEMANGLED: Dispatcher_ChangeEmotion::~Dispatcher_ChangeEmotion()
    virtual ~Dispatcher_ChangeEmotion();

    // IDA_EA: 0x81D1B08
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotion4readER9PacketBufR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeEmotion::read(PacketBuf &,MSG_BASE &)
    virtual int read(PacketBuf& packet, MSG_BASE& message);

    // IDA_EA: 0x81D1B5E
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotion7processEP5CUserR8MSG_BASER9ParamBase
    // DEMANGLED: Dispatcher_ChangeEmotion::process(CUser *,MSG_BASE &,ParamBase &)
    virtual int process(CUser* user, MSG_BASE& message, ParamBase& param);

    // IDA_EA: 0x81D1CEC
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotion11check_errorEP5CUserR8MSG_BASE
    // DEMANGLED: Dispatcher_ChangeEmotion::check_error(CUser *,MSG_BASE &)
    int check_error(CUser* user, MSG_BASE& message);

    // IDA_EA: 0x81D1D60
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotion4sendEP5CUserR9ParamBase
    // DEMANGLED: Dispatcher_ChangeEmotion::send(CUser *,ParamBase &)
    virtual void send(CUser* user, ParamBase& param);

    // IDA_EA: 0x81D1EBE
    // IDA_NAME: _ZN24Dispatcher_ChangeEmotion19isVaildEmotionValueEs
    // DEMANGLED: Dispatcher_ChangeEmotion::isVaildEmotionValue(short)
    int isVaildEmotionValue(int16_t emotion);
};

#if defined(__i386__)
static_assert(sizeof(Dispatcher_ChangeEmotion) == 0x04u,
              "CMD258 dispatcher must retain its i386 base-only ABI");
#endif

#endif  // OLDSRV_OLD_GAME_USERINFO_CHANGE_EMOTION_PACKET_DISPATCHER_H_
