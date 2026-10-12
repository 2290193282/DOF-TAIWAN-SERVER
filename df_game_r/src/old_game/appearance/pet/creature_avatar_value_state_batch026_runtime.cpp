#include "old_game/appearance/pet/creature.h"

#include "common/manager/global_data.h"
#include "common/manager/system_time.h"

#include <string.h>

// IDA_EA: 0x833D636
// IDA_NAME: _ZN13user_creature14COverSkillTimeC2Ev
// DEMANGLED: user_creature::COverSkillTime::COverSkillTime(void)
user_creature::COverSkillTime::COverSkillTime() : time_(0) {}

// IDA_EA: 0x833D644
// IDA_NAME: _ZN13user_creature14COverSkillTime7SetTimeEv
// DEMANGLED: user_creature::COverSkillTime::SetTime(void)
int user_creature::COverSkillTime::SetTime() {
    time_ = GlobalData::system_time()->getCurSec();
    return time_;
}

// IDA_EA: 0x833D65E
// IDA_NAME: _ZN13user_creature14COverSkillTime9checkTimeEv
// DEMANGLED: user_creature::COverSkillTime::checkTime(void)
bool user_creature::COverSkillTime::checkTime() {
    const float elapsed = static_cast<float>(
        GlobalData::system_time()->getCurSec() - time_);
    return elapsed > 0.0f && elapsed < 20.0f;
}

// IDA_EA: 0x833EF04
// IDA_NAME: _ZN13user_creature14COverSkillTime7GetTimeEv
// DEMANGLED: user_creature::COverSkillTime::GetTime(void)
user_creature::COverSkillTime* user_creature::COverSkillTime::GetTime() {
    return this;
}

// IDA_EA: 0x833EF98
// IDA_NAME: _ZN13user_creature13CCreatureItem19GetCreatureItemTypeEv
// DEMANGLED: user_creature::CCreatureItem::GetCreatureItemType(void)
int user_creature::CCreatureItem::GetCreatureItemType() {
    return creature_item_type_;
}

// IDA_EA: 0x833EFD6
// IDA_NAME: _ZN13user_creature13CCreatureItem11GetNoChargeEv
// DEMANGLED: user_creature::CCreatureItem::GetNoCharge(void)
int user_creature::CCreatureItem::GetNoCharge() {
    return no_charge_;
}

// IDA_EA: 0x833EFF0
// IDA_NAME: _ZN13user_creature13CCreatureItem7GetStatEv
// DEMANGLED: user_creature::CCreatureItem::GetStat(void)
int user_creature::CCreatureItem::GetStat() {
    return stat_;
}

// IDA_EA: 0x833EFFC
// IDA_NAME: _ZN13user_creature13CCreatureItem13SetExpireTimeEi
// DEMANGLED: user_creature::CCreatureItem::SetExpireTime(int)
user_creature::CCreatureItem*
user_creature::CCreatureItem::SetExpireTime(int expire_time) {
    expire_time_ = expire_time;
    return this;
}

// IDA_EA: 0x83384EE
// IDA_NAME: _ZN13user_creature9CCreature10GetStomachEv
// DEMANGLED: user_creature::CCreature::GetStomach(void)
int user_creature::CCreature::GetStomach() {
    return static_cast<int>(
        stomach_.GetStomachValue(state_, item_grow_state_ != 0u));
}

// IDA_EA: 0x833F070
// IDA_NAME: _ZN13user_creature9CCreature10SetStomachEi
// DEMANGLED: user_creature::CCreature::SetStomach(int)
user_creature::CStomach* user_creature::CCreature::SetStomach(int value) {
    return stomach_.SetStomachValue(static_cast<float>(value));
}

// IDA_EA: 0x833F08E
// IDA_NAME: _ZN13user_creature9CCreature6GetExpEv
// DEMANGLED: user_creature::CCreature::GetExp(void)
int user_creature::CCreature::GetExp() {
    return experience_;
}

// IDA_EA: 0x833F0A6
// IDA_NAME: _ZN13user_creature9CCreature10IsNoChargeEv
// DEMANGLED: user_creature::CCreature::IsNoCharge(void)
int user_creature::CCreature::IsNoCharge() {
    return no_charge_;
}

// IDA_EA: 0x833F0B2
// IDA_NAME: _ZN13user_creature9CCreature11SetNoChargeEb
// DEMANGLED: user_creature::CCreature::SetNoCharge(bool)
user_creature::CCreature*
user_creature::CCreature::SetNoCharge(bool no_charge) {
    no_charge_ = no_charge;
    return this;
}

// IDA_EA: 0x833F0DA
// IDA_NAME: _ZN13user_creature9CCreature13GetCreatureIdEv
// DEMANGLED: user_creature::CCreature::GetCreatureId(void)
int user_creature::CCreature::GetCreatureId() {
    return creature_id_;
}

// IDA_EA: 0x833F0E6
// IDA_NAME: _ZN13user_creature9CCreature16GetOverSkillTimeEv
// DEMANGLED: user_creature::CCreature::GetOverSkillTime(void)
user_creature::COverSkillTime*
user_creature::CCreature::GetOverSkillTime() {
    return &over_skill_time_;
}

