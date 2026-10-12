#include "infra/log/format/trace.h"
#include "old_game/world/region/game_world.h"
#include "infra/network/packet/packet_guard.h"

#include <stdint.h>
#include <vector>


// IDA_EA: 0x86C3B58
// IDA_NAME: _ZN7Village13get_gate_areaEv
// DEMANGLED: Village::get_gate_area(void)
bool Village::has_gate_area() const {
    for (int i = 0; i < area_count; ++i) {
        if (static_cast<int>(areas[i].town_area_type) == 1) {
            return true;
        }
    }
    return false;
}

int Village::get_gate_area() {
    for (int i = 0; i < area_count; ++i) {
        if (static_cast<int>(areas[i].town_area_type) == 1) {
            return i;
        }
    }
    return 1;
}

// IDA_EA: 0x86C3BA2
// IDA_NAME: _ZN7Village7getAreaEi
// DEMANGLED: Village::getArea(int)
Area* Village::getArea(int area_index) {
    if (area_index >= 0 && area_count > area_index) {
        return &areas[area_index];
    }

    cMyTrace trace("Area* Village::getArea(int)", 854, 5);
    trace("area(%d) < 0 || area >= m_iAreaCount(%d)",
          area_index,
          area_count);
    return 0;
}

// IDA_EA: 0x86C3C20
// IDA_NAME: _ZN7Village12send_to_areaEiR11PacketGuard
// DEMANGLED: Village::send_to_area(int,PacketGuard &)
bool Village::send_to_area(int area_index, PacketGuard& packet) {
    if (area_index >= 0 && area_count > area_index) {
        return areas[area_index].send_to_all(packet);
    }

    cMyTrace trace("void Village::send_to_area(int, PacketGuard&)", 864, 5);
    trace("area(%d) < 0 || area >= m_iAreaCount(%d)",
          area_index,
          area_count);
    return false;
}

// IDA_EA: 0x86C3C98
// IDA_NAME: _ZN7Village16get_user_id_listEiRSt6vectorItSaItEE
// DEMANGLED: Village::get_user_id_list(int,std::vector<unsigned short,std::allocator<unsigned short>> &)
bool Village::get_user_id_list(int area_index,
                               std::vector<uint16_t>& user_ids) {
    if (area_index >= 0 && area_count > area_index) {
        return areas[area_index].get_user_id_list(user_ids);
    }

    cMyTrace trace(
        "void Village::get_user_id_list(int, std::vector<short unsigned int, std::allocator<short unsigned int> >&)",
        874,
        5);
    trace("area(%d) < 0 || area >= m_iAreaCount(%d)",
          area_index,
          area_count);
    return false;
}

// IDA_EA: 0x86C3D10
// IDA_NAME: _ZNK7Village12GetUserCountEi
// DEMANGLED: Village::GetUserCount(int)const
int Village::GetUserCount(int area_index) const {
    if (area_index >= 0 && area_count > area_index) {
        return areas[area_index].GetUserCount();
    }
    return 0;
}
