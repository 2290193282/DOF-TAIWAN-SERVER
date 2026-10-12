#include "old_game/player/core/change_emotion_packet_dispatcher.h"

#include <type_traits>

#include "old_game/player/core/action_point_packet_dispatchers.h"
#include "common/manager/data_manager.h"
#include "common/manager/global_lifecycle.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_data/PVF/etc/emotion_type_tags.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/packet/packet_guard.h"

namespace {

const char kChangeEmotionReadSignature[] =
    "virtual int Dispatcher_ChangeEmotion::read(PacketBuf&, MSG_BASE&)";
const char kChangeEmotionCheckErrorSignature[] =
    "int Dispatcher_ChangeEmotion::check_error(CUser*, MSG_BASE&)";
const char kChangeEmotionTemplateCheckStateSignature[] =
    "int IPacketDispatcher<MSG_TYPE, PARAM_TYPE, STATE>::check_state(CUser*, PacketBuf&) [with MSG_TYPE = MSG_CHANGE_EMOTION, PARAM_TYPE = stChangeEmotionParam, ch_state STATE = (ch_state)3u]";

// Raw dispatch_template @0x82532A4 does not call either MSG_CHANGE_EMOTION
// or MSG_BASE C2.  Aligned local storage retains that old stack behavior;
// read() is the only operation that fills the request's observed short.
typedef typename std::aligned_storage<sizeof(MSG_CHANGE_EMOTION),
                                      alignof(MSG_CHANGE_EMOTION)>::type
    UnconstructedChangeEmotion;

}  // namespace

// These declarations must precede the raw dispatch_template body.  Calling a
// member before its explicit-specialization declaration would make G++
// instantiate the generic state-3 member and create a second, non-original
// provider.
template <>
IPacketDispatcher<MSG_CHANGE_EMOTION,
                  stChangeEmotionParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher();

template <>
IPacketDispatcher<MSG_CHANGE_EMOTION,
                  stChangeEmotionParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher();

template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser* user,
                                                               PacketBuf& packet);

template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet);

template <>
void IPacketDispatcher<MSG_CHANGE_EMOTION,
                       stChangeEmotionParam,
                       static_cast<ch_state>(3)>::send(CUser* user,
                                                        ParamBase& param);

// IDA_EA: 0x8231468
// IDA_NAME: _ZN20stChangeEmotionParamC2Ev
// DEMANGLED: stChangeEmotionParam::stChangeEmotionParam(void)
stChangeEmotionParam::stChangeEmotionParam()
    : ParamBase(),
      emotion(0) {}

// IDA_EA: 0x82472AC
// IDA_NAME: _ZN20stChangeEmotionParamD2Ev
// DEMANGLED: stChangeEmotionParam::~stChangeEmotionParam()
// IDA_EA: 0x82472DC
// IDA_NAME: _ZN20stChangeEmotionParamD0Ev
// DEMANGLED: stChangeEmotionParam::~stChangeEmotionParam()
stChangeEmotionParam::~stChangeEmotionParam() {}

// IDA_EA: 0x82532A4
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::dispatch_template(
    CUser* user,
    PacketBuf& packet) {
    if (check_state(user, packet) > 0) {
        return 0;
    }

    int result = 0;
    UnconstructedChangeEmotion message_storage;
    MSG_CHANGE_EMOTION& message =
        *reinterpret_cast<MSG_CHANGE_EMOTION*>(&message_storage);
    result = read(packet, static_cast<MSG_BASE&>(message));
    if (result != 0) {
        return result;
    }

    stChangeEmotionParam param;
    result = process(user,
                     static_cast<MSG_BASE&>(message),
                     static_cast<ParamBase&>(param));
    if (result > 0) {
        return result;
    }
    if (result < 0) {
        return 0;
    }

    send(user, static_cast<ParamBase&>(param));
    return dispatch_sig(user, packet) > 0 ? 133 : 0;
}

// IDA_EA: 0x82533F4
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::dispatch_sig(CUser*,
                                                               PacketBuf&) {
    return 0;
}

// IDA_EA: 0x82533FE
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::check_state(CUser *,PacketBuf &)
template <>
int IPacketDispatcher<MSG_CHANGE_EMOTION,
                      stChangeEmotionParam,
                      static_cast<ch_state>(3)>::check_state(CUser* user,
                                                              PacketBuf& packet) {
    if (user->get_state() > 2) {
        return 0;
    }

    packet.set_index(1);
    short code;
    packet.get_short(code);
    LogManager::logFormat(1,
                           "PacketDispatcher.h",
                           kChangeEmotionTemplateCheckStateSignature,
                           148,
                           "return __LINE__: code(%d) size(%d)",
                           code,
                           packet.get_len());
    return 149;
}

// IDA_EA: 0x825C952
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825C982
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::~IPacketDispatcher()
template <>
IPacketDispatcher<MSG_CHANGE_EMOTION,
                  stChangeEmotionParam,
                  static_cast<ch_state>(3)>::~IPacketDispatcher() {}

// IDA_EA: 0x825D8A0
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::send(CUser *,ParamBase &)
template <>
void IPacketDispatcher<MSG_CHANGE_EMOTION,
                       stChangeEmotionParam,
                       static_cast<ch_state>(3)>::send(CUser*, ParamBase&) {}

// IDA_EA: 0x859890A
// IDA_NAME: _ZN17IPacketDispatcherI18MSG_CHANGE_EMOTION20stChangeEmotionParamL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_CHANGE_EMOTION,stChangeEmotionParam,(ch_state)3>::IPacketDispatcher(void)
template <>
IPacketDispatcher<MSG_CHANGE_EMOTION,
                  stChangeEmotionParam,
                  static_cast<ch_state>(3)>::IPacketDispatcher() {}

