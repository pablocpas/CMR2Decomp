#!/usr/bin/env python3
"""Native route limits with production Graphics and actual function bodies.

Checks whole objects, high pointers, both directions, inactive players, negative
limits and ten detail settings. The old Win32 stores must fail the model.
"""
from pathlib import Path
import subprocess
import tempfile
from check_64bit_car_contacts import body

ROOT = Path(__file__).resolve().parents[1]
PORT = ROOT.parent / "OpenCMR2"
PRE = r'''
#include "port/types.h"
#include "Graphics.h"
#include "GameInfo.h"
#include "RallyRouteNode.h"
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
Graphics *g_pGraphics;
double CGraphics::m_oneOver65536 = 1.0 / 65536.0;
double CGraphics::m_65536 = 65536.0;
static int players, detail, request, calls, distances[2], recipients[2];
static RallyRouteNode route[4];
BYTE RallyDataState() { return players; }
int RallyData_GetRouteAvailabilityState() { return 4; }
int Race_IsMultiplayerRecordMode10() { return 0; }
BYTE CGameInfo::IsConfiguredMultiplayer() { return players > 1; }
int CGameInfo::GetGraphicsOptionBits21To24() { return detail; }
RallyRouteNode *RallyData_GetAvailableRouteNodeRecord(int index) {
    request = index; assert(index >= 0 && index < 4); return &route[index];
}
void View_SetDistanceOverride(BYTE player, int distance) {
    assert(calls < 2); recipients[calls] = player; distances[calls++] = distance;
}
'''
MAIN = r'''
int main() {
    auto *storage = calloc(1, 2 * 1024 * 1024);
    assert(storage && (uintptr_t)storage > UINT32_MAX);
    g_pGraphics = (Graphics *)storage;
    assert(offsetof(Graphics, baseDrawDistance) != 0x3c4);
    const int limits[] = {-1, 0, 55 * 65536 + 37, 175 * 65536 + 19};
    const int farLimits[] = {-1, 0, 100 * 65536 + 3, 300 * 65536 + 7};
    unsigned cases = 0;
    for (players = 0; players < 4; players++) for (int player = 0; player < 4; player++)
    for (int direction = -1; direction <= 1; direction++) for (int near : limits)
    for (int far : farLimits) for (detail = 0; detail < 10; detail++) {
        memset(g_pGraphics, 0xa5, sizeof(Graphics));
        g_pGraphics->baseDrawDistance = 80 * 65536 + 11;
        g_pGraphics->maxDrawDistance = 200 * 65536 + 5;
        Graphics expected; memcpy(&expected, g_pGraphics, sizeof(expected));
        for (auto &node : route) {
            memset(&node, 0xa5, sizeof(node));
            node.forwardNearLimit = near; node.forwardFarLimit = far;
            // Distinct backward values catch choosing the wrong pair.
            node.backwardNearLimit = far; node.backwardFarLimit = near;
        }
        RallyRouteNode before[4]; memcpy(before, route, sizeof(route));
        int wanted[2], n = 0;
        if (player < players) {
            int selectedNear = direction < 0 ? far : near;
            int selectedFar = direction < 0 ? near : far;
            for (int pass = 0; pass < 2; pass++) {
                int limit = pass ? selectedNear : selectedFar;
                if (limit <= 0) continue;
                if (pass) expected.baseDrawDistance = limit;
                else expected.maxDrawDistance = limit;
                float base = float(expected.baseDrawDistance / 65536.0);
                float scaled = base + base * float((detail - 1) / 2.0);
                wanted[n++] = std::min(int(int64_t(double(scaled) * 65536)), expected.maxDrawDistance);
            }
        }
        calls = 0; request = -1;
        unsigned node = cases % 4;
        StageTiming_SetViewRouteDistanceLimit(player, node, direction);
        assert(!memcmp(&expected, g_pGraphics, sizeof(expected)));
        assert(!memcmp(before, route, sizeof(route)) && calls == n);
        assert(request == (player < players ? int(direction < 0 ? std::min(node + 1, 3u) : node) : -1));
        for (int i = 0; i < n; i++) assert(recipients[i] == player && distances[i] == wanted[i]);
        cases++;
    }
    free(storage);
    printf("PASS %u native route/detail cases: whole Graphics, route records, distances and high pointers\n", cases);
}
'''


def main():
    scale = body((ROOT / "CMR2Decomp/Car.cpp").read_text(), "Render_GetDetailDistanceScale")
    limits = body((ROOT / "CMR2Decomp/StageTiming.cpp").read_text(), "StageTiming_SetViewRouteDistanceLimit")
    code = PRE + scale + limits + MAIN
    old_limits = limits.replace("g_pGraphics->maxDrawDistance", "*(int *)((BYTE *)g_pGraphics + 0x3c8)")
    old_limits = old_limits.replace("g_pGraphics->baseDrawDistance = nearDist", "*(int *)((BYTE *)g_pGraphics + 0x3c4) = nearDist")
    old = PRE + scale + old_limits + MAIN
    with tempfile.TemporaryDirectory(prefix="cmr2-distance-native-") as temp:
        folder = Path(temp)
        for name, source in (("old-offsets", old), ("typed-fields", code)):
            cpp, exe = folder / (name + ".cpp"), folder / name
            cpp.write_text(source)
            subprocess.run(["clang++", "-std=c++17", "-O2", "-g", "-fsanitize=address,undefined",
                            "-fno-sanitize-recover=all", "-fms-extensions", "-Wno-everything",
                            "-I" + str(PORT / "src"), "-I" + str(PORT / "src/port/compat"),
                            "-I" + str(PORT / "game"), str(cpp), "-o", str(exe)], check=True)
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            print(name, result.returncode, result.stdout, result.stderr[-700:])
            assert (result.returncode == 0) == (name == "typed-fields")


if __name__ == "__main__":
    main()
