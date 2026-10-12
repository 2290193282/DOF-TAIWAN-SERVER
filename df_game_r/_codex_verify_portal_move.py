from __future__ import print_function
import os
import re
import subprocess
import sys


def extract(path):
    with open(path, 'r') as stream:
        text = stream.read()
    start = text.index('int CBattle_Field::MoveMap(')
    end = text.index('{', start) + 1
    depth = 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


root, baseline, pvf_root = sys.argv[1:4]
relative = 'old_game/pvp/battlefield/battle_field_move_select_runtime.cpp'
before = extract(os.path.join(baseline, relative)).replace('::MoveMap(', '::MoveMapBefore(', 1)
after = extract(os.path.join(root, 'src', relative))
with open(os.path.join(pvf_root, 'TimeSpiral.dgn'), 'r') as stream:
    dgn = stream.read()
with open(os.path.join(pvf_root, 'global_timespiral.etc'), 'r') as stream:
    routes = stream.read()
maps = [(int(x), int(y), int(m)) for x,y,m in re.findall(
    r'\[(?:boss )?map specification\]\s*(\d+)\s+(\d+)\s+(\d+)', dgn)]
coordinates = dict((m, (x,y)) for x,y,m in maps)
edges = []
for source, payload in re.findall(r'\[teleport trap\]\s*(\d+)\s*\[move list\](.*?)\[/move list\]', routes, re.S):
    numbers = list(map(int, re.findall(r'-?\d+', payload)))
    assert len(numbers) % 4 == 0
    assert sum(numbers[2::4]) == 1000000
    sx, sy = coordinates[int(source)]
    for i in range(0, len(numbers), 4):
        edges.append((sx, sy, numbers[i], numbers[i+1]))
assert len(maps) == 25 and len(edges) == 48

