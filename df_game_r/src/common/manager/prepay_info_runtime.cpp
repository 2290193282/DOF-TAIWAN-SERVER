#include "common/manager/prepay_info_abi.h"

#include "common/manager/global_instance.h"

#include <new>
#include <utility>


// IDA_EA: 0x8231EDE
// IDA_NAME: _ZN14CPrePayInfoMgrC2Ev
// DEMANGLED: CPrePayInfoMgr::CPrePayInfoMgr(void)
CPrePayInfoMgr::CPrePayInfoMgr() : entries_() {
    // The old constructor immediately calls std::map::clear after C2.
    entries_.clear();
}


// IDA_EA: 0x82151B8
// IDA_NAME: _ZN14CPrePayInfoMgr14FindPrePayInfoEi
// DEMANGLED: CPrePayInfoMgr::FindPrePayInfo(int)
PrePayInfo* CPrePayInfoMgr::FindPrePayInfo(int charac_no) {
    PrePayInfoMap::iterator it = entries_.find(charac_no);
    return it == entries_.end() ? 0 : &it->second;
}


// IDA_EA: 0x84ED0BC
// IDA_NAME: _ZN14CPrePayInfoMgr16InsertPrePayInfoEi10PrePayInfo
// DEMANGLED: CPrePayInfoMgr::InsertPrePayInfo(int,PrePayInfo)
void CPrePayInfoMgr::InsertPrePayInfo(int charac_no, PrePayInfo info) {
    entries_.insert(std::make_pair(charac_no, info));
}


// IDA_EA: 0x84ED10E
// IDA_NAME: _ZN14CPrePayInfoMgr16DeletePrePayInfoEi
// DEMANGLED: CPrePayInfoMgr::DeletePrePayInfo(int)
void CPrePayInfoMgr::DeletePrePayInfo(int charac_no) {
    entries_.erase(charac_no);
}


template <>
CPrePayInfoMgr* GlobalInstance<CPrePayInfoMgr>::m_p = 0;

// Declare the specialization before inst_ptr calls it.  This preserves the
// original two-function template surface without allowing the primary
// GlobalInstance template to instantiate a synthetic create body here.
template <>
CPrePayInfoMgr* GlobalInstance<CPrePayInfoMgr>::create();

template <>
CPrePayInfoMgr* GlobalInstance<CPrePayInfoMgr>::inst_ptr() {
    GlobalInstance<CPrePayInfoMgr>::create();
    return GlobalInstance<CPrePayInfoMgr>::m_p;
}

template <>
CPrePayInfoMgr* GlobalInstance<CPrePayInfoMgr>::create() {
    CPrePayInfoMgr* ptr = GlobalInstance<CPrePayInfoMgr>::m_p;
    if (ptr == 0) {
        // IDA create @ 0x823929C allocates precisely 0x18 bytes before C2.
        void* const storage = ::operator new(0x18u);
        ptr = new (storage) CPrePayInfoMgr();
        GlobalInstance<CPrePayInfoMgr>::m_p = ptr;
    }
    return ptr;
}


// IDA_EA: 0x8231F20
// IDA_NAME: _Z16G_CPrePayInfoMgrv
// DEMANGLED: G_CPrePayInfoMgr(void)
CPrePayInfoMgr* G_CPrePayInfoMgr() {
    return GlobalInstance<CPrePayInfoMgr>::inst_ptr();
}
