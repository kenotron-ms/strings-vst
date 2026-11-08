# Architecture — 0.2 research engine

`src/dsp/Engine.*` handles performance/allocation; `PhysicalModel.*` implements
the dependency-free C++17 instrument. `Plugin.cpp` adapts it to DPF and `UI.cpp`
draws the native editor. `Parameters.hpp` is the shared parameter schema.

## Signal path

```
MIDI / automation -> note ledger + fixed-player divisi allocator
  -> each player: nonlinear bow contact <-> stopped string, 2 polarizations
                    3 open strings     <-> reciprocal passive bridge/body
  -> stereo placement -> shared room -> gain -> soft safety saturation
```

Each string lane has nut, outgoing-bridge and returning-bridge delay paths.
Fractional interpolation, a lossy nut reflection and an allpass approximate
propagation, damping and dispersion. Pitch tuning compensates those filters
and the loaded body's reflection phase; it is not a stiff-string PDE solver.
Retuning coefficients are cached while pitch/contact/body settings are constant.

Eight trapezoidal mass/spring/damper modes form a symmetric 2x2 admittance.
Positive outer-product modal participation and implicit string loading produce
reciprocal bridge motion. Sympathetic amount is a two-way transformer ratio;
zero disconnects the other strings. Body profiles are synthetic, not measured.

The bow junction solves static sticking or a bounded kinetic friction root.
Its kinetic curve adopts Desvages/Bilbao Eq. 17a, with heuristic rosin/heating.
Signed velocity and engagement are independent of pitch and output amplitude.
Pizzicato uses a short force pulse. Tremolo cyclically reverses signed motion.
The body observation mixes weighted modal velocity and bridge force; there is
no surrounding-air pressure solver. See the research ledger for exact fidelity.

## Real-time and performance ownership

Delay storage is allocated at prepare/sample-rate changes, never in render.
The engine uses at most 16 voices and 128 held-note ledger entries, fixed MIDI
arrays and no audio-thread locks, filesystem or network calls. Continuous
controls are smoothed. Structural changes reassign players. MIDI is applied at
its sample offset. There is no lookahead or declared plugin latency. These
properties do not guarantee meeting every host/device buffer deadline.

Solo is last-note priority. Ensemble assigns a fixed player count round-robin
to recent held notes; surplus notes wait for a player. Sustain is per MIDI
channel. Host state saves input parameters, not transient notes/delay buffers.
New physical parameters append after the existing output IDs to preserve the
original parameter indices. CC16–28 are global expression/configuration edits,
not MPE. Panic clears voices and the room when no voices remain.
MIDI parameter edits request host/UI notifications through DPF. Hosts can ignore
those requests: Pedalboard's controller readback remained cached in this run.
Record the actual CC stream; do not assume MIDI edits become written host
automation or that a displayed host parameter proves the engine's current CC state.

Native pointer gestures update host-visible controls outside the audio thread.
Browser studies produce optional MIDI through a user-armed output; a DAW/virtual
port forwards it to the same engine. They do not embed a second audio engine.
The external route can be recorded as MIDI; native UI edits use host automation
according to the DAW's write mode. Neither automatically records itself.

## Limits

No measured calibration, finite bow width/hairs, distributed heat, torsion,
finger collisions, directivity, learned performer or trained friction. Two
transverse lanes do not reproduce the full published FDTD contact model.
Component energy tests do not prove global stability under moving boundaries.
MPE zones/per-note timbre are not implemented. MIDI is not constrained to
physically playable registers. Very extreme settings can sound non-acoustic.
