#include "old_game/social/party/packet_dispatcher_item_hyperlink_message.h"
#include <cstring>
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "old_data/PVF/data/data_manager.h"
#include "old_game/economy/inventory/inventory.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/log/format/trace.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/packet/PacketEnum.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"

// IDA_EA: 0x824EDEC
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824EF48
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824EF52
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825BEF6
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825BF26
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D672
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598D4E
// IDA_NAME: _ZN17IPacketDispatcherI27MSG_ITEM_HYPER_LINK_MESSAGE22stItemHyperLinkMessageL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,
                                 stItemHyperLinkMessage,
                                 static_cast<ch_state>(3)>;

// IDA_EA: 0x85978A2
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessageC2Ev
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::Dispatcher_ItemHyperLinkMessage(void)
Dispatcher_ItemHyperLinkMessage::Dispatcher_ItemHyperLinkMessage():IPacketDispatcher<MSG_ITEM_HYPER_LINK_MESSAGE,stItemHyperLinkMessage,static_cast<ch_state>(3)>(){}
// IDA_EA: 0x8244AA6
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessageD2Ev
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::~Dispatcher_ItemHyperLinkMessage()
// IDA_EA: 0x8244AD6
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessageD0Ev
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::~Dispatcher_ItemHyperLinkMessage()
Dispatcher_ItemHyperLinkMessage::~Dispatcher_ItemHyperLinkMessage(){}
// IDA_EA: 0x81D98C4
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessage4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ItemHyperLinkMessage::read(PacketBuf& p,MSG_BASE& b){MSG_ITEM_HYPER_LINK_MESSAGE&m=static_cast<MSG_ITEM_HYPER_LINK_MESSAGE&>(b);if(p.get_byte(m.chat_type)!=1)return LineFunc(18277,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);if(p.get_short(m.item_space)!=1)return LineFunc(18280,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);if(p.get_int(m.item_position)!=1)return LineFunc(18283,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);if(p.get_int(m.message_length)!=1||p.get_str(m.message,256,m.message_length)!=1)return LineFunc(18289,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);if(m.chat_type==1||m.chat_type==38){if(p.get_int(m.target_name_length)!=1||p.get_str(m.target_name,30,m.target_name_length)!=1||p.get_byte(m.sender_marker)!=1)return LineFunc(18309,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);}if(p.get_byte(m.hyperlink_count)!=1||!m.hyperlink_count||m.hyperlink_count>3)return LineFunc(18325,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);for(int i=0;i<m.hyperlink_count;++i)if(p.get_binary(reinterpret_cast<char*>(m.hyperlinks[i]),104)!=1)return LineFunc(18330,"virtual int Dispatcher_ItemHyperLinkMessage::read(PacketBuf&, MSG_BASE&)",0,0);return 0;}
// IDA_EA: 0x81DA514
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessage11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::check_error(CUser *,MSG_BASE &)
int Dispatcher_ItemHyperLinkMessage::check_error(CUser*u,MSG_BASE&b){if(!u||u->get_state()<=2)return -1;uint8_t c=static_cast<MSG_ITEM_HYPER_LINK_MESSAGE&>(b).chat_type;return c==36||c==37||c==33||c==7?-1:0;}
// IDA_EA: 0x81D9C4A
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessage7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ItemHyperLinkMessage::process(CUser*u,MSG_BASE&b,ParamBase&p){MSG_ITEM_HYPER_LINK_MESSAGE&m=static_cast<MSG_ITEM_HYPER_LINK_MESSAGE&>(b);stItemHyperLinkMessage&out=static_cast<stItemHyperLinkMessage&>(p);out.error=check_error(u,b);if(out.error||m.message_length<0||m.message_length>256)return out.error?0:LineFunc(18346,"virtual int Dispatcher_ItemHyperLinkMessage::process(CUser*, MSG_BASE&, ParamBase&)",-1,0);G_CDataManager()->rewriteUsableEmoticon(*u,m.message);if(u->IsCurCharacGhost()&&m.chat_type!=2&&m.chat_type!=6&&m.chat_type!=8)return 0;G_GameWorld()->send_chat_msg_hyper_link(u,m,static_cast<ENUM_NOTIPACKET>(370));return 0;}
// IDA_EA: 0x81DA584
// IDA_NAME: _ZN31Dispatcher_ItemHyperLinkMessage4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ItemHyperLinkMessage::send(CUser *,ParamBase &)
void Dispatcher_ItemHyperLinkMessage::send(CUser*u,ParamBase&p){int e=static_cast<stItemHyperLinkMessage&>(p).error;if(e)u->SendCmdErrorPacket(ENUM_CMDPACKET_ITEM_HYPERLINK_MESSAGE,static_cast<unsigned char>(e));else u->SendCmdOkPacket(ENUM_CMDPACKET_ITEM_HYPERLINK_MESSAGE);}
