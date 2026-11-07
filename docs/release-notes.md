# Strings 0.1.0

First experimental release: a working sample-free physical-model string
instrument, not a claim of measured acoustic equivalence.

- Violin, viola, cello and double bass.
- Solo legato and 2–16-player fixed-size divisi ensemble.
- Arco, pizzicato, tremolo; twelve factory presets.
- Native resizable interface, playable keyboard, automatable bow and tone controls.
- MIDI velocity, pitch bend, expression, vibrato, sustain and channel pressure.
- VST3, CLAP and standalone builds for Windows x86-64, macOS universal, Linux x86-64.
- Includes five audio examples, installation instructions, source/license notices.
- Built and VST3-validated by GitHub Actions before publication.

Extract the archive for your OS and copy the complete `Strings.vst3` bundle to
your VST3 folder. See the included README for exact paths. macOS is ad-hoc
signed, not notarized; Windows is unsigned. Logic Pro requires an Audio Unit,
which is not included. Linux standalone requires desktop audio/OpenGL libraries.

Body resonances are hand-tuned. Torsion, sympathetic strings, measured body
coupling and full MPE configuration are not yet modeled. Expect a synthesizer
with a bowed-string character, not parity with mature commercial instruments.
