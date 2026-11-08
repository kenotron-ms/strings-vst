# Research iteration evidence ledger

**Historical 0.2 snapshot.** The current 0.3.1 build uses native knob-led
performance components and local expression takes. Its usage/validation status
is in `ux-components/native-performance.md`; its component inventory and trial
are in `ux-components/catalog.md` and `ux-components/standalone-trial.md`.
The current delivery audit is `delivery-0.3.1.md`; current fingerprints and
test inventory are in `build/performance-validation/manifest.json`. None of the
old package hashes below identifies the new worktree binary.

Historical research/package snapshot: a later local native expression-pad
iteration changes the UI and appends three read-only monitor parameters.
Its evidence is separate in `ux-components/expression-pad.md` and
`build/expression-pad-validation/`. The module hash below identifies the older
research archive, not the newest worktree binary.

November 7, 2026. Scope: a reduced, independently implemented research engine,
Linux x86-64 native build, five Astra interaction studies and their documented
input adapter. **Not commercial-grade acoustic certification.** All new source
and binaries are local/uncommitted; no new tag, push or release was performed.
GitHub's latest published release was checked and remains v0.1.0, published
November 7 at 07:06:47 UTC. It does not contain this work.

## Required goal outcomes

| # | Status | Direct evidence / limitation |
| --- | --- | --- |
| 1. Executable research physics | PASS, bounded model | `src/dsp/PhysicalModel.*`, `docs/research-implementation.md`, seven verified primary PDFs identified by hash, and component energy/contact tests. Reciprocal bridge, two polarizations, four strings, stick/slip, loss and dispersion execute in the signal path. Synthetic coefficients and major omitted mechanisms are explicit. |
| 2. Playable instrument | PASS on Linux | Actual `build/bin/Strings.vst3`, pluginval 0.2.0 report, `build/host-smoke.json`, `build/research-validation/audio.json` and WAVs. Four profiles, solo/sections, physical controls, automation/state, releases and panic. High-register intonation remains a known flaw, not concealed by the pass. |
| 3. Ordinary expressive inputs | PASS for implemented routes | Native pointer/keyboard through standalone/JACK capture; five browser adapters and synthetic standard-pad traces through the actual VST3. Neutral capture, deadzone, safe release and disconnect/re-arm tests pass. No physical PS5/Xbox, pressure sensing, adaptive triggers, haptic feedback or live OS virtual-port/DAW routing is claimed tested. |
| 4. Five Astra UX alternatives | PASS | `docs/ux-performance/index.html`, five view routes, actual screenshots, exported controls, `design.md` and `comparison.md`. Same five composer tasks compared by expert inspection. Mood is the proposed front door, Phrase the revision workspace. No musician study or native implementation of all five is claimed. |
| 5. Inspectable validation | PASS within stated checks | Release CTest and ASan/UBSan, three-rate actual VST host checks, broader register/state/gesture auditions, pluginval strictness 5, browser interaction/safety tests, native capture and measured render cost. Exact reports/logs are included in Research validation. These do not establish acoustic truth or every DAW/device deadline. |
| 6. Usable, candid delivery | PASS for local delivery | Source in this checkout, identified 0.2.0 VST3, CLAP/standalone build, audition WAVs, five interactive studies/screenshots, comparison and reproducible guide. `tools/package.py` produces the Linux archive; source is also exported separately. `research-validation.md` names acoustic, platform, controller and model limits. |

VST3 module: `build/bin/Strings.vst3/Contents/x86_64-linux/Strings.so`.
SHA-256: `095e98148b0e94b5fef4339251e0fefb74faa8ccba2496ebf7fb11482e66faa3`.
The whole bundle, not only that module, is required for installation.

## Paper-by-paper disposition

| Paper | Disposition | Executable result or explicit deferral |
| --- | --- | --- |
| Maestre, Spa & Smith 2014 | SIMPLIFIED / IMPLEMENTED architectural subset | Reciprocal positive-modal bridge/scattering and traveling-wave strings; no measured fit, finite-width/hair or spatial thermal model. |
| Desvages & Bilbao 2016 | IMPLEMENTED curve / SIMPLIFIED system | Eq. 17a kinetic friction/static bound, two transverse lanes and component energy accounting; no full FDTD collision scheme or its global theorem. |
| Percival 2013 | IMPLEMENTED separation / SIMPLIFIED performer | Physics separated from authored expression, independent ensemble players and repeatable control takes. Open-loop mappings, not learned feedback; no imported measurement/IR dataset. |
| Luan & Scavone 2025 | EVALUATED / DEFERRED | Reviewed small-system PINN/operator demonstration; defer neural friction until training-domain, force robustness, error and dissipation can be validated against the numerical solver. |
| Zheleznov et al. 2026 | EVALUATED / DEFERRED | Reviewed stable differentiable nonlinear-modal approach. Current linear modal bridge is not its SAV/neural implementation; fitting awaits appropriate measurements/data. |
| Tablas de Paula et al. 2026 | IMPLEMENTED representation choices | Waveguides/scattering, low-order propagation filters and measured cost; not every fitting method in the review is implemented. |
| Chabassier, Chaigne & Joly 2014 | SIMPLIFIED whole-system principle | Reciprocal excitation/string/body signal flow; observation is an approximate mix, not a plate/air field or Pianoteq internals. |

Detailed sections, equations, source locations, tests, primary links and PDF
fingerprints are in `research-implementation.md`. No paper was counted as
reviewed solely from an abstract or a blocked source.

## What the measurements actually say

- Twelve mid-register actual-VST measurements cover four instruments at
  44.1/48/96 kHz, with worst absolute pitch error about 7.79 cents in this run.
- The wider 48-kHz low/mid/high register audit finds violin MIDI 88 about
  **19.38 cents sharp** under the tested strong-bow condition. Its exploratory
  25-cent bound is not professional intonation acceptance.
- Release offline DSP/WAV renders on the i7-8700K-labelled four-vCPU environment
  run about 29–30x real time solo, 4.7x with eight players and 2.4x with sixteen.
  They include release/silence and file writing; no live worst-case guarantee.
- Native synthetic pointer capture has a finite nonclipped signal and silent
  final tail after focus loss. Recorder overflow was detected in an earlier
  invalid run; a clean larger-buffer run replaced it, not a weakened silence test.
- Pizzicato's short attack is checked separately from a sustained tone. It is
  quiet/fast-decaying and not evidence of a calibrated plucked instrument.

## Latest listening-first / image-model follow-up

**DELIVERED locally after the original package snapshot.** The user authorized
the API fallback and use of their private credential file. The local launcher
loads only the needed key at execution time, leaves the original file private
and does not copy the credential into the repository. Redaction and permission
checks were tested with fake credentials; real output/prompts were checked
without displaying the key.

`docs/ux-listening-first/` now contains the component contracts, five GPT Image
concept states, one targeted Listen revision, an overview, prompts, generation
manifest and design review. Generation used the bundled CLI with `gpt-image-2`,
not a substitute provider or a hand-coded screenshot. All selected images were
visually inspected; axis/navigation and cross-screen consistency limits are
explicit in `review.md`. The new image-only delivery is separate from the five
earlier Astra interactive studies and from the earlier 0.2.0 build archives.

Native implementation of the proposed listening/compare/continuous editor
components and advanced DualSense feedback remains future work, not
functionality hidden behind these images. Numerical/acoustic limitations
remain listed. Temporary test browsers/servers, JACK and Xvfb processes were
torn down; no listening service is left running.