// IDA_EA: 0x833F0F2
// IDA_NAME: _ZN13user_creature9CCreature16IsCreatureScriptEv
// DEMANGLED: user_creature::CCreature::IsCreatureScript(void)
bool user_creature::CCreature::IsCreatureScript() {
    return creature_script_ != 0;
}

// IDA_EA: 0x822D202
// IDA_NAME: _ZNK8WongWork11Avatar_Item14GetIPGAgencyNoEv
// DEMANGLED: WongWork::Avatar_Item::GetIPGAgencyNo(void)const
const char* WongWork::Avatar_Item::GetIPGAgencyNo() const {
    return ipg_agency_no_;
}

// IDA_EA: 0x82FA228
// IDA_NAME: _ZNK8WongWork11Avatar_Item13GetRegistTimeEv
// DEMANGLED: WongWork::Avatar_Item::GetRegistTime(void)const
int WongWork::Avatar_Item::GetRegistTime() const {
    return regist_time_;
}

// IDA_EA: 0x82FA234
// IDA_NAME: _ZN8WongWork11Avatar_Item7SetDataEiiPKc
// DEMANGLED: WongWork::Avatar_Item::SetData(int,int,char const*)
char* WongWork::Avatar_Item::SetData(int expire_date,
                                     int regist_time,
                                     const char* agency_no) {
    expire_date_ = expire_date;
    regist_time_ = regist_time;
    return strncpy(ipg_agency_no_, agency_no, 0x18u);
}

// IDA_EA: 0x82FA26A
// IDA_NAME: _ZN8WongWork11Avatar_Item19setEmblemSocketDataERK20stAvatarEmblemInfo_t
// DEMANGLED: WongWork::Avatar_Item::setEmblemSocketData(stAvatarEmblemInfo_t const&)
WongWork::Avatar_Item* WongWork::Avatar_Item::setEmblemSocketData(
    const stAvatarEmblemInfo_t& emblem) {
    memcpy(&jewel_socket_data_, &emblem, sizeof(jewel_socket_data_));
    return this;
}

// IDA_EA: 0x82FA2A6
// IDA_NAME: _ZN8WongWork11Avatar_Item18updateEmblemSocketEjj
// DEMANGLED: WongWork::Avatar_Item::updateEmblemSocket(unsigned int,unsigned int)
int WongWork::Avatar_Item::updateEmblemSocket(unsigned int socket,
                                               unsigned int item_id) {
    stAvatarEmblemSocket_t& record = jewel_socket_data_.sockets[socket];
    if (record.socket_type == 0u) {
        return 0;
    }
    record.emblem_item_index = item_id;
    return 1;
}

// IDA_EA: 0x82FA2FA
// IDA_NAME: _ZN8WongWork11Avatar_Item8setColorEss
// DEMANGLED: WongWork::Avatar_Item::setColor(short,short)
WongWork::Avatar_Item* WongWork::Avatar_Item::setColor(short first,
                                                       short second) {
    expansion_info_.avatar_expansion[0] = static_cast<uint16_t>(first);
    expansion_info_.avatar_expansion[1] = static_cast<uint16_t>(second);
    return this;
}

// IDA_EA: 0x82FA326
// IDA_NAME: _ZN8WongWork11Avatar_Item16setExpansionInfoERK23stAvatarExpansionInfo_t
// DEMANGLED: WongWork::Avatar_Item::setExpansionInfo(stAvatarExpansionInfo_t const&)
WongWork::Avatar_Item* WongWork::Avatar_Item::setExpansionInfo(
    const stAvatarExpansionInfo_t& expansion) {
    memcpy(&expansion_info_, &expansion, sizeof(expansion_info_));
    return this;
}

// IDA_EA: 0x82FA342
// IDA_NAME: _ZNK8WongWork11Avatar_Item19getExnansionInfoRefEv
// DEMANGLED: WongWork::Avatar_Item::getExnansionInfoRef(void)const
const stAvatarExpansionInfo_t*
WongWork::Avatar_Item::getExnansionInfoRef() const {
    return &expansion_info_;
}

// IDA_EA: 0x82FA34E
// IDA_NAME: _ZN8WongWork11Avatar_ItemC2Ev
// DEMANGLED: WongWork::Avatar_Item::Avatar_Item(void)
WongWork::Avatar_Item::Avatar_Item() {}

// IDA_EA: 0x82FA354
// IDA_NAME: _ZN8WongWork11Avatar_ItemD2Ev
// DEMANGLED: WongWork::Avatar_Item::~Avatar_Item()
WongWork::Avatar_Item::~Avatar_Item() {}

// IDA_EA: 0x82FA35A
// IDA_NAME: _ZN8WongWork11Avatar_Item4initEv
// DEMANGLED: WongWork::Avatar_Item::init(void)
void* WongWork::Avatar_Item::init() {
    expire_date_ = 0;
    regist_time_ = 0;
    memset(ipg_agency_no_, 0, sizeof(ipg_agency_no_));
    return memset(&expansion_info_, 0, sizeof(expansion_info_));
}
