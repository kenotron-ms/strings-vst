# Local delivery audit — Strings 0.3.1

November 8, 2026. Scope is the unchanged approved condition in
`.amplifier/goals/research-driven-strings-vst.md`: a working, bounded physical
research instrument with five interactive alternatives and inspectable
evidence. It is **not** professional acoustic certification, implementation of
every paper, or a newly published release. This replaces the historical 0.3.0
delivery crosswalk for the current binary.

## Per-outcome ledger

| Required outcome | Result | Current authoritative evidence and boundary |
| --- | --- | --- |
| 1. Executable research physics and seven-paper checklist | PASS | `src/dsp/PhysicalModel.cpp` and `Engine.cpp`; `research-implementation.md` records verified sources, equations/sections, code, tests, fidelity and omissions for all seven. Four strings, two polarizations, nonlinear contact, reciprocal passive modal bridge/body, sympathy and lossy/dispersive propagation execute in the audio path. |
| 2. Playable native VST3 | PASS | `host.json`: four profiles at three rates, MIDI timing, tuning, articulations, ensemble, state; `host-automation.json`: 60 sustained-note force/contact/reversal/lift/vibrato comparisons with identical prefixes, nonzero audio differences, release and state recall. CTest covers allocation, divisi, legato, panic and extremes. Pluginval discovers the exact 0.3.1 bundle and opens its editor. |
| 3. Ordinary expressive inputs | PASS, adapter scope | Native keys/knobs and position-based pointer/touchpad; `native.json` captures actual audio/readback/cancellation. `ux-bridge.json` renders the five browser routes and synthetic standard gamepad through actual VST3 MIDI. `ux-interactions.json` checks recording, edits, neutral/deadzone, unknown mapping and disconnect. Physical hardware, live OS MIDI routing and advanced DualSense features remain unverified. |
| 4. Five Astra alternatives and common-task comparison | PASS | `ux-performance/index.html` has Bow, Mood, Phrase, Conductor and Material routes. `ux-performance/comparison.md` compares intimate tone, tension, phrase revision, ensemble shaping and unfamiliar sounds. Recommendation: Mood + Phrase, with Material as the deeper sound-design view. This is expert inspection, not musician testing. |
| 5. Inspectable validation and listening renders | PASS, stated numerical bounds | Four Release CTest suites and four ASan/UBSan suites; actual-host audio/automation/state, 12-point wider register audit, physics/contact/passivity/sympathy tests, allocation-free processing, pluginval strictness 5 with editor, browser/native input safety and measured offline render costs. Reports/logs are fingerprinted in the current manifest. Numerical validity is not perceptual realism. |
| 6. Usable, identified, candid delivery | PASS when archive check agrees | Linux VST3/CLAP/standalone and source archives identified below, screenshot/component catalogue, audition renders, five mockups and reproducible commands. `package-check.json` checks archive contents against current source and binaries and records a clean extracted-source build/test. No new push, tag, public upload or release is claimed. |

## Source-to-evidence audit

The seven cached primary PDFs were re-fingerprinted against the research
ledger. All seven match their recorded SHA-256 values. No source is counted
as reviewed merely from a search snippet. The papers themselves are linked,
not redistributed. `research-implementation.md` contains their source records,
relevant equations, method dispositions and specific deferrals:

- Maestre: waveguide/reciprocal passive coupling principle; no distributed
  thermal, finite-width or individual-hair simulation and no measured fit.
- Desvages/Bilbao: Equation 17a contact curve plus independent junction/root
  residual checks; not their full FDTD/collision scheme or energy theorem.
- Percival: physics/performer separation and independent ensemble states;
  authored open-loop recipes, not trained feedback or measured constants.
- Luan/Scavone: explicitly evaluated fixed-oscillator training domain and
  conditioning; neural surrogate deferred, not claimed as code in this build.
- Zheleznov et al.: nonlinear differentiable fitting evaluated/deferred;
  conventional linear modal integration is not relabelled as their SAV solver.
- Digital-waveguide review: representation, loss/dispersion and cost choices;
  measured implementation cost, not a general speed claim over all solvers.
- Chabassier et al.: reciprocal whole-instrument decomposition only;
  no piano hammer, soundboard/air mesh or knowledge of Pianoteq internals.

The Astra delegation record was checked in the local Codex transcript:
November 7, 2026, 08:58:29 UTC, model `gpt-6-astra`, agent
`01a1b53b-0fe3-7e01-b1e3-582646cb9ab1`. Its final result lists the five
interactive routes and changed files. Later integration/safety fixes are
separate from that design provenance. Private transcripts are not packaged.

## Current binary and artifacts

