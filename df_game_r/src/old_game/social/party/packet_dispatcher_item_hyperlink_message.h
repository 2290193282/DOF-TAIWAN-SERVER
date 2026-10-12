#ifndef OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_ITEM_HYPERLINK_MESSAGE_H_
#define OLDSRV_OLD_GAME_GROUP_PACKET_DISPATCHER_ITEM_HYPERLINK_MESSAGE_H_
#include <stdint.h>
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
class CUser; class PacketBuf;
#pragma pack(push,1)
struct MSG_ITEM_HYPER_LINK_MESSAGE : MSG_BASE { uint8_t chat_type; uint16_t item_space; uint32_t item_position; int32_t message_length; char message[256]; int32_t target_name_length; char target_name[30]; uint8_t sender_marker; uint8_t hyperlink_count; uint8_t hyperlinks[3][104]; };
#pragma pack(pop)
class stItemHyperLinkMessage : public ParamBase { public: stItemHyperLinkMessage():ParamBase(),error(0){} virtual ~stItemHyperLinkMessage(){} int error; };
class Dispatcher_ItemHyperLinkMessage : public IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,static_cast<ch_state>(3)> { public: Dispatcher_ItemHyperLinkMessage(); virtual ~Dispatcher_ItemHyperLinkMessage(); virtual int read(PacketBuf&,MSG_BASE&); virtual int process(CUser*,MSG_BASE&,ParamBase&); virtual void send(CUser*,ParamBase&); int check_error(CUser*,MSG_BASE&); };
#if defined(__i386__)
static_assert(sizeof(MSG_ITEM_HYPER_LINK_MESSAGE)==0x274u,"CMD425 old wire ABI");
#endif
#endif
