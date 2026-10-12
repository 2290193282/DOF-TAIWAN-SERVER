#!/bin/sh
set -eu
build=/root/oldsrv_rebuild_full_0828_native_pvf0828_fresh_20260828_222433_2268
runtime=/home/neople/game/df_game_r
previous=fa72a47646829502b9e63e2fa7b487fca0774fee65d65a51b0bba3e2176e5230
backup=/home/neople/game/.codex_backups/six_regressions_20260910/df_game_r
cd "$build"
test "$(pwd -P)" = "$build"
test -f df_game_r
test ! -L df_game_r
test ! -e df_game_r.release.six_new
strip --strip-debug --strip-unneeded -o df_game_r.release.six_new df_game_r
file df_game_r.release.six_new
readelf -h df_game_r.release.six_new | grep 'ELF32'
LD_LIBRARY_PATH=/home/neople/game/lib:/home/neople/game/lib/mysql32:/home/neople/game ldd df_game_r.release.six_new > /tmp/codex_six_ldd.log
if grep -q 'not found' /tmp/codex_six_ldd.log; then
    cat /tmp/codex_six_ldd.log
    exit 1
fi
strings df_game_r.release.six_new | grep -F '[GUILD_EXP_BOOK]'
strings df_game_r.release.six_new | grep -F '[CHRONICLE_ENCHANT][RESULT]'
strings df_game_r.release.six_new | grep -F '[SPECIAL_TUTORIAL][NORMALIZE]'
strings df_game_r.release.six_new | grep -F '[CARD_FLOW][GENERATED]'
strings df_game_r.release.six_new | grep -F '[BIND_CUBE][CHECK]'
candidate=$(sha256sum df_game_r.release.six_new | cut -d ' ' -f1)
if pgrep -x df_game_r >/dev/null; then
    printf 'REFUSING_INSTALL_GAME_IS_RUNNING\n'
    exit 1
fi
test "$(sha256sum "$runtime" | cut -d ' ' -f1)" = "$previous"
test ! -L "$runtime"
test ! -e "$backup"
test ! -e /home/neople/game/df_game_r.six_new
mkdir -p /home/neople/game/.codex_backups/six_regressions_20260910
cp -p "$runtime" "$backup"
test "$(sha256sum "$backup" | cut -d ' ' -f1)" = "$previous"
install -m 755 df_game_r.release.six_new /home/neople/game/df_game_r.six_new
test "$(sha256sum /home/neople/game/df_game_r.six_new | cut -d ' ' -f1)" = "$candidate"
if pgrep -x df_game_r >/dev/null; then
    printf 'REFUSING_INSTALL_GAME_STARTED_DURING_STAGING\n'
    exit 1
fi
mv -T /home/neople/game/df_game_r.six_new "$runtime"
mv -T df_game_r.release.six_new df_game_r.release
sha256sum df_game_r.release "$runtime" "$backup"
stat -c '%s %n' df_game_r.release "$runtime"
printf 'INSTALLED_GAME_LEFT_STOPPED\n'
