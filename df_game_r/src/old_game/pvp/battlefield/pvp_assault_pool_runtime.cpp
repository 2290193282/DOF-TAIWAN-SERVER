#include "old_game/pvp/battlefield/pvp_assault_pool_runtime.h"

#include "common/manager/global_tools.h"
#include "infra/log/format/trace.h"

#include <stddef.h>

namespace {

const uint32_t kAssaultPlacePoolDummy = 36641u;
typedef StaticPool<pvp_assault::CAssaultPlace, 128> AssaultPlacePool;

#if defined(__i386__)
typedef char AssaultPlace_Size_Must_Be_0x140[
    (sizeof(pvp_assault::CAssaultPlace) == 0x140) ? 1 : -1];
typedef char AssaultPlacePool_Node_Size_Must_Be_0x148[
    (sizeof(AssaultPlacePool::CNode) == 0x148) ? 1 : -1];
typedef char AssaultPlacePool_Size_Must_Be_0x30[
    (sizeof(AssaultPlacePool) == 0x30) ? 1 : -1];
typedef char AssaultPlacePool_Nodes_Offset_Must_Be_0x00[
    (offsetof(AssaultPlacePool, nodes) == 0x00) ? 1 : -1];
typedef char AssaultPlacePool_FreeNodes_Offset_Must_Be_0x04[
    (offsetof(AssaultPlacePool, free_nodes) == 0x04) ? 1 : -1];
typedef char AssaultPlacePool_ErrorState_Offset_Must_Be_0x2c[
    (offsetof(AssaultPlacePool, error_state) == 0x2c) ? 1 : -1];
#endif

}  // namespace

// IDA_EA: 0x82F13E8
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EE5CNodeD2Ev
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::CNode::~CNode()
template <>
AssaultPlacePool::CNode::~CNode() {}

// IDA_EA: 0x82F1EAC
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EE5CNodeC2Ev
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::CNode::CNode(void)
template <>
AssaultPlacePool::CNode::CNode()
    : pvp_assault::CAssaultPlace(),
      dummy(kAssaultPlacePoolDummy),
      reference_count(0) {}

// IDA_EA: 0x82F1462
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EEC2Ev
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::StaticPool()
template <>
AssaultPlacePool::StaticPool()
    : free_nodes() {
    nodes = new CNode[128];
    if (!nodes) {
        cMyTrace trace("StaticPool", 108, 5);
        trace("[%s][%d]", "StaticPool");
    }

    for (int index = 0; index != 128; ++index) {
        free_nodes.push(nodes + index);
    }
    error_state = 0;
}

// IDA_EA: 0x82F13FC
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EED2Ev
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::~StaticPool()
template <>
AssaultPlacePool::~StaticPool() {
    if (nodes) {
        delete[] nodes;
    }
}

// IDA_EA: 0x82F111C
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EE7AcquireEv
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::Acquire()
template <>
pvp_assault::CAssaultPlace* AssaultPlacePool::Acquire() {
    if (free_nodes.empty()) {
        return 0;
    }

    CNode* const node = free_nodes.front();
    free_nodes.pop();
    if (node->dummy != kAssaultPlacePoolDummy) {
        cMyTrace trace("Acquire", 137, 0);
        trace("STATIC MEMORY_POOL Acquire error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 3;
    }
    if (node->reference_count) {
        cMyTrace trace("Acquire", 143, 0);
        trace("STATIC MEMORY_POOL Acquire error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 4;
    }
    ++node->reference_count;
    return static_cast<pvp_assault::CAssaultPlace*>(node);
}

// IDA_EA: 0x82F124A
// IDA_NAME: _ZN10StaticPoolIN11pvp_assault13CAssaultPlaceELi128EE4FreeEPS1_
// DEMANGLED: StaticPool<pvp_assault::CAssaultPlace,128>::Free(pvp_assault::CAssaultPlace*)
template <>
pvp_assault::CAssaultPlace* AssaultPlacePool::Free(
    pvp_assault::CAssaultPlace* place) {
    if (!place) {
        cMyTrace trace("Free", 160, 5);
        trace("[%s][%d]", "Free");
    }

    CNode* const node = static_cast<CNode*>(place);
    free_nodes.push(node);
    if (node->dummy != kAssaultPlacePoolDummy) {
        cMyTrace trace("Free", 170, 0);
        trace("STATIC MEMORY_POOL Free error dummy(%x), node(%x)",
              node->dummy, node);
        error_state = 5;
    }
    if (node->reference_count != 1) {
        cMyTrace trace("Free", 176, 0);
        trace("STATIC MEMORY_POOL Free error reference count(%d), node(%x)",
              node->reference_count, node);
        error_state = 6;
        PrintBackTrace();
    }
    --node->reference_count;
    return place;
}

namespace pvp_assault {

// IDA_EA: 0x82F10D2
// IDA_NAME: _ZN11pvp_assault13CAssaultPlaceD2Ev
// DEMANGLED: pvp_assault::CAssaultPlace::~CAssaultPlace()
CAssaultPlace::~CAssaultPlace() {}

// IDA_EA: 0x82F0F96
// IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactoryC2Ev
// DEMANGLED: pvp_assault::CAssaultPlaceFactory::CAssaultPlaceFactory()
CAssaultPlaceFactory::CAssaultPlaceFactory()
    : StaticPool<CAssaultPlace, 128>() {}

// IDA_EA: 0x82F0F82
// IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactoryD2Ev
// DEMANGLED: pvp_assault::CAssaultPlaceFactory::~CAssaultPlaceFactory()
CAssaultPlaceFactory::~CAssaultPlaceFactory() {}

// IDA_EA: 0x82F0F54
// IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactory19AcquireAssaultPlaceEv
// DEMANGLED: pvp_assault::CAssaultPlaceFactory::AcquireAssaultPlace()
CAssaultPlace* CAssaultPlaceFactory::AcquireAssaultPlace() {
    return Acquire();
}

// IDA_EA: 0x82F0F68
// IDA_NAME: _ZN11pvp_assault20CAssaultPlaceFactory16FreeAssaultPlaceEPNS_13CAssaultPlaceE
// DEMANGLED: pvp_assault::CAssaultPlaceFactory::FreeAssaultPlace(pvp_assault::CAssaultPlace*)
CAssaultPlace* CAssaultPlaceFactory::FreeAssaultPlace(CAssaultPlace* place) {
    return Free(place);
}

// IDA_EA: 0x82F0FA9
// IDA_NAME: _ZN11pvp_assault30GetInstanceAssaultPlaceFactoryEv
// DEMANGLED: pvp_assault::GetInstanceAssaultPlaceFactory()
CAssaultPlaceFactory* GetInstanceAssaultPlaceFactory() {
    static CAssaultPlaceFactory instance;
    return &instance;
}

// IDA_EA: 0x82EDB6A
// IDA_NAME: _ZN11pvp_assault11CAssaultMgr16FreeAssaultPlaceEiPNS_13CAssaultPlaceE
// DEMANGLED: pvp_assault::CAssaultMgr::FreeAssaultPlace(int,pvp_assault::CAssaultPlace*)
void CAssaultMgr::FreeAssaultPlace(int assault_place_id,
                                   CAssaultPlace* place) {
    if (assault_place_id) {
        std::map<unsigned int, CAssaultPlace*>::iterator it =
            assault_places.find(assault_place_id);
        if (it != assault_places.end()) {
            assault_places.erase(it);
        }
    }
    if (place) {
        place->Reset();
        GetInstanceAssaultPlaceFactory()->FreeAssaultPlace(place);
    }
}

}  // namespace pvp_assault
