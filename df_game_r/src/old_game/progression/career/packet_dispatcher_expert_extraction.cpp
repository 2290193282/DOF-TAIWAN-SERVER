#include "old_game/progression/career/packet_dispatcher_expert_extraction.h"
#include "infra/network/packet/PacketEnum.h"
#include "old_game/progression/career/expert_extraction_runtime.h"
#include "common/manager/global_data.h"
#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"
#include "old_game/player/core/cuser.h"
#include "infra/network/packet/packet_buf.h"
#include "infra/network/dispatcher/packet_dispatcher_msg_template.tpp"
#include "infra/network/packet/packet_guard.h"

// IDA_EA: 0x824EFE6
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EE17dispatch_templateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::dispatch_template(CUser *,PacketBuf &)
// IDA_EA: 0x824F142
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EE12dispatch_sigEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::dispatch_sig(CUser *,PacketBuf &)
// IDA_EA: 0x824F14C
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EE11check_stateEP5CUserR9PacketBuf
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::check_state(CUser *,PacketBuf &)
// IDA_EA: 0x825BF44
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EED2Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825BF74
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EED0Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::~IPacketDispatcher()
// IDA_EA: 0x825D678
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EE4sendEP5CUserR9ParamBase
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::send(CUser *,ParamBase &)
// IDA_EA: 0x8598456
// IDA_NAME: _ZN17IPacketDispatcherI21MSG_EXPERT_EXTRACTION18stExpertExtractionL8ch_state3EEC2Ev
// DEMANGLED: IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,(ch_state)3>::IPacketDispatcher(void)
template class IPacketDispatcher<MSG_EXPERT_EXTRACTION,
                                 stExpertExtraction,
                                 static_cast<ch_state>(3)>;

stExpertExtraction::stExpertExtraction():ParamBase(),result(0),target_slot(0),pad(),result_items(){}
stExpertExtraction::~stExpertExtraction(){}
// IDA_EA: 0x85959A8
// IDA_NAME: _ZN27Dispatcher_ExpertExtractionC2Ev
// DEMANGLED: Dispatcher_ExpertExtraction::Dispatcher_ExpertExtraction(void)
Dispatcher_ExpertExtraction::Dispatcher_ExpertExtraction():IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,static_cast<ch_state>(3)>(){}
// IDA_EA: 0x8244AF4
// IDA_NAME: _ZN27Dispatcher_ExpertExtractionD2Ev
// DEMANGLED: Dispatcher_ExpertExtraction::~Dispatcher_ExpertExtraction()
// IDA_EA: 0x8244B24
// IDA_NAME: _ZN27Dispatcher_ExpertExtractionD0Ev
// DEMANGLED: Dispatcher_ExpertExtraction::~Dispatcher_ExpertExtraction()
Dispatcher_ExpertExtraction::~Dispatcher_ExpertExtraction(){}
// IDA_EA: 0x81D9336
// IDA_NAME: _ZN27Dispatcher_ExpertExtraction4readER9PacketBufR8MSG_BASE
// DEMANGLED: Dispatcher_ExpertExtraction::read(PacketBuf &,MSG_BASE &)
int Dispatcher_ExpertExtraction::read(PacketBuf& p,MSG_BASE& b) { MSG_EXPERT_EXTRACTION& m=static_cast<MSG_EXPERT_EXTRACTION&>(b); if(p.get_byte(m.expert_job_type)!=1)return LineFunc(17836,"virtual int Dispatcher_ExpertExtraction::read(PacketBuf&, MSG_BASE&)",0,0); if(p.get_short(m.source_slot)!=1)return LineFunc(17838,"virtual int Dispatcher_ExpertExtraction::read(PacketBuf&, MSG_BASE&)",0,0); return p.get_short(m.target_slot)==1?0:LineFunc(17840,"virtual int Dispatcher_ExpertExtraction::read(PacketBuf&, MSG_BASE&)",0,0); }
// IDA_EA: 0x81D962E
// IDA_NAME: _ZN27Dispatcher_ExpertExtraction11check_errorEP5CUserR8MSG_BASE
// DEMANGLED: Dispatcher_ExpertExtraction::check_error(CUser *,MSG_BASE &)
int Dispatcher_ExpertExtraction::check_error(CUser* u,MSG_BASE& b) { if(!u||u->get_state()<=2)return -1; MSG_EXPERT_EXTRACTION& m=static_cast<MSG_EXPERT_EXTRACTION&>(b); unsigned t=expert_extraction::GetExtracterType(m.expert_job_type); if(t==3||!GlobalData::g_ExtractionMgr)return 1; expert_extraction::CExtraction* e=GlobalData::g_ExtractionMgr->get_extracter(t); return e?e->check_error(u,static_cast<char>(m.expert_job_type),m.source_slot,m.target_slot):1; }
// IDA_EA: 0x81D9412
// IDA_NAME: _ZN27Dispatcher_ExpertExtraction7processEP5CUserR8MSG_BASER9ParamBase
// DEMANGLED: Dispatcher_ExpertExtraction::process(CUser *,MSG_BASE &,ParamBase &)
int Dispatcher_ExpertExtraction::process(CUser* u,MSG_BASE& b,ParamBase& p) { stExpertExtraction& out=static_cast<stExpertExtraction&>(p); MSG_EXPERT_EXTRACTION& m=static_cast<MSG_EXPERT_EXTRACTION&>(b); out.result=check_error(u,b); if(out.result>0)return 0; if(out.result<0)return LineFunc(17855,"virtual int Dispatcher_ExpertExtraction::process(CUser*, MSG_BASE&, ParamBase&)",out.result,0); expert_extraction::CExtraction* e=GlobalData::g_ExtractionMgr->get_extracter(expert_extraction::GetExtracterType(m.expert_job_type)); if(!e)return LineFunc(17867,"virtual int Dispatcher_ExpertExtraction::process(CUser*, MSG_BASE&, ParamBase&)",0,u->get_acc_id()); std::vector<std::pair<int,int> > materials; if(!e->extraction(u,m.target_slot,materials)){out.result=1;return 0;} if(!e->inven_process(u,m.target_slot,materials,out.result_items)){out.result=17;return 0;} out.target_slot=m.target_slot; return 0; }
// IDA_EA: 0x81D9700
// IDA_NAME: _ZN27Dispatcher_ExpertExtraction4sendEP5CUserR9ParamBase
// DEMANGLED: Dispatcher_ExpertExtraction::send(CUser *,ParamBase &)
void Dispatcher_ExpertExtraction::send(CUser* u,ParamBase& p) { stExpertExtraction& out=static_cast<stExpertExtraction&>(p); if(out.result){u->SendCmdErrorPacket(ENUM_CMDPACKET_EXPERT_EXTRACTION,static_cast<unsigned char>(out.result));return;} PacketGuard q;q.clear();q.put_header(1,ENUM_CMDPACKET_EXPERT_EXTRACTION);q.put_byte(1);q.put_short(out.target_slot);int n=static_cast<int>(out.result_items.size()/3);q.put_byte(n);for(int i=0;i<n*3;i+=3){q.put_short(out.result_items[i]);q.put_int(out.result_items[i+1]);q.put_int(out.result_items[i+2]);}q.finalize(true);u->Send(q); }