// IDA_EA: 0x8596D58
// IDA_NAME: _ZN24Dispatcher_ChangeEmotionC2Ev
// DEMANGLED: Dispatcher_ChangeEmotion::Dispatcher_ChangeEmotion(void)
Dispatcher_ChangeEmotion::Dispatcher_ChangeEmotion() {}

// IDA_EA: 0x8245724
// IDA_NAME: _ZN24Dispatcher_ChangeEmotionD2Ev
// DEMANGLED: Dispatcher_ChangeEmotion::~Dispatcher_ChangeEmotion()
// IDA_EA: 0x8245754
// IDA_NAME: _ZN24Dispatcher_ChangeEmotionD0Ev
// DEMANGLED: Dispatcher_ChangeEmotion::~Dispatcher_ChangeEmotion()
Dispatcher_ChangeEmotion::~Dispatcher_ChangeEmotion() {}

// IDA_EA: 0x81D1B08
// IDA_NAME: _ZN24Dispatcher_ChangeEmotion4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeEmotion::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ChangeEmotion::read(PacketBuf& packet, MSG_BASE& message) {
    MSG_CHANGE_EMOTION& request = static_cast<MSG_CHANGE_EMOTION&>(message);
    if (packet.get_short(request.emotion) == 1) {
        return 0;
    }
    return LineFunc(11837, kChangeEmotionReadSignature, 0, 0);
}

// IDA_EA: 0x81D1CEC
// IDA_NAME: _ZN24Dispatcher_ChangeEmotion11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ChangeEmotion::check_error(CUser *,MSG_BASE &)
int Dispatcher_ChangeEmotion::check_error(CUser* user, MSG_BASE&) {
    if (!user) {
        return LineFunc(11895, kChangeEmotionCheckErrorSignature, 0, 0);
    }
    if (user->getCurCharacR()) {
        return 0;
    }
    return LineFunc(11898, kChangeEmotionCheckErrorSignature, 0, 0);
}

// IDA_EA: 0x81D1EBE
// IDA_NAME: _ZN24Dispatcher_ChangeEmotion19isVaildEmotionValueEs
// DEMANGLED: Dispatcher_ChangeEmotion::isVaildEmotionValue(short)
int Dispatcher_ChangeEmotion::isVaildEmotionValue(int16_t emotion) {
    // The old body obtains the singleton once for begin and again for end;
    // retain those two observed calls instead of coalescing them into a
    // synthetic cached manager boundary.
    const stEmotionTypeTagsInfo* const begin_emotion_info =
        G_CDataManager()->get_emotion_type_tags_info();
    const std::vector<int>& begin_values = begin_emotion_info->emotion_types;
    const stEmotionTypeTagsInfo* const end_emotion_info =
        G_CDataManager()->get_emotion_type_tags_info();
    const std::vector<int>& end_values = end_emotion_info->emotion_types;
    for (std::vector<int>::const_iterator it = begin_values.begin();
         it != end_values.end();
         ++it) {
        if (*it == emotion) {
            return 1;
        }
    }
    return 0;
}

// IDA_EA: 0x81D1B5E
// IDA_NAME: _ZN24Dispatcher_ChangeEmotion7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ChangeEmotion::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ChangeEmotion::process(CUser* user,
                                      MSG_BASE& message,
                                      ParamBase& param) {
    const int check_result = check_error(user, message);
    if (check_result > 0) {
        return check_result;
    }
    if (check_result < 0) {
        return -1;
    }

    const MSG_CHANGE_EMOTION& request =
        static_cast<const MSG_CHANGE_EMOTION&>(message);
    stChangeEmotionParam& response =
        static_cast<stChangeEmotionParam&>(param);
    response.emotion = request.emotion;

    try {
        if (user->get_state() <= 2) {
            throw static_cast<ENUM_ERROR>(3);
        }
        if (isVaildEmotionValue(response.emotion) != 1) {
            throw static_cast<ENUM_ERROR>(1);
        }
    } catch (ENUM_ERROR error) {
        user->SendCmdErrorPacket(ENUM_CMDPACKET_CHANGE_EMOTION,
                                 static_cast<unsigned char>(error));
        return -1;
    }

    user->SetCurCharacEmotion(response.emotion);
    APSystem::CUserProc::ClearActionAndSendtoUser(
        user,
        static_cast<APSystem::_ActionGroupIndex>(3),
        0,
        ENUM_PACKETCLASS_NOTIPACKET);
    return 0;
}

// IDA_EA: 0x81D1D60
// IDA_NAME: _ZN24Dispatcher_ChangeEmotion4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ChangeEmotion::send(CUser *,ParamBase &)
void Dispatcher_ChangeEmotion::send(CUser* user, ParamBase& param) {
    if (!user) {
        return;
    }

    const stChangeEmotionParam& response =
        static_cast<const stChangeEmotionParam&>(param);
    PacketGuard packet;
    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_CMDPACKET,
                      ENUM_CMDPACKET_CHANGE_EMOTION);
    packet.put_byte(1);
    packet.put_short(response.emotion);
    packet.finalize(true);
    user->Send(packet);

    packet.clear();
    packet.put_header(ENUM_PACKETCLASS_NOTIPACKET, ENUM_NOTIPACKET_USERINFO);
    packet.put_byte(0);
    packet.put_short(1);
    user->make_basic_info(packet, 0);
    packet.finalize(true);
    G_GameWorld()->send_all(packet, user);
}
