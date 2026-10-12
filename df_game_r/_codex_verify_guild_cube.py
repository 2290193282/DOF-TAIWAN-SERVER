from __future__ import print_function
import os
import subprocess
import sys
import tempfile

root, baseline = sys.argv[1:3]
def read(relative, old=False):
    with open(os.path.join(baseline if old else os.path.join(root,'src'),relative),'r') as f:
        return f.read()
def extract(text, marker, anchored=True):
    start=text.index('\n'+marker)+1 if anchored else text.index(marker)
    end=text.index('{',start)+1
    depth=1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}')
        end+=1
    return text[start:end]

cpp=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
struct LogManager {template<class...T>static void logFormat(T...){}};
namespace route_test {
#pragma pack(push,1)
struct PacketHeader {uint16_t type,size;char reserved[6];};
struct Packet_Monitor_Call_Guild_Members_Reply_Prefix {PacketHeader header;int target_user_uid;uint32_t charac,guild;char name[23];uint32_t cursor;uint16_t page;uint8_t member_count;};
struct Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix {PacketHeader header;int target_user_uid;uint32_t charac,guild;uint8_t member_count;};
#pragma pack(pop)
static_assert(sizeof(Packet_Monitor_Call_Guild_Members_Reply_Prefix)==52,"first prefix");
static_assert(sizeof(Packet_Monitor_Call_Guild_Members_Next_Reply_Prefix)==23,"next prefix");
const int kGuildMemberRecordSize=63,kGuildMembersReplyOpcode=0x401,kGuildMembersNextReplyOpcode=0x423,kGuildAllMembersReplyOpcode=0x424,kGuildAllMembersNextReplyOpcode=0x428;
const int kInterSignalGuildMembersReply=91,kInterSignalGuildMembersNextReply=198,kInterSignalGuildAllMembersReply=199,kInterSignalGuildAllMembersNextReply=200;
int signal_out=0,uid_out=0,size_out=0;
bool RouteGuildQueryReply(const char*,int bytes,int signal,int uid){signal_out=signal;uid_out=uid;size_out=bytes;return true;}
'''
bridge='infra/security/policy/hades_server_packet_bridge.cpp'
for name,old in [('Before',True),('After',False)]:
    code=read(bridge,old)
    cpp+='bool Route'+name+'(const char* packet,int packet_size){const PacketHeader& h=*reinterpret_cast<const PacketHeader*>(packet);const int opcode=h.type,declared_size=h.size;if(packet_size!=declared_size||declared_size<10)return false;\n'
    for marker in ['if (opcode == kGuildMembersReplyOpcode','if (opcode == kGuildMembersNextReplyOpcode']:
        cpp+=extract(code,marker,False)+'\n'
    cpp+='return false;}\n'
cpp+=r'''
void run(){for(int count=0;count<=255;count+=17){for(int page=0;page<2;++page){int prefix=page?23:52;std::vector<char> frame(prefix+63*count,0);PacketHeader* h=(PacketHeader*)&frame[0];h->type=page?0x428:0x424;h->size=frame.size();int uid=123456;memcpy(&frame[10],&uid,4);frame[prefix-1]=count;assert(!RouteBefore(&frame[0],frame.size()));assert(RouteAfter(&frame[0],frame.size()));assert(signal_out==(page?200:199)&&uid_out==uid&&size_out==(int)frame.size());assert(!RouteAfter(&frame[0],frame.size()-1));if(count){h->size--;assert(!RouteAfter(&frame[0],frame.size()-1));h->size++;}h->type=page?0x423:0x401;assert(RouteAfter(&frame[0],frame.size()));assert(signal_out==(page?198:91));}}
puts("PASS: missing all-member routes reproduced; first/next pages, 0..255 members, UID forwarding, short frames and existing online routes");}
}
namespace guild_test {
enum{ENUM_NOTIPACKET_USERINFO=2,ENUM_NOTIPACKET_GUILD_MEMBER_INFO=127,ENUM_NOTIPACKET_GUILD_INFO=70,ENUM_PACKETCLASS_NOTIPACKET=0};typedef int ENUM_SERVER_GROUP;
struct STGuildSkill{int skill_id;int8_t level;};
struct STGuildDBInfoOnly {char guild_name[23];uint32_t guild_master_charac_no;uint8_t guild_level,guild_flag;uint16_t guild_member_count;int raw_24;char raw_28;uint32_t guild_experience;short raw_42;uint8_t skill_count;STGuildSkill skills[16];uint8_t power_side,raw_9f,guild_agit_flag;uint32_t guild_fund;char raw_a4[25];STGuildDBInfoOnly(){memset(this,0,sizeof(*this));}};
struct PacketGuard{int id;std::vector<unsigned char> bytes;void put_header(int,int n){id=n;}void put_byte(int n){bytes.push_back(n);}void put_byte(int index,int n){bytes.at(index)=n;}void put_short(int n){put_byte(n);put_byte(n>>8);}void put_int(int n){for(int i=0;i<4;++i)put_byte(n>>(i*8));}void put_str(const char* p,int n){bytes.insert(bytes.end(),p,p+n);}int get_index(){return bytes.size();}void finalize(bool){}};
struct STGuildProxy{char guild_name[23];uint8_t guild_level,power_side,guild_agit_flag;STGuildProxy(){memset(this,0,sizeof(*this));}};
struct CGuildServerProxy{std::map<unsigned,STGuildProxy> values;int InsertGuildInfoProxy(unsigned k,STGuildProxy& v){return values.insert(std::make_pair(k,v)).second;}int ModifyGuildInfoProxy(unsigned k,STGuildProxy& v){if(!values.count(k))return 0;values[k]=v;return 1;}};
struct ProxyManager{CGuildServerProxy proxy;CGuildServerProxy* GetServerProxy(int){return &proxy;}}manager;
namespace GlobalData{ProxyManager* s_guild_proxy_mgr=&manager;}
struct CSkill{std::vector<std::vector<int> > level_values_;CSkill(){level_values_.resize(1);for(int i=1;i<=20;++i)level_values_[0].push_back(i);}};
struct Data{CSkill skill;CSkill* find_skill(int,int){return &skill;}}data;Data* G_CDataManager(){return &data;}
struct CUser {int state,job,grade,charac,key,skill_packets,basic_packets;bool exists;STGuildDBInfoOnly info;std::vector<int> sent;CUser():state(3),job(0),grade(0),charac(5),key(1),skill_packets(0),basic_packets(0),exists(true){}void* getCurCharacR(){return exists?this:0;}int get_state(){return state;}int get_charac_job(){return job;}int getCurCharacNo(){return charac;}int get_charac_no(int){return charac;}int get_charac_guildkey(){return key;}int GetServerGroup(){return 0;}int getGuildMemberGrade(){return grade;}void setGuildMemberGrade(uint8_t n){grade=n;}const char* getCurCharacName(){return "tester";}void send_skill_info(){++skill_packets;}void make_basic_info(PacketGuard&,int){++basic_packets;}void Send(PacketGuard& p){sent.push_back(p.id);}int get_guild_create_right(){return 0;}void SetGuildDBInfo(STGuildDBInfoOnly& v){info=v;}int IsGuildSkillLearn(int id){for(int i=0;i<info.skill_count&&i<16;++i)if(info.skills[i].skill_id==id)return info.skills[i].level;return 0;}int gainGuildSkillExp(int);};
#pragma pack(push,1)
struct ST_Guild_Mem_Info{uint8_t job,guild_member_grade;uint16_t level;char character_name[30],member_message[21];uint8_t channel_no,state,block_flag,master_grade;uint32_t logout_time;};
#pragma pack(pop)
struct FunctionGuildMember{static bool SyncCurrentMemberGrade(CUser*,uint32_t,uint32_t,int,const ST_Guild_Mem_Info*);static void RefreshGuildBenefits(CUser*);};
struct GuildInfoNoticeReplyPrefix{struct{struct{uint32_t charac_no;}recipient;uint32_t guild_key;}target;STGuildDBInfoOnly guild_info;char reserved;};
struct Inter_MonitorNoticeGuildInfo{int dispatch_sig(CUser*,char*,int);};
'''
members='old_game/social/guild/guild_member_packet_runtime.cpp'
cpp+=extract(read(members),'void FunctionGuildMember::RefreshGuildBenefits(')+'\n'
cpp+=extract(read(members),'bool FunctionGuildMember::SyncCurrentMemberGrade(')+'\n'
cpp+=extract(read('old_game/player/core/cuser_reward_exp_runtime.cpp'),'int CUser::gainGuildSkillExp(')+'\n'
cpp+=extract(read('old_game/social/guild/guild_monitor_query_dispatchers_a.cpp'),'int Inter_MonitorNoticeGuildInfo::dispatch_sig(')+'\n'
cpp+=r'''
void run(){CUser user;user.info.skill_count=2;user.info.skills[0]={200,20};user.info.skills[1]={201,20};assert(user.gainGuildSkillExp(1000)==0);ST_Guild_Mem_Info rows[2];memset(rows,0,sizeof(rows));strcpy(rows[0].character_name,"tester");rows[0].guild_member_grade=36;rows[0].master_grade=3;
assert(!FunctionGuildMember::SyncCurrentMemberGrade(&user,99,1,1,rows));assert(!FunctionGuildMember::SyncCurrentMemberGrade(&user,5,2,1,rows));assert(FunctionGuildMember::SyncCurrentMemberGrade(&user,5,1,1,rows));assert(user.grade==3&&user.gainGuildSkillExp(1000)==200&&user.skill_packets==1&&user.basic_packets==1);assert(user.sent[0]==127&&user.sent[1]==2);assert(!FunctionGuildMember::SyncCurrentMemberGrade(&user,5,1,1,rows));rows[1]=rows[0];rows[0].master_grade=1;assert(!FunctionGuildMember::SyncCurrentMemberGrade(&user,5,1,2,rows));rows[0].master_grade=0;assert(FunctionGuildMember::SyncCurrentMemberGrade(&user,5,1,1,rows));assert(user.gainGuildSkillExp(1000)==0);rows[0].character_name[29]='x';memset(rows[0].character_name,'x',29);assert(!FunctionGuildMember::SyncCurrentMemberGrade(&user,5,1,1,rows));
user.grade=3;std::vector<char> payload(sizeof(GuildInfoNoticeReplyPrefix)+32,0);GuildInfoNoticeReplyPrefix* p=(GuildInfoNoticeReplyPrefix*)&payload[0];new (&p->guild_info)STGuildDBInfoOnly();p->target.recipient.charac_no=5;p->target.guild_key=1;strcpy(p->guild_info.guild_name,"guild");p->guild_info.guild_level=17;p->guild_info.guild_agit_flag=1;p->guild_info.skill_count=2;p->guild_info.skills[0]={200,8};p->guild_info.skills[1]={201,8};Inter_MonitorNoticeGuildInfo handler;int old_refresh=user.basic_packets;handler.dispatch_sig(&user,&payload[0],payload.size());assert(user.IsGuildSkillLearn(201)==8&&user.gainGuildSkillExp(1000)==80);assert(user.basic_packets==old_refresh+1&&manager.proxy.values[1].guild_level==17&&manager.proxy.values[1].guild_agit_flag==1);handler.dispatch_sig(&user,&payload[0],payload.size());assert(user.basic_packets==old_refresh+1);p->guild_info.skill_count=17;handler.dispatch_sig(&user,&payload[0],payload.size());assert(user.IsGuildSkillLearn(201)==8);p->guild_info.skill_count=0;handler.dispatch_sig(&user,&payload[0],payload.size());assert(user.IsGuildSkillLearn(201)==0&&user.basic_packets==old_refresh+2);
puts("PASS: authoritative self grade enables actual 20% EXP calculation; trainee/wrong recipient/wrong guild/ambiguous roster stay restricted; native skill+USERINFO refresh, cache seed/update, unchanged snapshot idempotence and invalid skill count");}
}
namespace cube_test {
struct Inven_Item{int item_id_;Inven_Item():item_id_(0){}void reset(){item_id_=0;}};
struct CItem;struct Vtable{bool(*is_random_option)(const CItem*);int(*make_item)(CItem*,Inven_Item&);};
struct CItem{Vtable* vtable_;int id,group,level,kind,rate;int get_index()const{return id;}int get_gen_rate()const{return rate;}int get_grade()const{return 1;}int getItemGroupName()const{return group;}int getUsableLevel()const{return level;}};
struct STEquipmentScript{struct{int value_24;}item;CItem fixture;};
bool RandomOption(const CItem*){return false;}int MakeItem(CItem* p,Inven_Item& out){out.item_id_=p->id;return 1;}Vtable vt={RandomOption,MakeItem};
struct CEquipItem:CItem{CEquipItem(){vtable_=&vt;}void set_item(STEquipmentScript& s){static_cast<CItem&>(*this)=s.fixture;vtable_=&vt;}bool verify(){return true;}int GetUsableEquipmentType()const{return kind;}};
struct Data{std::map<int,CItem*> items;CItem* find_item(int id){return items.count(id)?items[id]:0;}}data;Data* G_CDataManager(){return &data;}
struct CMTRand{unsigned next;CMTRand():next(0){}unsigned randInt(){return next++*7919u;}};
namespace ExtreamDungeon{
typedef int ENUM_UNUSABLE_EQUIPMENT_TYPE;typedef int ENUM_INPUTITEMS_RARITY;typedef int ENUM_OUTPUTITEMS_RARITY;typedef int eBindSphereType;typedef std::pair<int,int> OUTPUT_ITEM_STR;
bool BindTable_Predicate(const OUTPUT_ITEM_STR& a,const OUTPUT_ITEM_STR& b){return a.first<b.first;}
struct forceRule_ResultItem{bool force;int item_index;forceRule_ResultItem(bool f=false,int n=0):force(f),item_index(n){}};
struct CCompound_ExtreamDun_Item{struct{std::vector<OUTPUT_ITEM_STR> output_items[59][4],secondary_output_items[4];std::map<int,forceRule_ResultItem> force_rule_result_items;uint32_t rarity_decision_thresholds[3][4];}script_data_;struct Bonus{std::vector<unsigned> level_boundaries,decision_threshold_by_input_rarity[3];}level_bonus_script_data_[2];CMTRand rng;CMTRand* random_;CCompound_ExtreamDun_Item():random_(&rng){for(int s=0;s<2;++s){level_bonus_script_data_[s].level_boundaries={0,5,10};for(int r=0;r<3;++r)level_bonus_script_data_[s].decision_threshold_by_input_rarity[r]={0,0,1000000};}for(int i=0;i<3;++i)for(int j=0;j<4;++j)script_data_.rarity_decision_thresholds[i][j]=1000000;}
int _convertOutputItem(int)const;int _convertInput_To_Output(int)const;int _convertInputItem(int)const;int _determine_rarity(unsigned,int)const;unsigned _determine_level_bonus_index(unsigned,int,int)const;std::pair<unsigned,unsigned> _determine_level_bonus(int,unsigned,int,int)const;int _FindItemRelateCreationRate(std::vector<OUTPUT_ITEM_STR>::const_iterator,std::vector<OUTPUT_ITEM_STR>::const_iterator)const;int _FindSelectItem(unsigned,int,const std::pair<unsigned,unsigned>&)const;int _FindRandomItem(int,const std::pair<unsigned,unsigned>&)const;int _FindItemAll_BindSphere(const CEquipItem* const[2],char,int,const std::pair<unsigned,unsigned>&)const;int _exception_Rare(const CEquipItem* const[2],char,int,unsigned)const;int makeCompoundItem(const CEquipItem* const[2],char,Inven_Item&,int);bool _isPossibleResultItem(CEquipItem&)const;bool _AddItem(CEquipItem&);bool check_N_addItem(CEquipItem&);bool after_addItem();unsigned _get_base_level(const CEquipItem* const[2])const;
};
'''
cube=read('old_game/dungeon/instance/compound_extream_dun_item_runtime.cpp')
markers=['int CCompound_ExtreamDun_Item::_convertOutputItem(','int CCompound_ExtreamDun_Item::_convertInput_To_Output(','int CCompound_ExtreamDun_Item::_convertInputItem(','int CCompound_ExtreamDun_Item::_determine_rarity(','unsigned int CCompound_ExtreamDun_Item::_determine_level_bonus_index(','int CCompound_ExtreamDun_Item::_FindItemRelateCreationRate(','int CCompound_ExtreamDun_Item::_FindSelectItem(','int CCompound_ExtreamDun_Item::_FindRandomItem(','int CCompound_ExtreamDun_Item::_FindItemAll_BindSphere(','int CCompound_ExtreamDun_Item::_exception_Rare(','int CCompound_ExtreamDun_Item::makeCompoundItem(','bool CCompound_ExtreamDun_Item::_isPossibleResultItem(','bool CCompound_ExtreamDun_Item::_AddItem(','bool CCompound_ExtreamDun_Item::check_N_addItem(','bool CCompound_ExtreamDun_Item::after_addItem(','unsigned int CCompound_ExtreamDun_Item::_get_base_level(']
for marker in markers:cpp+=extract(cube,marker)+'\n'
cpp+='std::pair<unsigned int,unsigned int>\n'+extract(cube,'CCompound_ExtreamDun_Item::_determine_level_bonus(\n    ENUM_INPUTITEMS_RARITY')+'\n}\n'
cpp+=r'''
namespace GlobalData{ExtreamDungeon::CCompound_ExtreamDun_Item* s_pCompoundBindCube;}
typedef std::map<int,std::string> STScriptFileList;STScriptFileList g_stEquipmentScriptFileList;std::map<int,CItem> fixtures;
int ImportEquipmentScript(STEquipmentScript* s,const char*,bool,int,int id){s->fixture=fixtures[id];s->item.value_24=0;return 1;}
namespace sync_script{struct CSyncScript{int insert_equiptment_item_to_db(STEquipmentScript*,int,int,int){return 1;}};}
struct CItemList{void insert_item(int,int id,CItem* p){data.items[id]=p;}};
const char* kLoadItemFilesPrettyFunction="load";
'''
loader='old_data/PVF/data/item_list_pvf_import.cpp'
cpp+=extract(read(loader,True),'bool ImportEquipmentPVF(').replace('ImportEquipmentPVF(', 'ImportEquipmentPVFBefore(',1)+'\n'
cpp+=extract(read(loader),'bool ImportEquipmentPVF(')+'\n'
cpp+=r'''
void run(){ExtreamDungeon::CCompound_ExtreamDun_Item empty,filled;CItemList list;sync_script::CSyncScript sync;int ids[]={28279,28309,999001,999002,999003};int levels[]={55,65,65,70,80};for(int i=0;i<5;++i){CItem p;p.id=ids[i];p.group=5;p.level=levels[i];p.kind=3;p.rate=10;p.vtable_=&vt;fixtures[p.id]=p;g_stEquipmentScriptFileList[p.id]="fixture";}GlobalData::s_pCompoundBindCube=&empty;assert(ImportEquipmentPVFBefore(&list,&sync));empty.after_addItem();const CEquipItem* inputs[]={static_cast<CEquipItem*>(data.items[28279]),static_cast<CEquipItem*>(data.items[28309])};Inven_Item output;assert(empty.makeCompoundItem(inputs,0,output,0)==0&&output.item_id_==0);
GlobalData::s_pCompoundBindCube=&filled;assert(ImportEquipmentPVF(&list,&sync));filled.after_addItem();assert(filled.script_data_.output_items[5][3].size()==4);inputs[0]=static_cast<CEquipItem*>(data.items[28279]);inputs[1]=static_cast<CEquipItem*>(data.items[28309]);for(int i=0;i<1000;++i){assert(filled.makeCompoundItem(inputs,0,output,0)==1);assert(output.item_id_==999002);}
puts("PASS: actual loader omission reproduces empty synthesis table; restored registration plus production generation succeeds for level55+65 unique fixture; level80 excluded and 1000 rolls preserve the result range");}
}
int main(){route_test::run();guild_test::run();cube_test::run();}
'''
directory=tempfile.mkdtemp(prefix='codex_guild_cube_')
path,binary=os.path.join(directory,'test.cpp'),os.path.join(directory,'test')
with open(path,'w') as stream:stream.write(cpp)
subprocess.check_call(['/opt/gcc-4.9/bin/g++','-m32','-std=gnu++11','-O0',path,'-o',binary])
subprocess.check_call([binary])
print('HARNESS='+path)
