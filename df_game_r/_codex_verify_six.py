from __future__ import print_function
import os
import re
import subprocess
import sys
import tempfile


def read(path):
    with open(path, 'r') as stream:
        return stream.read()


def block(text, marker, body=False):
    # Function names also occur inside diagnostic string literals.  Extract
    # only a definition at the start of a source line, not those strings.
    if not body:
        start = text.index('\n' + marker) + 1
    else:
        start = text.index(marker)
    brace = text.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[brace+1:end-1] if body else text[start:end]


root, baseline, pvf = sys.argv[1:4]
def source(relative, old=False):
    return read(os.path.join(baseline if old else os.path.join(root, 'src'), relative))

mission='old_game/dungeon/mission/dungeon_mission.cpp'
equip='old_data/PVF/equipment/equipment_script.cpp'
combo='old_data/Login/packet_dispatch/combo_skill_dispatcher.cpp'
tutorial='old_game/dungeon/instance/dungeon_pvp_dispatchers.cpp'
cards='old_game/social/party/cparty_clear_reward_runtime.cpp'
cube='old_game/dungeon/instance/packet_dispatcher_compound_extream_item.cpp'

cpp=r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#pragma pack(push,1)
struct CS_Header {uint32_t length;unsigned char tail[9];};
#pragma pack(pop)
struct PacketBuf {enum{PACKETBUF_ERROR_NONE=0};CS_Header header;int cursor,error;std::vector<unsigned char> bytes;PacketBuf(int n=13,int len=13):cursor(n),error(0){header.length=len;}int get_index(){return cursor;}int getLastError(){return error;}void* get_packet(int){return &header;}int get_byte(unsigned char& b){if(cursor-13>=(int)bytes.size()){error=1;return 0;}b=bytes[cursor++-13];return 1;}int get_short(unsigned short& s){unsigned char a,b;if(!get_byte(a)||!get_byte(b))return 0;s=a|(b<<8);return 1;} };
'''
cpp+=block(source(mission,True),'bool PacketBodyConsumed(').replace('PacketBodyConsumed(', 'PacketBodyConsumedBefore(',1)+'\n'
cpp+=block(source(mission),'bool PacketBodyConsumed(')+'\n'
cpp+=r'''
std::vector<std::string> tokens;size_t token_pos;
int ScanType(std::string& s,bool){while(token_pos<tokens.size()){s=tokens[token_pos++];if(!s.empty()&&s[0]=='[')return 1;}return 0;}
int ScanInt(bool* ok){if(token_pos>=tokens.size()||tokens[token_pos].empty()||tokens[token_pos][0]=='['){*ok=false;return 0;}*ok=true;return atoi(tokens[token_pos++].c_str());}
int ScanStr(std::string* s){if(token_pos>=tokens.size())return 0;*s=tokens[token_pos++];return 1;}
struct STEquipmentScript {struct{int value_10e;}item;uint8_t equipment_growth_type,equipment_growth_stage;std::string string_7f0;std::vector<std::pair<int,int> > pairs_7d8,pairs_7e4;STEquipmentScript():equipment_growth_type(0),equipment_growth_stage(0){item.value_10e=0;}};
#define EQUIP_FAIL() return 0
'''
for name,old in [('Before',True),('After',False)]:
    cpp+='int ParseGrowth'+name+'(STEquipmentScript* script){std::string tag;bool has_buffered_tag=false;'+block(source(equip,old),'if (tag == "[emancipate]")',True)+'return 1;}\n'
cpp+=r'''
namespace oldsrv {namespace config {enum{kCharacterJobDemonicSwordman=9,kCharacterJobCreatorMage=10,kCharacterLevelMaximum=85};}}
struct LogManager{template<class...T>static void logFormat(T...){}};
struct CDungeon {bool defense,risk;CDungeon():defense(false),risk(false){}bool isDefenseDungeon(){return defense;}bool isRiskDungeon()const{return risk;}};
struct CEquipItem{};
struct Inven_Item{int item_id_;bool equip;Inven_Item():item_id_(0),equip(false){}void reset(){item_id_=0;equip=false;}bool isEmpty()const{return item_id_==0;}bool isEquipableItemType()const{return equip;}};
struct Inventory {std::map<int,Inven_Item> slots;Inven_Item GetInvenSlot(int,int slot)const{std::map<int,Inven_Item>::const_iterator it=slots.find(slot);return it==slots.end()?Inven_Item():it->second;}};
struct CParty;
typedef int ENUM_SKILL_TREE_KIND;
struct MSG_BASE{};struct ParamBase{};struct stSelectDungeonParam:ParamBase{};
struct MSG_SELECT_DUNGEON:MSG_BASE{unsigned short dungeon_index;char dungeon_difficulty;unsigned char dungeon_mode,request_flag;};
struct MSG_COMBO_SKILL:MSG_BASE{uint8_t skill_tree_index;std::map<short,std::vector<int> > combo_skills;};
struct stComboSkill:ParamBase{int result;};
struct SkillSlot{int edits,verifies;SkillSlot():edits(0),verifies(0){}void setComboSkillInfo(int,std::map<short,std::vector<int> >&){++edits;}void verifyComboSkillTree(int){++verifies;}};
struct CUser {int state,job,level,tutorial,queued,dirty;bool character,busy;CParty* party;SkillSlot skills;Inventory inventory;CUser():state(5),job(9),level(1),tutorial(0),queued(0),dirty(0),character(true),busy(false),party(0){}int get_state(){return state;}void* getCurCharacR(){return character?this:0;}int get_charac_level(){return level;}int getCurCharacTutorialFlag(){return tutorial;}int get_charac_job(){return job;}int IsBusyDisjointStore(){return busy;}int getCurCharacNo(){return 42;}CParty* GetParty(){return party;}SkillSlot* getCurCharacSkillW(){return &skills;}const Inventory* getCurCharacInvenR(){return &inventory;}void enableSaveSkill(){dirty=1;}void updateComboSkill(){assert(dirty==1);assert(skills.edits==skills.verifies);++queued;}};
struct CParty{CUser* owner;int state;CParty(CUser* u):owner(u),state(1){}CUser* getManager(){return owner;}int get_state(){return state;}int is_quick_party(){return 0;}};
struct CDataManager{CDungeon dungeon;CEquipItem equip;CDungeon* find_dungeon(int){return &dungeon;}CEquipItem* find_item(int id){return id==100||id==101?&equip:0;}}data;
CDataManager* G_CDataManager(){return &data;}
struct DisPatcher_SelectDungeon{int check_error(CUser*,MSG_BASE&,ParamBase&);int check_error_before(CUser*,MSG_BASE&,ParamBase&);};
'''
cpp+=block(source(tutorial),'bool IsInitialSpecialTutorial(')+'\n'
cpp+=block(source(tutorial,True),'int DisPatcher_SelectDungeon::check_error(').replace('::check_error(', '::check_error_before(',1)+'\n'
cpp+=block(source(tutorial),'int DisPatcher_SelectDungeon::check_error(')+'\n'
cpp+=r'''
const char* kComboSkillProcessSignature="test";int LineFunc(int,const char*,int,int){return -1;}
struct Dispatcher_ComboSkill{int err;Dispatcher_ComboSkill():err(0){}int check_error(CUser*,MSG_BASE&){return err;}int process(CUser*,MSG_BASE&,ParamBase&);int process_before(CUser*,MSG_BASE&,ParamBase&);};
'''
cpp+=block(source(combo,True),'int Dispatcher_ComboSkill::process(').replace('::process(', '::process_before(',1)+'\n'
cpp+=block(source(combo),'int Dispatcher_ComboSkill::process(')+'\n'
cpp+=block(source(cards),'int ResolveClearRewardItemLevel(')+'\n'
cpp+=r'''
struct Dispatcher_Compound_ExtreamItem{unsigned short cube_slot_,source_slots_[2];short result_slot_;Inven_Item input_items_[2],output_item_;const CEquipItem* input_prototypes_[2];unsigned char selection_type_,sphere_type_;CUser* user_;Dispatcher_Compound_ExtreamItem* _reset();int _read_packet(PacketBuf&);};
'''
cpp+=block(source(cube),'Dispatcher_Compound_ExtreamItem* Dispatcher_Compound_ExtreamItem::_reset(')+'\n'
cpp+=block(source(cube),'int Dispatcher_Compound_ExtreamItem::_read_packet(')+'\n'
cpp+=r'''
void TestMission(){
 PacketBuf exact(16,16),observed(16,21),truncated(16,15),extra(16,29),bad(16,22),empty(13,13);
 assert(PacketBodyConsumedBefore(exact));assert(!PacketBodyConsumedBefore(observed));
 assert(PacketBodyConsumed(exact)&&PacketBodyConsumed(observed)&&PacketBodyConsumed(empty));
 assert(!PacketBodyConsumed(truncated)&&!PacketBodyConsumed(extra)&&!PacketBodyConsumed(bad));
 for(int n=13;n<100;++n){PacketBuf p(n,13+((n-13+7)&~7));assert(PacketBodyConsumed(p));p.error=1;assert(!PacketBodyConsumed(p));}
 puts("PASS: CMD553 16/21 padding regression; exact frame, truncation, extra blocks, read errors");
}
void TestGrowth(){tokens.clear();
'''
actual=read(os.path.join(pvf,'101000042.equ'))
start=actual.index('[emancipate]')+len('[emancipate]')
finish=actual.index('[/emancipate]',start)+len('[/emancipate]')
for token in re.findall(r'\[[^\]]+\]|-?\d+',actual[start:finish]):
    cpp+='tokens.push_back("'+token+'");\n'
cpp+=r'''
 token_pos=0;STEquipmentScript before;assert(ParseGrowthBefore(&before)==1);assert(before.equipment_growth_type==0&&before.pairs_7d8.empty());
 token_pos=0;STEquipmentScript after;assert(ParseGrowthAfter(&after)==1);assert(after.equipment_growth_type==1&&after.equipment_growth_stage==1);
 assert(after.pairs_7d8.size()==1&&after.pairs_7d8[0]==std::make_pair(10000224,1));assert(after.pairs_7e4.size()==1&&after.pairs_7e4[0]==std::make_pair(101000043,1));
 puts("PASS: actual PVF 101000042 nested growth type/stage, upgrade material and next-stage output recovered");
}
void TestTutorial(){CUser user;CParty party(&user);user.party=&party;DisPatcher_SelectDungeon dispatch;stSelectDungeonParam result;MSG_SELECT_DUNGEON msg;
 msg.dungeon_index=10001;msg.dungeon_difficulty=0;msg.request_flag=0;msg.dungeon_mode=4;
 assert(dispatch.check_error_before(&user,msg,result)==-1);assert(dispatch.check_error(&user,msg,result)==0&&msg.dungeon_mode==0);
 user.job=10;msg.dungeon_index=10002;msg.dungeon_mode=4;assert(dispatch.check_error(&user,msg,result)==0&&msg.dungeon_mode==0);
 user.job=9;msg.dungeon_index=10002;msg.dungeon_mode=4;assert(dispatch.check_error(&user,msg,result)==-1);
 msg.dungeon_index=10001;user.level=2;assert(dispatch.check_error(&user,msg,result)==-1);user.level=1;
 user.tutorial=1;assert(dispatch.check_error(&user,msg,result)==-1);user.tutorial=0;
 user.state=13;data.dungeon.defense=true;msg.dungeon_index=900;assert(dispatch.check_error(&user,msg,result)==0&&msg.dungeon_mode==4);
 puts("PASS: job 9/10 initial tutorial normalized; wrong job/map, completed tutorial and older character rejected; guardian altar retained");
}
void TestCombo(){CUser user;Dispatcher_ComboSkill dispatch;MSG_COMBO_SKILL msg;stComboSkill result;msg.skill_tree_index=0;
 dispatch.process_before(&user,msg,result);assert(user.dirty==0&&user.queued==0);
 dispatch.process(&user,msg,result);assert(user.dirty==1&&user.queued==1);msg.skill_tree_index=1;dispatch.process(&user,msg,result);assert(user.queued==2);
 dispatch.err=19;dispatch.process(&user,msg,result);assert(user.queued==2);puts("PASS: combo-only edits mark skill dirty and queue after verification; rejected requests do not save");}
