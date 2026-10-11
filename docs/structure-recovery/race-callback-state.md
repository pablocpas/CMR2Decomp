# Stage/replay/view callback machine

The first argument of Race_HandleStageReplayViewTransitions (original
0x0041e8d0) is CallbackStateMachine, rather than an untyped BYTE buffer.
The first byte is its count and the per-player packed callback records
are obtained from its records member. Calls to the promotion/teardown
helpers now carry that type directly.

The MSVC6 preservation source retains the original reuse of the parameter
slot for the resume timer out-pointer; OpenCMR2 uses a separate native
pointer for that local to make provenance explicit. The timer arithmetic
still wraps modulo 32 bits.

Validation: after the type recovery, the complete rebuilt .text section
is byte-identical to the preceding build, not just the recovered function.
.text SHA-256: `4490f208016e345efdf138a5b82d3af3337aa00b8cf21193ccb1544c862ae1ba`.
The differential_race_handler fixture passes all 11003 cases: every entry
of the five jump tables, real phase/game/rally/timer queries, guarded input
and global state, and complete traces across controlled action boundaries.

This is source type recovery; no new original-game behavior is introduced.