- VST3: `build/bin/Strings.vst3`, module SHA-256
  `17e5370ffe6bdc07484971c347c2b5896cee35254dc300b83630ac397cb91869`.
- Standalone: `build/bin/Strings`, SHA-256
  `2b96493c725508aad283c4e2f39938bd1f4a8dbd80ea62bc4de73886e90e0486`.
- Binary package: `dist/Strings-0.3.1-linux-x86_64.zip`.
- Source package: `dist/Strings-0.3.1-source.tar.gz`, including vendored DPF
  sources and licenses, not only a submodule pointer.
- Archive checksums: `dist/Strings-0.3.1-SHA256SUMS`.
- Current file/report inventory: `build/performance-validation/manifest.json`.
  Archive verification: `build/performance-validation/package-check.json`.
- Component catalogue: `ux-components/catalog.md`; native UI guide:
  `ux-components/native-performance.md`; actual screenshot storyboard:
  `ux-components/screenshots/standalone-storyboard.png`.
- Audition: `build/standalone-workflow-r2/harbour-run-strings.wav`, raw stems,
  MIDI, schedule, screenshots and `audio-audit.json`. The binary archive copies
  them to `Standalone trial/`; other representative renders are in
  `Audio examples/` and the validation evidence inventory.

## Actual standalone trial, not a generated product screenshot

`Harbour Run` is an **original** 16-bar 6/8 pirate-adventure cue, not the
requested Pirates of the Caribbean theme. That exact-melody follow-up remains
unfulfilled without the user's musical material; do not present this cue as
its arrangement. The standalone sibling uses the VST3's engine/UI but is not
itself a VST3 host; the actual VST3 is tested independently.

The initial eight-player/1024-frame run failed with JACK deadline errors.
The preserved failed session is not hidden or treated as clean audio. A fresh
four-player/2048-frame run captured baseline violin, shaped violin, viola,
cello and bass from the real application's outputs, using actual X11 key and
knob events. Its complete log has zero processing-error lines. Independent
checks verify finite/nonclipped audio, silent starts/releases, the constant-gain
four-stem mix and 14 accepted long-note baseline pitch estimates. The 26-second
master peaks at 0.79996. Maximum UI-event lateness is 52.8 ms: this is not a
sample-accurate or low-latency device benchmark.

See `ux-components/standalone-trial.md` for commands and detailed limits.
No claim is made that numerical checks replaced a human listening session.

## Remaining limits are not hidden implementations

Body/string profiles are synthetic; measured bridge admittance/radiativity and
musician comparisons remain necessary for a realism claim. The wider register
audit measures strong-bow violin MIDI 88 about 19.38 cents sharp, within its
exploratory 25-cent bound, not professional intonation acceptance.

Offline eight-second renders measure about 28.7–30.0x real time for solos,
4.68x for eight players and 2.35x for sixteen on the recorded four-vCPU
environment. File writing, release and silence are included; live deadlines
and worst-case polyphony are not proven. DPF lifecycle assertions/warnings
are retained even though pluginval passes. No new Windows/macOS builds,
universal DAW certification or separate Steinberg-validator result is implied.

The local phrase is eight seconds/eight notes, UI-idle scheduled, not the
performed 23-second score or a host-synchronized DAW clip. Scratch bow takes
are editor-local; Tone/Vibrato automation belongs in the DAW. Arbitrary phrase
import, persistent multilanes, native WAV export, trained performers, raw
controller touch, haptics and adaptive triggers are unfinished production work.
The new image-generation brief is pending API-fallback permission; actual
native captures and the existing generated concepts are clearly distinguished.

The trial's JACK/Xvfb/recorder children are terminated by its cleanup path.
No temporary listening service is handed over. Credentials were not opened or
copied for this native implementation, validation or packaging.

## Reproduce this delivery

Build/run/test instructions and Python/browser dependencies are in
`research-validation.md` and `ux-components/native-performance.md`.
After completing the named validation probes and standalone audit:

```sh
python tests/package_guard_tests.py
python tests/delivery_manifest_tests.py
python tools/delivery_manifest.py
python tools/package.py linux-x86_64 --source
sha256sum dist/Strings-0.3.1-linux-x86_64.zip dist/Strings-0.3.1-source.tar.gz
```

The manifest tool validates named report coverage, binary identity and file
freshness before inventorying current artifacts. It does not rerun tests or
establish perceptual quality. Packaging verifies every inventoried file and
refuses stale evidence. The source exporter excludes Git metadata, environment
files and unsafe symlinks; DPF's legitimate internal platform-source link is
materialized as source. The archive audit is separate to avoid self-referential
archive hashes. A source rebuild should use a new build directory, not mutate
the certified binary in place.
