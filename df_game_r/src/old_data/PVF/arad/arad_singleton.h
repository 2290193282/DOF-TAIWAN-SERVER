#ifndef OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SINGLETON_H
#define OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SINGLETON_H

namespace ARAD {

// The old server uses an empty CRTP base for its singleton-backed services.
// Keep this declaration separate from individual service headers so every
// explicit specialization denotes the same C++ template type.
#ifndef OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_
#define OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_
template <typename T>
class Singleton {
protected:
    Singleton();

public:
    static T* Get();
    static T* GetPtr();
};
#endif  // OLDSRV_ARAD_SINGLETON_TEMPLATE_DECLARATION_H_

}  // namespace ARAD

#endif  // OLDSRV_REBUILD_OLD_DATA_PVF_ARAD_ARAD_SINGLETON_H
