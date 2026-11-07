# Validation

Automated checks are functional/numerical evidence, not acoustic validation.

- Engine tests cover all four instruments, three articulations and 44.1/48/96
  kHz; finite bounded output, audible energy, release to silence, fixed-player
  chord allocation, sustain, solo note priority, panic, malformed MIDI,
  parameter sanitization and all twelve factory presets.
- An allocation counter checks MIDI/chord redistribution and audio processing
  without heap allocation after preparation. Extreme control settings are
  checked for finite, bounded output with all instruments.
- AddressSanitizer and UndefinedBehaviorSanitizer run against engine tests.
- pluginval 1.0.4, strictness 5, loads the built VST3, exercises processing,
  parameters/state and editor behavior on each release platform.
- CI renders five PCM stereo WAV examples from the same DSP library linked
  into the plugin. These examples are included in release archives.
- A separate Pedalboard host loads the actual VST3 binary and verifies sound
  for all instruments at three rates, tuning within 12 cents with vibrato off,
  MIDI onset timestamps, articulations, stereo ensemble and saved-state recall.
  Numeric measurements are retained as workflow artifacts.
- On Windows, Pedalboard is given the inner VST3 module path because of its
  [bundle-path limitation](https://github.com/spotify/pedalboard/issues/433).
  pluginval independently validates the entire installable bundle on Windows.
- A successful exit alone is insufficient: CI also requires a fresh pluginval
  report proving exactly one discovered plugin and final `SUCCESS`.
- SHA-256 checksums cover the final binary archives.

Run `ctest --test-dir build -C Release --output-on-failure` locally.
For plugin validation: `xvfb-run -a python3 tools/validate_plugin.py
build/bin/Strings.vst3` on headless Linux, or omit `xvfb-run -a` on a desktop.

Limits: pluginval does not prove every DAW works, output sounds like a recorded
instrument, low latency on every machine, or correct rendering on every GPU.
No blind listening study or real-instrument measurement fit has been performed.
