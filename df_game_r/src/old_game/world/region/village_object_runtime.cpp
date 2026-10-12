#include "common/manager/user.h"
#include "old_game/player/core/cuser.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <algorithm>
#include <list>
#include <map>
#include <stdint.h>




namespace village_object {

typedef unsigned char (*IObjectMakePacketFn)(IObject*, PacketGuard*);

// IDA_EA: 0x85CB63A
// IDA_NAME: _ZN14village_object4ZoneC2Ehh
// DEMANGLED: village_object::Zone::Zone(unsigned char,unsigned char)
Zone::Zone(uint8_t village_value, uint8_t area_value)
    : village(village_value), area(area_value) {}

// IDA_EA: 0x86B744C
// IDA_NAME: _ZNK14village_object4ZoneltERKS0_
// DEMANGLED: village_object::Zone::operator<(village_object::Zone const&)const
bool Zone::operator<(const Zone& rhs) const {
    if (village < rhs.village) {
        return true;
    }
    return village == rhs.village && area < rhs.area;
}

// IDA_EA: 0x86B6F30
// IDA_NAME: _ZN14village_object17CVillageObjectMgrC2Ev
// DEMANGLED: village_object::CVillageObjectMgr::CVillageObjectMgr(void)
CVillageObjectMgr::CVillageObjectMgr()
    : objects() {}

// IDA_EA: 0x86B6F44
// IDA_NAME: _ZN14village_object17CVillageObjectMgrD2Ev
// DEMANGLED: village_object::CVillageObjectMgr::~CVillageObjectMgr()
CVillageObjectMgr::~CVillageObjectMgr() {}

// IDA_EA: 0x86B6F58
// IDA_NAME: _ZN14village_object17CVillageObjectMgr15register_objectENS_4ZoneEPNS_7IObjectE
// DEMANGLED: village_object::CVillageObjectMgr::register_object(village_object::Zone,village_object::IObject *)
int CVillageObjectMgr::register_object(Zone zone, IObject* object) {
    std::map<Zone, std::list<IObject*> >::iterator area = objects.find(zone);
    if (area != objects.end()) {
        std::list<IObject*>& area_objects = area->second;
        if (std::find(area_objects.begin(), area_objects.end(), object) !=
            area_objects.end()) {
            return 0;
        }
        area_objects.push_back(object);
    } else {
        std::list<IObject*> area_objects;
        area_objects.push_back(object);
        objects.insert(std::make_pair(zone, area_objects));
    }
    return 1;
}

// IDA_EA: 0x86B70D0
// IDA_NAME: _ZN14village_object17CVillageObjectMgr17unregister_objectENS_4ZoneEPNS_7IObjectE
// DEMANGLED: village_object::CVillageObjectMgr::unregister_object(village_object::Zone,village_object::IObject *)
int CVillageObjectMgr::unregister_object(Zone zone, IObject* object) {
    std::map<Zone, std::list<IObject*> >::iterator area = objects.find(zone);
    if (area == objects.end()) {
        return 0;
    }

    std::list<IObject*>& area_objects = area->second;
    std::list<IObject*>::iterator found =
        std::find(area_objects.begin(), area_objects.end(), object);
    if (found == area_objects.end()) {
        return 0;
    }
    area_objects.erase(found);
    return 1;
}


// IDA_EA: 0x86B71DA
// IDA_NAME: _ZN14village_object17CVillageObjectMgr12on_move_areaEP5CUser
// DEMANGLED: village_object::CVillageObjectMgr::on_move_area(CUser *)
void CVillageObjectMgr::on_move_area(CUser* user) {
    const uint8_t area = static_cast<uint8_t>(user->get_area(false));
    const uint8_t village = static_cast<uint8_t>(
        user->getCurCharacVill());
    Zone zone(village, area);

    std::map<Zone, std::list<IObject*> >::iterator it = objects.find(zone);
    if (it == objects.end()) {
        return;
    }

    std::list<IObject*>& objects = it->second;
    PacketGuard packet;
    packet.clear();
    packet.put_header(0, 202);
    int count_index = packet.get_index();
    int object_count = 0;
    packet.put_byte(0);

    for (std::list<IObject*>::const_iterator obj_it = objects.begin();
         obj_it != objects.end();
         ++obj_it) {
        IObject* object = *obj_it;
        if (!object) {
            continue;
        }
        void** vtable = *reinterpret_cast<void***>(object);
        IObjectMakePacketFn make_packet =
            reinterpret_cast<IObjectMakePacketFn>(vtable[1]);
        if (make_packet && make_packet(object, &packet) == 1) {
            ++object_count;
        }
    }

    packet.put_byte(count_index, object_count);
    packet.finalize(true);
    user->Send(packet);
}

}  // namespace village_object
