# Research build: reproduce, audition, inspect

For the current **local 0.3.1** native knobs, local expression takes and
current validation paths, start with `ux-components/native-performance.md`.
Commands for engine/browser validation below remain applicable; old native UI
coordinates, screenshots, binary hashes and 0.2 package results are historical.
The current per-outcome audit is [delivery-0.3.1.md](delivery-0.3.1.md), with a
clean actual-standalone recording documented in
[standalone-trial.md](ux-components/standalone-trial.md).

This guide concerns the **local Linux x86-64 0.2.0 working tree**, not the public
0.1.0 release. New Windows/macOS binaries have not been built or validated in
this iteration. The existing cross-platform workflow remains scaffolding.
No commercial-realism or universal real-time compatibility claim is made.

## Build and play

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The source archive includes the pinned DPF source and assets. When building an
extracted archive rather than a git checkout, skip the `git submodule` command.

Use the complete `build/bin/Strings.vst3` bundle in a VST3-capable host, or run
`build/bin/Strings` with an audio backend available. The CLAP is also built,
but the actual plugin-host tests here target VST3. Native gesture mode is an
addition to the existing editor, not the full proposed listening-first redesign.

Select an instrument and Solo or Ensemble. A MIDI note supplies pitch. The
latest local native editor has a direct expression pad: held X position sets
signed bow speed, Y assigns weight, and center rests. A stationary held pointer
maintains its command. Release/focus loss lifts the bow; disable direct mode
to restore the prior controls. See `ux-components/expression-pad.md` for the
new component, separate evidence and its limitations. Earlier packaged native
gesture captures used the previous relative-motion behavior.

See `input-devices.md` for the optional browser/gamepad-to-MIDI route. Its live
OS virtual-port / DAW connection and physical controllers remain unverified.

## Functional test commands

The engine tests require only the compiler. Optional Python checks used Python
3.13.14, NumPy 2.5.4, Pedalboard 0.9.26, and Playwright 1.64.1. A fresh environment:

```sh
python3 -m venv /tmp/strings-validation
/tmp/strings-validation/bin/pip install numpy==2.5.4 pedalboard==0.9.26 playwright==1.64.1
/tmp/strings-validation/bin/python -m playwright install chromium
/tmp/strings-validation/bin/python tests/host_smoke.py build/bin/Strings.vst3
/tmp/strings-validation/bin/python tests/research_audio.py
/tmp/strings-validation/bin/python tests/ux_bridge_smoke.py
/tmp/strings-validation/bin/python tests/ux_interaction_smoke.py
```

If deliberately using an existing compatible Chromium binary, set
`PLAYWRIGHT_CHROMIUM_EXECUTABLE` to its absolute executable path. This run used
headless-shell revision 1243 via that override; standard installation is the
portable default. Tests create temporary localhost servers and close them in
`finally` blocks; no web server needs to remain running.

Actual native pointer/audio capture on headless Linux additionally needs Xvfb,
xdotool, ImageMagick `import`, libX11, JACK dummy driver, `jack_lsp` and `jack_rec`:

```sh
/tmp/strings-validation/bin/python tests/native_gesture_smoke.py
xvfb-run -a python3 tools/validate_plugin.py build/bin/Strings.vst3
cmake -S . -B build-sanitize -DSTRINGS_BUILD_PLUGIN=OFF -DSTRINGS_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-sanitize --parallel
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-sanitize --output-on-failure
python3 tools/research_demos.py
python3 tools/package.py linux-x86_64
```

The native probe owns a uniquely named JACK dummy server and Xvfb display and
terminates its own processes. A large recorder ring buffer and 1024-frame dummy
period avoid interpreting recorder overflow as an instrument failure. Recorder
overruns invalidate that capture even if `jack_rec` returns zero. This is a
synthetic-input route check, not a low-latency audio-device benchmark.

## Evidence locations and scope

