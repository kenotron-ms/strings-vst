# Strings 0.3.1 (research/performance build)

Experimental physical-modeling instrument with a keyboard-first native editor.

- Four strings per player, two transverse waveguide lanes per string, nonlinear
  stick/slip bow contact and reciprocal passive modal bridge/body coupling.
- Sympathetic strings, propagation loss/dispersion, continuous bow direction,
  force, speed, lift, contact, rosin and coupling controls.
- Energy/Tone/Vibrato performance knobs, editable local audition phrase and
  bow-weight curve, scratch expression capture/replay and Original/Take compare.
- Violin, viola, cello and double bass; solo legato and fixed-size divisi.
- Research-to-code ledger, five browser UX studies, reusable native components,
  expanded physics/interaction tests and an optional MIDI-to-strings render tool.
- Cross-platform CI builds VST3, CLAP and standalone; Mac binaries are universal
  Apple Silicon/Intel, deployment target macOS 11, ad-hoc signed, not notarized.

Read `mac-testing.md` for CI artifact downloads and installation. Logic Pro is
not supported: no Audio Unit is included. Local phrase/take tools are not host
transport, MIDI-clip import, or persistent projects. Controller expression uses
an external MIDI adapter; DualSense haptics are not implemented.

This is a reduced model with synthetic body profiles, not measured acoustic
equivalence or professional-instrument parity. Full bow-hair/thermal-contact
simulation, measured admittance fitting and neural performance models remain
future work. The copyrighted score/audio used in the local MuseScore experiment
is not included in source or binary downloads.

## Previous release: Strings 0.1.0

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
