from __future__ import print_function
import os
import subprocess
import sys
import tempfile


def read(path):
    with open(path, 'r') as handle:
        return handle.read()


def block(text, marker):
    start = text.index(marker)
    brace = text.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[brace + 1:end - 1]


def function(text, marker):
    start = text.index(marker)
    return text[start:text.index('{', start) + 1] + block(text, marker) + '}\n'


root, baseline = sys.argv[1:3]
relative = 'old_data/PVF/equipment/equipment_script.cpp'
before = block(read(os.path.join(baseline, relative)), 'if (tag == "[character item check]")')
after = block(read(os.path.join(root, 'src', relative)), 'if (tag == "[character item check]")')
upgrade = read(os.path.join(root, 'src/old_data/PVF/equipment/item_upgrade_3rd_chronicle.cpp'))
old_upgrade = read(os.path.join(baseline, 'old_data/PVF/equipment/item_upgrade_3rd_chronicle.cpp'))
cpp = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <sstream>
namespace oldsrv { namespace config {enum {kEquipmentTypeTitle=12,kOrdinaryEquipmentTypeFirst=11,kOrdinaryEquipmentTypeLast=22};} }
struct STEnchantSystemData {int32_t value_00;uint8_t value_04,pad[3];int32_t value_08;STEnchantSystemData():value_00(0),value_04(0),value_08(0){} };
static_assert(sizeof(STEnchantSystemData)==12,"ABI");
struct STEnchantSystemMapKey:STEnchantSystemData {uint8_t job;bool operator<(const STEnchantSystemMapKey& o)const {if(value_00!=o.value_00)return value_00<o.value_00;if(value_04!=o.value_04)return value_04<o.value_04;if(value_08!=o.value_08)return value_08<o.value_08;return job<o.job;} };
struct EquipmentSkillDataUp {int n;};
struct STEnchantSystemMapData {std::vector<EquipmentSkillDataUp> skill_data;};
struct STEquipmentScript {std::vector<STEnchantSystemData> enchant_data_7cc;};
std::istringstream scanner;
int ScanInt(bool* ok){int n=0;*ok=bool(scanner>>n);return n;}
int ScanStr(std::string* s){return bool(scanner>>*s);}
int getEquipmentTypeToEnumValue(const std::string& s){return s=="weapon"?11:atoi(s.c_str());}
#define EQUIP_FAIL() return 0
'''
cpp += 'int ParseBefore(STEquipmentScript* script) {' + before + 'return 1;}\n'
cpp += 'int ParseAfter(STEquipmentScript* script) {' + after + 'return 1;}\n'
cpp += r'''
enum INVEN_TYPE {INV1=1};enum ENUM_EQUIPMENTTYPE {EQ11=11};enum eItemDelReason {DEL12=12};enum ENUM_ITEMSPACE {SPACE0=0};enum VALUE_STATISTIC_FIELD {FIELD15=15};
struct STEnchantedSkillOption {uint8_t flags_;STEnchantedSkillOption():flags_(0){} };
struct Amplify {int identified;bool ability;Amplify():identified(1),ability(false){}int isIdentified()const{return identified;}bool hasAbility()const{return ability;} };
struct Inven_Item {enum ITEM_TYPE{QUICK=0};int32_t item_id_;uint8_t item_category_;uint16_t avatar_select_;int add_info_;Amplify amplify_option_;struct {STEnchantedSkillOption enchanted_skill_options_[2];}chronicle_random_option_;Inven_Item():item_id_(0),item_category_(0),avatar_select_(35),add_info_(0){}bool isEmpty()const{return !item_id_;}int get_add_info()const{return add_info_;} };
struct CItem;
struct VTable {int(*get_item_type)(const CItem*);};
struct CItem {VTable* vtable_;int id,type;bool stack;CItem();int get_rarity()const{return 3;}int get_index()const{return id;}int is_stackable()const{return stack;} };
int ItemType(const CItem* p){return p->type;}VTable vt={ItemType};CItem::CItem():vtable_(&vt),id(0),type(11),stack(false){}
struct CEquipItem:CItem {std::vector<STEnchantSystemData> allowed;int get_endurance()const{return 35;}const void* getEnchantSystemData()const{return &allowed;} };
struct CStackableItem:CItem {std::map<STEnchantSystemMapKey,STEnchantSystemMapData> values;uint8_t group;char rates[2];CStackableItem():group(0){stack=true;rates[0]=100;rates[1]=11;}uint8_t third_chronicle_enchant_group()const{return group;}char* third_chronicle_enchant_rates(){return rates;}std::map<STEnchantSystemMapKey,STEnchantSystemMapData>& third_chronicle_enchant_map(){return values;} };
struct CInventory {std::map<int,Inven_Item> slots;int deletes,updates,empty,quick;bool delete_fail;CInventory():deletes(0),updates(0),empty(10),quick(0),delete_fail(false){}Inven_Item GetInvenSlot(INVEN_TYPE,int s)const{std::map<int,Inven_Item>::const_iterator it=slots.find(s);return it==slots.end()?Inven_Item():it->second;}void StartEndPos(Inven_Item::ITEM_TYPE,int& a,int& b){a=3;b=8;}int InvenEmptycount(Inven_Item::ITEM_TYPE)const{return empty;}int QuickEmptyCount()const{return quick;}int delete_item(INVEN_TYPE,int s,int,int,bool){if(delete_fail)return 0;++deletes;--slots[s].add_info_;return 1;}void update_item(INVEN_TYPE,int s,const Inven_Item& t){++updates;slots[s]=t;} };
struct History {void Enchant3rdChronicleItem(bool,int,int,int,int){} };
struct CUser {enum eSendTarget{SELF=1};int job,grow,sends;CInventory inv;History hist;CUser():job(0),grow(1),sends(0){}int get_charac_job()const{return job;}int getCurCharFirstGrowType()const{return grow;}int getCurCharacNo()const{return 42;}const CInventory* getCurCharacInvenR()const{return &inv;}CInventory* getCurCharacInvenW(){return &inv;}void SendUpdateItem(eSendTarget,ENUM_ITEMSPACE,int){++sends;}History& history_log(){return hist;} };
struct Disjoint {int getPredictResultItemTypeCount(int){return 2;} };
struct Data {std::map<int,CItem*> items;Disjoint disjoint;CItem* find_item(int id){return items.count(id)?items[id]:0;}Disjoint* get_disjoint(){return &disjoint;} } data;
Data* G_CDataManager(){return &data;}
struct LogManager {template<class...T>static void logFormat(T...){}};
struct PacketGuard {std::vector<uint8_t> bytes;void put_byte(int n){bytes.push_back(n);}void put_short(int n){put_byte(n);put_byte(n>>8);} };
struct Store {void GetSellItemPrice(const Inven_Item&,CItem*,int16_t,bool,int& n){n=0;} }store;
Store* G_Store(){return &store;}
struct Stats {void AddValueStatistic(VALUE_STATISTIC_FIELD,CUser*,unsigned){} }stats;
Stats* GetInstanceValueStatistic(){return &stats;}
struct MSG_ENCHANT {uint16_t target_slot,material_slot;uint32_t target_item_id,material_item_id;uint8_t enchant_type;};
struct Random {unsigned value,calls;Random():value(0),calls(0){}unsigned randInt(unsigned){++calls;return value;} }random_instance;
namespace WongWork {struct CItemUpgrade {Random* random_;int penalties;CItemUpgrade():random_(&random_instance),penalties(0){}bool isDoEnchant(const Inven_Item&,char*);int _CheckCondition(const CUser*,const Inven_Item&,const CItem*,const Inven_Item&,uint8_t);int ProcUpgrade_3rdChronicle(CUser*,ENUM_EQUIPMENTTYPE,uint8_t,Inven_Item&,const CItem*);int ProcUpgrade(CUser*,MSG_ENCHANT*,PacketGuard*);int ProcUpgradeBefore(CUser*,MSG_ENCHANT*,PacketGuard*);int _ProcPenalty(CUser*,Inven_Item* t,const CItem*,uint16_t slot,PacketGuard* p){++penalties;t->item_id_=0;p->put_short(slot);p->put_byte(0);return 0;} };
'''
for marker in ('bool CItemUpgrade::isDoEnchant(', 'int CItemUpgrade::_CheckCondition(',
               'int CItemUpgrade::ProcUpgrade_3rdChronicle(', 'int CItemUpgrade::ProcUpgrade('):
    cpp += function(upgrade, marker)
cpp += function(old_upgrade, 'int CItemUpgrade::ProcUpgrade(').replace('::ProcUpgrade(', '::ProcUpgradeBefore(', 1)
cpp += r'''
}
void parse(STEquipmentScript& script,bool fixed,int job,int grow,int type){std::ostringstream os;os<<job<<" "<<grow<<" "<<type;scanner.clear();scanner.str(os.str());assert((fixed?ParseAfter(&script):ParseBefore(&script))==1);assert(script.enchant_data_7cc.size()==1);}
int main(){
 CEquipItem equip;equip.id=10001;CStackableItem breath;breath.id=1254;data.items[equip.id]=&equip;data.items[breath.id]=&breath;
 CUser user;Inven_Item target;target.item_id_=equip.id;target.item_category_=1;Inven_Item material;material.item_id_=breath.id;material.add_info_=5;
 WongWork::CItemUpgrade proc;int cases=0;
 for(int job=0;job<=10;++job)for(int grow=1;grow<=4;++grow)for(int type=11;type<=22;++type){
  if(type==12)continue;
  STEquipmentScript bad,good;parse(bad,false,job,grow,type);parse(good,true,job,grow,type);user.job=job;user.grow=grow;equip.type=type;
  equip.allowed=bad.enchant_data_7cc;assert(proc._CheckCondition(&user,target,&equip,material,3)==1);
  equip.allowed=good.enchant_data_7cc;assert(good.enchant_data_7cc[0].value_04==grow);assert(good.enchant_data_7cc[0].value_08==type);assert(proc._CheckCondition(&user,target,&equip,material,3)==0);++cases;
 }
 user.job=0;user.grow=1;equip.type=11;STEquipmentScript allowed;parse(allowed,true,0,1,11);equip.allowed=allowed.enchant_data_7cc;
 STEnchantSystemMapKey key;key.value_00=0;key.value_04=1;key.value_08=11;key.job=3;breath.values[key]=STEnchantSystemMapData();
 MSG_ENCHANT msg={9,90,10001,1254,3};PacketGuard packet;
 user.inv.slots[9]=target;user.inv.slots[90]=material;
 assert(proc.ProcUpgrade(&user,&msg,&packet)==0);assert(user.inv.slots[9].chronicle_random_option_.enchanted_skill_options_[0].flags_==25);
 const uint8_t expected[]={1,90,0,4,0,1,9,0,0};assert(packet.bytes==std::vector<uint8_t>(expected,expected+9));assert(user.inv.deletes==1&&user.inv.updates==1);
 breath.group=2;packet.bytes.clear();assert(proc.ProcUpgrade(&user,&msg,&packet)==0);assert(user.inv.slots[9].chronicle_random_option_.enchanted_skill_options_[0].flags_==25);assert(user.inv.slots[9].chronicle_random_option_.enchanted_skill_options_[1].flags_==29);
 int consumed=user.inv.deletes;packet.bytes.clear();assert(proc.ProcUpgrade(&user,&msg,&packet)==19);assert(user.inv.deletes==consumed&&packet.bytes.empty());
 user.inv.slots[9]=target;user.inv.slots[9].chronicle_random_option_.enchanted_skill_options_[0].flags_=29;assert(proc.ProcUpgrade(&user,&msg,&packet)==19);assert(user.inv.deletes==consumed);
 user.inv.slots[9]=target;msg.enchant_type=31;unsigned rolls=random_instance.calls;assert(proc.ProcUpgrade(&user,&msg,&packet)==19);assert(user.inv.deletes==consumed&&random_instance.calls==rolls&&proc.penalties==0);msg.enchant_type=3;
 user.inv.slots[9].avatar_select_=34;assert(proc.ProcUpgrade(&user,&msg,&packet)==7);user.inv.slots[9]=target;
 user.job=1;assert(proc.ProcUpgrade(&user,&msg,&packet)==1);user.job=0;
 user.inv.empty=0;assert(proc.ProcUpgrade(&user,&msg,&packet)==21);user.inv.empty=10;
 user.inv.delete_fail=true;assert(proc.ProcUpgrade(&user,&msg,&packet)==17);assert(user.inv.deletes==consumed&&packet.bytes.empty());user.inv.delete_fail=false;
 user.inv.slots[9].chronicle_random_option_.enchanted_skill_options_[0].flags_=25;random_instance.value=99;assert(proc.ProcUpgrade(&user,&msg,&packet)==0);assert(packet.bytes.size()==9&&packet.bytes[5]==0&&packet.bytes[6]==9&&packet.bytes[8]==0);assert(proc.penalties==1);
 // Original success had only six bytes, but the 0828 receiver needs nine.
 user.inv.slots[9]=target;random_instance.value=0;packet.bytes.clear();assert(proc.ProcUpgradeBefore(&user,&msg,&packet)==0);assert(packet.bytes.size()==6);
 puts("PASS: old parser rejected / fixed parser accepted all job-grow-slot cases");printf("PARSER_CASES=%d\n",cases);
 puts("PASS: first/second infusion, duplicate/full rejection, missing skill no-consumption/no-roll, durability, job, free slots, delete failure, unchanged random failure and CMD367 9-byte success response");
}
'''
temp = tempfile.mkdtemp(prefix='codex_chronicle_test_')
source, binary = os.path.join(temp, 'test.cpp'), os.path.join(temp, 'test')
with open(source, 'w') as stream:
    stream.write(cpp)
subprocess.check_call(['/opt/gcc-4.9/bin/g++', '-m32', '-std=gnu++11', '-O0', '-g', source, '-o', binary])
subprocess.check_call([binary])
print('HARNESS=' + source)