| Check | Inspectable evidence | What it establishes |
| --- | --- | --- |
| C++ engine | `ctest`, `tests/engine_tests.cpp` | Four instruments x three rates x three articulations, finite bounded output, releases, allocation-free processing, sustain/divisi, malformed events, parameter extremes, CC16–28 and panic |
| Physics | `tests/physical_tests.cpp` | Modal/bridge discrete power balance at three rates, dissipative friction, static bounds, sympathetic transfer, signed reversal, zero-force/stationary/lifted behavior |
| Actual VST3 | `build/host-smoke.json` | Pedalboard loads real binary; MIDI sample offsets, mid-register tuning, sound, articulations, stereo section and state recall |
| Broader audition | `build/research-validation/audio.json` and WAVs | Low/mid/high register samples, physical CC sequence, panic silence, new physical-control state and 1/8/16-player auditions |
| Plugin host stress | `build/pluginval-research-logs/` | Fresh pluginval 1.0.4 strictness-5 discovery, processing, automation/state and editor tests |
| Browser / MIDI bridge | `build/research-validation/ux-bridge.json`, MIDI JSON, six WAVs | All five actual UI routes plus a synthetic standard pad send expression into native VST3 offline; reversal, blur/disconnect release, no stale re-arm and mobile overflow checks |
| Composition interactions | `build/research-validation/ux-interactions.json` and exported takes | Actual take capture/replay/export, phrase edits, stationary pointer decay and unknown-gamepad rejection |
| Native UI / audio | `build/research-validation/native-pointer.json`, WAV, PNG and logs | Synthetic X11 keyboard and pointer events excite the standalone engine; focus loss releases sound through JACK dummy capture |
| Measured cost | `build/research-validation/performance.json` | Eight-second offline C++ DSP/WAV renders on this machine, not real device deadlines |

The tests analyze output; no musician listening panel or blind reference test
was conducted. A nonzero waveform or a green validator is not proof of violin
realism, controller comfort or matching a composition's emotional intent.

## Audition choices

- `build/demos/research/`: four instrument phrases, 8/16-player sections, pizzicato and
  tremolo from the same C++ library linked into the plugin.
- The package selects only files listed in the current performance report;
  older demo files elsewhere in `build/demos/` are not part of this delivery.
- `build/research-validation/`: actual VST3 solo/section/control renders and
  browser-generated expression traces, plus the native input capture.
- Auditions retain their actual gain. No loudness matching or mastering has
  been applied; this is not an A/B timbre study. Pizzicato is a brief force pulse
  with a quiet, fast-decaying response. Its test measures the attack window,
  not three seconds of mostly silence as though it were a sustained note.

## Known limits that must remain visible

1. Body modes/string impedances are synthetic. No measured admittance fit,
   radiativity calibration or acoustic ground truth is incorporated.
2. Strong bowing in the high violin register can sharpen pitch by approximately
   19 cents at 48 kHz. The middle-register host check uses 12 cents; the broader
   exploratory register test uses 25 cents. Passing that looser bound does not
   mean professional intonation. No claim covers every playable note/settings.
3. Component passivity tests do not prove global stability with moving pitch /
   contact boundaries. Extreme controls can sound non-acoustic.
4. Neither thermal diffusion/finite-width bow hairs nor full finger/contact
   collision physics from the cited papers is implemented. No neural model or
   trained virtual musician is present. See the seven-paper fidelity ledger.
5. Physical DualSense/Xbox, pressure trackpads, haptics, adaptive triggers and
   native device enumeration remain untested or proposed. The working gamepad
   adapter uses standard browser controls; advanced feedback is not shipped.
6. DPF/Pedalboard emits nonfatal lifecycle assertions/warnings in these offline
   tests. Parameter-change notifications are requested, but this host's controller
   readback can remain cached after MIDI CC edits. Test the intended DAW; record
   CC input rather than assuming automatic automation capture.
7. Offline render ratios include file writing or host lifecycle overhead as
   specified by each report. They are not maximum-polyphony or buffer-deadline
   promises. The 16-player benchmark includes release/silence in its eight seconds.
8. Five Astra browser studies are working alternatives, not five native editors.
   Their expert comparison is in `ux-performance/comparison.md`. The newer
   listening-first component plan and GPT Image briefs are in `ux-listening-first/`;
   no image-model output is delivered while image generation is unavailable.
