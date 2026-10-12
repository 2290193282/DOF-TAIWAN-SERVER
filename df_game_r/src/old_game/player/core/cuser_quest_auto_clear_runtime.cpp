#include <stdint.h>

#include "old_game/player/core/cuser.h"

// IDA_EA: 0x8692390
// IDA_NAME: _ZN5CUser14questAutoClearEi
// DEMANGLED: CUser::questAutoClear(int)
//
// Original i386 instructions are only `push ebp; mov ebp, esp; pop ebp; ret`.
// The sole caller immediately returns EAX.  EAX still contains `this` at the
// call site, so preserve that observable ABI result without inventing quest
// clearing behavior.
int CUser::questAutoClear(int /*level*/) {
    return static_cast<int>(reinterpret_cast<uintptr_t>(this));
}
