#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_EXTRACTION_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_PACKET_DISPATCHER_EXPERT_EXTRACTION_H_
#include <stdint.h>
#include <vector>
#include "infra/network/dispatcher/packet_dispatcher_msg_base.h"
class CUser; class PacketBuf;
#pragma pack(push,1)
struct MSG_EXPERT_EXTRACTION : MSG_BASE { uint8_t expert_job_type; int16_t source_slot; int16_t target_slot; };
#pragma pack(pop)
class stExpertExtraction : public ParamBase { public: stExpertExtraction(); virtual ~stExpertExtraction(); int result; int16_t target_slot; uint8_t pad[2]; std::vector<int> result_items; };
class Dispatcher_ExpertExtraction : public IPacketDispatcher<MSG_EXPERT_EXTRACTION,stExpertExtraction,static_cast<ch_state>(3)> {
public: Dispatcher_ExpertExtraction(); virtual ~Dispatcher_ExpertExtraction();
 virtual int read(PacketBuf&,MSG_BASE&); virtual int process(CUser*,MSG_BASE&,ParamBase&); virtual void send(CUser*,ParamBase&); int check_error(CUser*,MSG_BASE&);
};
#if defined(__i386__)
static_assert(sizeof(MSG_EXPERT_EXTRACTION)==0x12u,"CMD416 old request ABI");
// IDA_EA: 0x82317FE: ParamBase occupies +0x00..+0x03; the constructor
// initializes result/+0x04, target_slot/+0x08 and vector<int>/+0x0c.
static_assert(sizeof(stExpertExtraction)==0x18u,"CMD416 old result ABI");
#endif
#endif