void TestCards(){CDungeon d;assert(ResolveClearRewardItemLevel(&d,100)==100);d.risk=true;assert(ResolveClearRewardItemLevel(&d,100)==85);assert(ResolveClearRewardItemLevel(&d,80)==80);assert(ResolveClearRewardItemLevel(0,100)==100);puts("PASS: risk-only item reference 100->85; ordinary dungeons and original EXP/gold reference untouched");}
void TestCube(){CUser user;Inven_Item a;a.item_id_=100;a.equip=true;user.inventory.slots[9]=a;a.item_id_=101;user.inventory.slots[10]=a;a.item_id_=1251;a.equip=false;user.inventory.slots[90]=a;Dispatcher_Compound_ExtreamItem d;d.user_=&user;PacketBuf good;unsigned char raw[]={90,0,9,0,10,0,2};good.bytes.assign(raw,raw+7);assert(d._read_packet(good)==1&&d.sphere_type_==0&&d.selection_type_==2);
 PacketBuf short_packet;short_packet.bytes.assign(raw,raw+6);assert(d._read_packet(short_packet)==0);
 PacketBuf duplicate;duplicate.bytes.assign(raw,raw+7);duplicate.bytes[4]=9;assert(d._read_packet(duplicate)==0);
 user.inventory.slots[10].item_id_=999;PacketBuf missing;missing.bytes.assign(raw,raw+7);assert(d._read_packet(missing)==0);
 puts("PASS: bind-cube request fields and truncation/duplicate/prototype guards; gameplay refusal cause remains unconfirmed");}
int main(){TestMission();TestGrowth();TestTutorial();TestCombo();TestCards();TestCube();}
'''
# Packet layout and gold awarding must not be changed to hide the empty-card cause.
assert block(source(cards),'void PrepareGoldRewardReference(')==block(source(cards,True),'void PrepareGoldRewardReference(')
assert source(cards)[source(cards).index('// After the clear-card cost,'):source(cards).index('const int hell_party_max')]==source(cards,True)[source(cards,True).index('// After the clear-card cost,'):source(cards,True).index('const int hell_party_max')]
tmp=tempfile.mkdtemp(prefix='codex_six_')
path,binary=os.path.join(tmp,'test.cpp'),os.path.join(tmp,'test')
with open(path,'w') as stream:stream.write(cpp)
subprocess.check_call(['/opt/gcc-4.9/bin/g++','-m32','-std=gnu++11','-O0',path,'-o',binary])
subprocess.check_call([binary])
print('HARNESS='+path)
