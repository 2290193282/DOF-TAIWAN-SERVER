#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"
make clean all
if [[ -f ../artifacts/channel_0828/channel_info_0828_authoritative.etc ]]; then
    make channel_daemon_integration_test
    make test CHANNEL_INFO=../artifacts/channel_0828/channel_info_0828_authoritative.etc
else
    ./channel_core_test
    echo "WARN: authoritative channel table missing; daemon integration test skipped" >&2
fi
