#ifndef OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_EXTRACTION_RUNTIME_H_
#define OLDSRV_OLD_GAME_QUEST_SKILL_EXPERT_EXTRACTION_RUNTIME_H_

#include <stdint.h>
#include <utility>
#include <vector>

class CUser;
class CMTRand;
struct Inven_Item;

namespace expert_extraction {

// Old 0x84a2938 runtime family.  The base owns the RNG; all three concrete
// strategies are intentionally 0x08 on i386 and differ only by vtable.
class CExtraction {
public:
    CExtraction();
    virtual ~CExtraction();
    virtual bool extraction(CUser*, int16_t, std::vector<std::pair<int,int> >&)=0;
    virtual int check_error(CUser*, char, int16_t, int16_t);
    virtual int get_extraction_count(Inven_Item)=0;
    virtual bool _check_need_slot(CUser*);
    virtual int _check_error(CUser*, int16_t, int16_t)=0;
    bool inven_process(CUser*, int16_t, std::vector<std::pair<int,int> >&, std::vector<int>&);
protected:
    ::CMTRand* random_;
};

class CAlchemistExtraction : public CExtraction {
public:
    bool extraction(CUser*, int16_t, std::vector<std::pair<int,int> >&);
    int get_extraction_count(Inven_Item);
    int _check_error(CUser*, int16_t, int16_t);
};
class CEnchanterExtraction : public CExtraction {
public:
    bool extraction(CUser*, int16_t, std::vector<std::pair<int,int> >&);
    int get_extraction_count(Inven_Item);
    int _check_error(CUser*, int16_t, int16_t);
};
class CDollControllerExtraction : public CExtraction {
public:
    bool extraction(CUser*, int16_t, std::vector<std::pair<int,int> >&);
    int get_extraction_count(Inven_Item);
    int _check_error(CUser*, int16_t, int16_t);
};
class CExtractionMgr {
public:
    CExtractionMgr();
    ~CExtractionMgr();
    CExtraction* get_extracter(unsigned int);
private:
    CExtraction* extracters_[3];
};
int GetExtracterType(int);
} // namespace expert_extraction
#endif
