#include "old_game/pvp/battlefield/pvp_assault.h"

#include "old_game/world/region/world_area_types.h"
#include "old_game/player/core/cuser.h"

#include <algorithm>

namespace pvp_assault {

namespace {

// IDA data: _ZN11pvp_assaultL25ASSAULT_RECT_PLAYER_POINTE_6 @ 0x8c1cce0.
const int kAssaultRectPlayerPoint[16] = {
    0, 15, 56, 5, 84, 0, 28, 10,
    260, 15, 204, 5, 176, 0, 232, 10,
};

// IDA data: _ZN11pvp_assaultL23ASSAULT_RECT_ICON_POINTE_6 @ 0x8c1cd20.
const int kAssaultRectIconPointX = 130;
const int kAssaultRectIconPointY = 5;

}  // namespace

// IDA_EA: 0x82E5894
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrix18_AddCandidatePointERKNS_7Point2dES3_S3_i
// DEMANGLED: pvp_assault::CCandidateMatrix::_AddCandidatePoint(pvp_assault::Point2d const&,pvp_assault::Point2d const&,pvp_assault::Point2d const&,int)
void CCandidateMatrix::_AddCandidatePoint(const Point2d& candidate,
                                          const Point2d& origin,
                                          const Point2d& opponent,
                                          int base_score) {
    CandidatePoint point;
    point.point = candidate;
    point.score = base_score + static_cast<int>(candidate - opponent) -
                  static_cast<int>(origin - candidate);
    push_back(point);
}

// IDA_EA: 0x82E590C
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrix2AtEjRiS1_
// DEMANGLED: pvp_assault::CCandidateMatrix::At(unsigned int,int &,int &)
int CCandidateMatrix::At(unsigned int index, int& pos_x, int& pos_y) {
    if (index < size()) {
        pos_x = (*this)[index].point.x;
        pos_y = (*this)[index].point.y;
    }
    return 0;
}

// IDA_EA: 0x82E5CA8
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrix18MakeCandidatePointERKNS_7Point2dES3_
// DEMANGLED: pvp_assault::CCandidateMatrix::MakeCandidatePoint(pvp_assault::Point2d const&,pvp_assault::Point2d const&)
bool CCandidateMatrix::MakeCandidatePoint(const Point2d& origin,
                                          const Point2d& opponent) {
    _AddCandidatePoint(origin, origin, opponent, 10);
    for (int distance = 1; distance <= 7; ++distance) {
        _AddCandidatePoint(Point2d(origin.x, origin.y + 10 * distance),
                           origin, opponent, 10);
        _AddCandidatePoint(Point2d(origin.x, origin.y - 10 * distance),
                           origin, opponent, 10);
    }

    // The original GetMaxCandidateMatirxRow() returns 15: exactly the seed
    // row count established above.  Freeze that snapshot before appending the
    // horizontal candidates so iteration never consumes its own additions.
    const unsigned int seed_rows = static_cast<unsigned int>(size());
    for (unsigned int row = 0; row < seed_rows; ++row) {
        const Point2d seed = (*this)[row].point;
        for (int distance = 1; distance <= 7; ++distance) {
            _AddCandidatePoint(Point2d(seed.x + 10 * distance, seed.y),
                               origin, opponent, 0);
            _AddCandidatePoint(Point2d(seed.x - 10 * distance, seed.y),
                               origin, opponent, 0);
        }
    }
    return false;
}

// IDA_EA: 0x82E5F36
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrix20RemoveCandidatePointEPSt6vectorI7MapAreaSaIS2_EE
// DEMANGLED: pvp_assault::CCandidateMatrix::RemoveCandidatePoint(std::vector<MapArea,std::allocator<MapArea>> *)
void CCandidateMatrix::RemoveCandidatePoint(std::vector<MapArea>* map_areas) {
    iterator it = begin();
    while (it != end()) {
        if (::IsIncludeVirtualMoveAreas(map_areas,
                                        it->point.x,
                                        it->point.y) == 1) {
            ++it;
        } else {
            it = erase(it);
        }
    }
}

// IDA_EA: 0x82E5FEE
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrix21ArrangeCandidatePointEv
// DEMANGLED: pvp_assault::CCandidateMatrix::ArrangeCandidatePoint(void)
void CCandidateMatrix::ArrangeCandidatePoint() {
    std::sort(begin(), end());
}

// IDA_EA: 0x82F1060
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrixC2Ev
// DEMANGLED: pvp_assault::CCandidateMatrix::CCandidateMatrix(void)
CCandidateMatrix::CCandidateMatrix()
    : std::vector<CandidatePoint>() {}

// IDA_EA: 0x82F1074
// IDA_NAME: _ZN11pvp_assault16CCandidateMatrixD2Ev
// DEMANGLED: pvp_assault::CCandidateMatrix::~CCandidateMatrix()
CCandidateMatrix::~CCandidateMatrix() {}

// IDA_EA: 0x82E5962
// IDA_NAME: _ZN11pvp_assault12CExtraMatrix8AddPointENS_7Point2dE
// DEMANGLED: pvp_assault::CExtraMatrix::AddPoint(pvp_assault::Point2d)
void CExtraMatrix::AddPoint(Point2d point) {
    push_back(point);
}

// IDA_EA: 0x82E597C
// IDA_NAME: _ZN11pvp_assault12CExtraMatrix9MovePointERKNS_7Point2dE
// DEMANGLED: pvp_assault::CExtraMatrix::MovePoint(pvp_assault::Point2d const&)
void CExtraMatrix::MovePoint(const Point2d& point) {
    for (iterator it = begin(); it != end(); ++it) {
        *it += point;
    }
}

// IDA_EA: 0x82E5B4E
// IDA_NAME: _ZN11pvp_assault12CExtraMatrix10CheckPointEPSt6vectorI7MapAreaSaIS2_EERNS_7Point2dE
// DEMANGLED: pvp_assault::CExtraMatrix::CheckPoint(std::vector<MapArea,std::allocator<MapArea>> *,pvp_assault::Point2d &)
bool CExtraMatrix::CheckPoint(std::vector<MapArea>* map_areas,
                              Point2d& moved_point) {
    for (iterator it = begin(); it != end(); ++it) {
        if (pvp_assault::IsIncludeVirtualMoveAreas(
                map_areas, it->x, it->y,
                moved_point.x, moved_point.y, 0)) {
            return true;
        }
    }
    return false;
}

// IDA_EA: 0x82E5C0A
// IDA_NAME: _ZN11pvp_assault12CExtraMatrix8MoveUserEPNS_20CForcedMoveCharacterE
// DEMANGLED: pvp_assault::CExtraMatrix::MoveUser(pvp_assault::CForcedMoveCharacter *)
bool CExtraMatrix::MoveUser(CForcedMoveCharacter* character) {
    if (character->point_index < 0 ||
        static_cast<unsigned int>(character->point_index) >= size()) {
        return false;
    }
    const Point2d& point = at(character->point_index);
    character->pos_x = point.x;
    character->pos_y = point.y;
    character->ForcedMove();
    return true;
}

// IDA_EA: 0x82F1088
// IDA_NAME: _ZN11pvp_assault12CExtraMatrixC2Ev
// DEMANGLED: pvp_assault::CExtraMatrix::CExtraMatrix(void)
CExtraMatrix::CExtraMatrix()
    : std::vector<Point2d>() {}

// IDA_EA: 0x82F109C
// IDA_NAME: _ZN11pvp_assault12CExtraMatrixD2Ev
// DEMANGLED: pvp_assault::CExtraMatrix::~CExtraMatrix()
CExtraMatrix::~CExtraMatrix() {}

// IDA_EA: 0x82F1052
// IDA_NAME: _ZN11pvp_assault12CDispositionC2Ev
// DEMANGLED: pvp_assault::CDisposition::CDisposition(void)
CDisposition::CDisposition() {}

// IDA_EA: 0x82E6032
// IDA_NAME: _ZN11pvp_assault19CDynamicDispositionC2EPP5CUserPi
// DEMANGLED: pvp_assault::CDynamicDisposition::CDynamicDisposition(CUser **,int *)
CDynamicDisposition::CDynamicDisposition(CUser** source_users, int*)
    : CDisposition(),
      flag_point(),
      candidate_matrices() {
    users[0] = source_users[0];
    users[1] = source_users[1];
    flag_point.x = users[0]->get_posX() + (users[1]->get_posX() >> 1);
    flag_point.y = users[0]->get_posY() + (users[1]->get_posY() >> 1);
    candidate_matrices[0].MakeCandidatePoint(
        Point2d(users[0]->get_posX(), users[0]->get_posY()),
        Point2d(users[1]->get_posX(), users[1]->get_posY()));
    candidate_matrices[1].MakeCandidatePoint(
        Point2d(users[1]->get_posX(), users[1]->get_posY()),
        Point2d(users[0]->get_posX(), users[0]->get_posY()));
}

// IDA_EA: 0x82E624C
// IDA_NAME: _ZN11pvp_assault19CDynamicDisposition13SetVirtualMapEPSt6vectorI7MapAreaSaIS2_EE
// DEMANGLED: pvp_assault::CDynamicDisposition::SetVirtualMap(std::vector<MapArea,std::allocator<MapArea>> *)
int CDynamicDisposition::SetVirtualMap(std::vector<MapArea>* map_areas) {
    candidate_matrices[0].RemoveCandidatePoint(map_areas);
    candidate_matrices[1].RemoveCandidatePoint(map_areas);
    candidate_matrices[0].ArrangeCandidatePoint();
    candidate_matrices[1].ArrangeCandidatePoint();
    return 1;
}

// IDA_EA: 0x82E62A0
// IDA_NAME: _ZN11pvp_assault19CDynamicDisposition8MoveUserEPNS_20CForcedMoveCharacterE
// DEMANGLED: pvp_assault::CDynamicDisposition::MoveUser(pvp_assault::CForcedMoveCharacter *)
int CDynamicDisposition::MoveUser(CForcedMoveCharacter*) {
    return 1;
}

// IDA_EA: 0x82F050A
// IDA_NAME: _ZN11pvp_assault19CDynamicDisposition12GetFlagPointERiS1_
// DEMANGLED: pvp_assault::CDynamicDisposition::GetFlagPoint(int &,int &)
void CDynamicDisposition::GetFlagPoint(int& pos_x, int& pos_y) {
    pos_x = flag_point.x;
    pos_y = flag_point.y;
}

// IDA_EA: 0x82E62AA
// IDA_NAME: _ZN11pvp_assault18CStaticDispositionC2EPP5CUserPi
// DEMANGLED: pvp_assault::CStaticDisposition::CStaticDisposition(CUser **,int *)
CStaticDisposition::CStaticDisposition(CUser** users, int* team_counts)
    : CDisposition(),
      reserved_users(),
      flag_point(),
      extra_matrices() {
    flag_point.x = kAssaultRectIconPointX;
    flag_point.y = kAssaultRectIconPointY;
    for (int team = 0; team <= 1; ++team) {
        for (int member = 0;
             member < team_counts[team] && member <= 3;
            ++member) {
            extra_matrices[team].AddPoint(Point2d(
                kAssaultRectPlayerPoint[8 * team + 2 * member],
                kAssaultRectPlayerPoint[8 * team + 2 * member + 1]));
        }
    }
    _MoveMatrix(Point2d(users[0]->get_posX(), users[0]->get_posY()));
}

// IDA_EA: 0x82E6498
// IDA_NAME: _ZN11pvp_assault18CStaticDisposition11_MoveMatrixERKNS_7Point2dE
// DEMANGLED: pvp_assault::CStaticDisposition::_MoveMatrix(pvp_assault::Point2d const&)
void CStaticDisposition::_MoveMatrix(const Point2d& delta) {
    extra_matrices[0].MovePoint(delta);
    extra_matrices[1].MovePoint(delta);
    flag_point += delta;
}

// IDA_EA: 0x82E64F2
// IDA_NAME: _ZN11pvp_assault18CStaticDisposition12_CheckMatrixEPSt6vectorI7MapAreaSaIS2_EERNS_7Point2dE
// DEMANGLED: pvp_assault::CStaticDisposition::_CheckMatrix(std::vector<MapArea,std::allocator<MapArea>> *,pvp_assault::Point2d &)
bool CStaticDisposition::_CheckMatrix(std::vector<MapArea>* map_areas,
                                      Point2d& moved_point) {
    for (int team = 0; team <= 1; ++team) {
        if (!extra_matrices[team].CheckPoint(map_areas, moved_point)) {
            return false;
        }
    }
    return pvp_assault::IsIncludeVirtualMoveAreas(
        map_areas, flag_point.x, flag_point.y,
        moved_point.x, moved_point.y, 0);
}

// IDA_EA: 0x82E6588
// IDA_NAME: _ZN11pvp_assault18CStaticDisposition13SetVirtualMapEPSt6vectorI7MapAreaSaIS2_EE
// DEMANGLED: pvp_assault::CStaticDisposition::SetVirtualMap(std::vector<MapArea,std::allocator<MapArea>> *)
int CStaticDisposition::SetVirtualMap(std::vector<MapArea>* map_areas) {
    Point2d moved_point;
    for (int attempt = 0; attempt <= 4; ++attempt) {
        if (_CheckMatrix(map_areas, moved_point)) {
            return 1;
        }
        _MoveMatrix(moved_point);
    }
    return 0;
}

// IDA_EA: 0x82E65EE
// IDA_NAME: _ZN11pvp_assault18CStaticDisposition8MoveUserEPNS_20CForcedMoveCharacterE
// DEMANGLED: pvp_assault::CStaticDisposition::MoveUser(pvp_assault::CForcedMoveCharacter *)
int CStaticDisposition::MoveUser(CForcedMoveCharacter* character) {
    if (character->team < 0 || character->team > 1) {
        return 0;
    }
    return extra_matrices[character->team].MoveUser(character) ? 1 : 0;
}

// IDA_EA: 0x82F0526
// IDA_NAME: _ZN11pvp_assault18CStaticDisposition12GetFlagPointERiS1_
// DEMANGLED: pvp_assault::CStaticDisposition::GetFlagPoint(int &,int &)
void CStaticDisposition::GetFlagPoint(int& pos_x, int& pos_y) {
    pos_x = flag_point.x;
    pos_y = flag_point.y;
}

// IDA_EA: 0x82F10B0
// IDA_NAME: _ZN11pvp_assault18CStaticDispositionD2Ev
// DEMANGLED: pvp_assault::CStaticDisposition::~CStaticDisposition()
CStaticDisposition::~CStaticDisposition() {}

}  // namespace pvp_assault
