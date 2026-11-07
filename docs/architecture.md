# Architecture

`src/dsp/Engine.*` is dependency-free C++17. `Plugin.cpp` adapts it to DPF;
`UI.cpp` draws a native NanoVG interface and sends host-visible edits.
`Parameters.hpp` supplies one parameter schema for engine, host and UI.

## Signal path

```
MIDI -> held-note ledger -> fixed-player divisi allocator
                            -> player 1: nonlinear bow <-> two waveguides -> modal body
                            -> player 2: nonlinear bow <-> two waveguides -> modal body
                            -> ...
                         stereo placement -> shared room -> gain -> soft safety saturation
```

The bridge path is frequency-dependently damped and reflected; the finger/nut
path reflects with velocity inversion. Relative bow/string velocity controls
the friction-reflection coefficient. Total delay approximates `sampleRate / f`
with compensation for the string-loss filter's phase delay. Fractional linear
interpolation supports pitch bend/vibrato and note transitions. Eight body
bandpass resonators use distinct instrument scale factors and small persistent
per-player offsets. These are qualitative, not measured mechanical admittances.

Pizzicato replaces bow excitation with a decaying noise burst. Tremolo changes
bow velocity cyclically. After note-off, excitation and output decay smoothly.
All sound generation is deterministic for a given preparation and event stream.

## Real-time constraints

Delay storage is allocated on prepare/sample-rate change, not inside processing.
There are at most 16 voices and 128 held-note ledger entries. MIDI handling uses
fixed arrays; sorting the pointer list requires no heap allocation. Audio
processing performs no filesystem/network calls or locks. Continuous controls
are smoothed; structural changes reassign players. MIDI is processed at its
sample offset in the host block. No lookahead, and no declared plugin latency.

Solo is last-note priority. Ensemble assigns a fixed number of players round
robin to the most recently held notes. Held notes beyond the player count are
silent until a player becomes available. Release tails may briefly overlap
when switching from ensemble to solo. Sustain is tracked independently per
MIDI channel. Host parameters are serialized by DPF; transient held notes and
delay buffers intentionally are not saved in projects.

## Limits

No finite-element body mesh, stiff-string dispersion, torsion, bow-hair
compliance, two-way measured body coupling, open-string sympathy or radiation
directivity. Body modes are output resonators; the feedback termination is a
simplified bridge-loss approximation. Full MIDI Polyphonic Expression setup
(RPN pitch range/zones and per-note timbre) is not implemented. Instrument
presets do not constrain MIDI to physical playable ranges.