cpp = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <vector>
#include <utility>
struct MapInfo {int map_index_;unsigned char visited_goto_,visited_transition_,map_layer_,completed_;std::map<int,int> monsters_;MapInfo():map_index_(0),visited_goto_(0),visited_transition_(0),map_layer_(0),completed_(0){} };
struct CMap {int id,entrance_mask;CMap(int n=0):id(n),entrance_mask(0){}bool CheckEntrance(int d){return (entrance_mask&d)!=0;} };
struct CDungeon {int IsEnterEachMap(){return 0;}int get_blood_dungeon_type(){return 0;} };
struct GridScript {int grid_type;std::vector<int> values_04,values_10,values_1c;GridScript():grid_type(1){} };
struct Maze {int grid_width,grid_height;GridScript grids[25];Maze():grid_width(5),grid_height(5){} GridScript& getGridR(int x,int y){return grids[y*5+x];} };
struct Data {CDungeon dungeon;std::map<int,CMap> maps;CDungeon* find_dungeon(int){return &dungeon;}CMap* find_map(int n){std::map<int,CMap>::iterator i=maps.find(n);return i==maps.end()?0:&i->second;} } data;
Data* G_CDataManager(){return &data;}
struct CParty {std::map<int,int> map_int_int_;};
struct CBattle_Field;
namespace time_spiral {bool GetMoveOverride(CParty*,int,int,int*,int*);bool FindCoordinateForMap(int,int*,int*);}
namespace dungeon_runtime {bool IsMoveMapEvenEnemyDungeon(int){return false;} }
namespace zombie_map_runtime {int ResolveGlobalFocusSeat(CBattle_Field*){return -1;} }
struct LogManager {template<class...T>static void logFormat(T...){}};
struct Blood {void reset_blood_values(){}void setBloodRoundOrder(){} };
std::map<int,std::pair<int,int> > coordinate;
struct CBattle_Field {
 CDungeon* dungeon_;Maze* maze_;int dungeon_index_,quest_stage_map_selected_;
 CParty* party_;CMap* current_map_;Blood blood_round_;
 std::pair<int,int> current_position_,previous_position_;
 std::map<std::pair<int,int>,MapInfo> infos;
 bool authorized,closed_room,fail_set,fail_consist;int tx,ty,lx,ly,entrance_checks,consists;
 CBattle_Field(Maze* m,CParty* p,int x,int y):dungeon_(&data.dungeon),maze_(m),dungeon_index_(3900),quest_stage_map_selected_(0),party_(p),current_map_(0),current_position_(x,y),previous_position_(-1,-1),authorized(true),closed_room(false),fail_set(false),fail_consist(false),tx(0),ty(0),lx(x),ly(y+1),entrance_checks(0),consists(0){
  MapInfo& s=infos[current_position_];s.map_index_=maze_->getGridR(x,y).values_04[0];current_map_=data.find_map(s.map_index_);s.monsters_[999]=7;
 }
 int MoveMap(int,int,bool&);int MoveMapBefore(int,int,bool&);
 MapInfo* GetCurrentMapInfo(){return &infos[current_position_];}
 MapInfo* GetMapInfoFromPos(int x,int y,bool){return &infos[std::make_pair(x,y)];}
 int check_grid_clear(){return !closed_room;}void moveMap_1_Log(int,int){}void moveMap_2_Log(int,int){}void moveMap_3_Log(int,int){}void moveMap_4_Log(int,int,const GridScript*){}
 void reset_blood_values(){}void getCurPosXY(int&x,int&y){x=current_position_.first;y=current_position_.second;}
 int check_boss_map(int,int){return 0;}int check_movemap_range(int x,int y){return std::abs(x-current_position_.first)+std::abs(y-current_position_.second)<=1?0:21;}
 void swap_curpos_xy(int x,int y){previous_position_=current_position_;current_position_=std::make_pair(x,y);}
 int check_visited_map(int x,int y,bool& layered){MapInfo* d=GetMapInfoFromPos(x,y,false);if(!d->visited_goto_)return 0;GetCurrentMapInfo()->visited_goto_=1;swap_curpos_xy(x,y);CMap* m=data.find_map(d->map_index_);if(m)current_map_=m;layered=true;return 1;}
 int get_invade_direction(int x,int y){if(x==current_position_.first)return y>current_position_.second?2:8;return x>current_position_.first?4:1;}
 void choose_random_bossmap_from_dummymaps(const CDungeon*,const GridScript*,int&,bool&,int,int,int){}
 int get_candidator_map(const CDungeon*,const GridScript*g,int&s,std::vector<int>&c,int,int,int d){if(s!=-1)return 0;const std::vector<int>&v=g->values_04.empty()?g->values_10:g->values_04;for(unsigned i=0;i<v.size();++i){CMap*m=data.find_map(v[i]);if(!m)return 23;++entrance_checks;if(m->CheckEntrance(d))c.push_back(v[i]);}return 0;}
 void set_visited_map(int,int,bool b){GetCurrentMapInfo()->visited_goto_=b;}
 void set_visited_map_goto(int x,int y){GetMapInfoFromPos(x,y,false)->visited_transition_=1;}
 int set_map_id(const GridScript*,int&s,std::vector<int>&c){if(s<0&&!c.empty())s=c[0];GetCurrentMapInfo()->map_index_=s;return fail_set||s<=0?21:0;}
 int ConsistMap(){++consists;MapInfo*d=GetCurrentMapInfo();current_map_=data.find_map(d->map_index_);d->monsters_[123]=4;return !fail_consist&&current_map_?1:0;}
};
CBattle_Field* active;
namespace time_spiral {
 bool GetMoveOverride(CParty*p,int x,int y,int*lx,int*ly){if(!active->authorized||p!=active->party_||x!=active->tx||y!=active->ty)return false;*lx=active->lx;*ly=active->ly;return true;}
 bool FindCoordinateForMap(int m,int*x,int*y){std::map<int,std::pair<int,int> >::iterator i=coordinate.find(m);if(i==coordinate.end())return false;*x=i->second.first;*y=i->second.second;return true;}
}
'''
cpp += before + '\n' + after
cpp += '\nint map_rows[][3]={' + ','.join('{%d,%d,%d}' % x for x in maps) + '};\n'
cpp += 'int edges[][4]={' + ','.join('{%d,%d,%d,%d}' % x for x in edges) + '};\n'
cpp += r'''
int main(){
 Maze maze;CParty party;
 for(unsigned i=0;i<25;++i){int*x=map_rows[i];coordinate[x[2]]=std::make_pair(x[0],x[1]);data.maps[x[2]]=CMap(x[2]);maze.getGridR(x[0],x[1]).values_04.push_back(x[2]);}
 CBattle_Field before(&maze,&party,0,3);active=&before;before.tx=3;before.ty=0;bool layered=false;
 assert(before.MoveMapBefore(3,0,layered)==21);assert(before.current_position_==std::make_pair(3,0));assert(before.current_map_->id==27103);
 puts("PASS reproducer: old MoveMap rejects target 27115 and leaves new coordinate with old map");
 for(unsigned i=0;i<sizeof(edges)/sizeof(edges[0]);++i){int*e=edges[i];CBattle_Field f(&maze,&party,e[0],e[1]);active=&f;f.tx=e[2];f.ty=e[3];layered=false;assert(f.MoveMap(e[2],e[3],layered)==0);assert(coordinate[f.current_map_->id]==std::make_pair(e[2],e[3]));assert(f.entrance_checks==0&&f.consists==1);}
 puts("PASS all 48 PVF source-target routes without a source-door entrance requirement");
 for(int point=0;point<2;++point){CBattle_Field f(&maze,&party,0,3);active=&f;f.tx=3;f.ty=0;f.fail_set=point==0;f.fail_consist=point==1;layered=false;assert(f.MoveMap(3,0,layered)==21);assert(f.current_position_==std::make_pair(0,3)&&f.previous_position_==std::make_pair(-1,-1)&&f.current_map_->id==27103);assert(f.GetCurrentMapInfo()->visited_goto_==0&&f.GetCurrentMapInfo()->monsters_[999]==7);MapInfo*d=f.GetMapInfoFromPos(3,0,false);assert(d->map_index_==0&&d->visited_transition_==0&&d->monsters_.empty());f.fail_set=f.fail_consist=false;assert(f.MoveMap(3,0,layered)==0);}
 puts("PASS injected map-ID/ConsistMap failures restore coordinates, map pointer, source visit and destination state; retry succeeds");
 CBattle_Field ordinary(&maze,&party,0,3);active=&ordinary;ordinary.authorized=false;data.maps[27104].entrance_mask=2;assert(ordinary.MoveMap(0,4,layered)==0&&ordinary.entrance_checks==1);
 CBattle_Field unauthorized(&maze,&party,0,3);active=&unauthorized;unauthorized.authorized=false;assert(unauthorized.MoveMap(3,0,layered)==21&&unauthorized.current_position_==std::make_pair(0,3));
 CBattle_Field invalid(&maze,&party,0,3);active=&invalid;invalid.tx=3;invalid.ty=0;CMap saved=data.maps[27115];data.maps.erase(27115);assert(invalid.MoveMap(3,0,layered)==23&&invalid.current_position_==std::make_pair(0,3));data.maps[27115]=saved;
 CBattle_Field cached(&maze,&party,0,3);active=&cached;cached.tx=3;cached.ty=0;MapInfo*d=cached.GetMapInfoFromPos(3,0,false);d->map_index_=27115;d->visited_goto_=1;d->monsters_[12]=99;layered=false;assert(cached.MoveMap(3,0,layered)==0&&layered&&cached.consists==0&&cached.GetCurrentMapInfo()->monsters_[12]==99);
 puts("PASS ordinary movement, unauthorized nonadjacent request, missing map and cached room re-entry");
}
'''
binary = os.path.join(root, '_codex_heras_portal_test')
p = subprocess.Popen(['/opt/gcc-4.9/bin/g++','-m32','-std=gnu++11','-O0','-x','c++','-','-o',binary], stdin=subprocess.PIPE)
p.communicate(cpp.encode('utf-8'))
if p.returncode:
    sys.exit(p.returncode)
sys.exit(subprocess.call([binary]))
