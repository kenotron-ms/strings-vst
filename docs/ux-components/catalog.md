# Native component catalogue

November 8, 2026. Current live instrument view; all controls are native NanoVG
geometry/text. The synthesis remains the model in `docs/research-implementation.md`.

![Current live instrument](screenshots/live-instrument-native.png)

| Component | Responsibility | Implementation | Verification / boundary |
| --- | --- | --- | --- |
| Instrument header | Profile, Solo/Ensemble, presets, articulation | `UI.cpp`, `Presets.hpp` | Instrument/preset changes release UI notes. Arco/Pizzicato/Tremolo remain accessible without opening Details. |
| Energy | Pair bow force and speed | `PerformanceKnob.hpp`, `PerformanceViews.hpp`, `UI.cpp` | Native held-note audio and marker checks; an authored macro over physical parameters. |
| Tone | Pair contact point and brightness | Same view/model bridge | Native drag and marker check, no sample crossfade or timbre classifier. |
| Vibrato | Pitch-modulation depth | Same view/model bridge | Native drag/reset marker checks. |
| Knob lifecycle | Drag, Shift, scroll, double-click reset | `UI.cpp` | Release, Escape, focus loss, presets and teardown end host editing gestures. |
| Piano | 88 keys A0–C8, 52 white/36 black, deeper key travel area | `PlayableKeyboard.hpp`, `PlayableKeyboardView.hpp` | All key geometries map to absolute MIDI pitches; native click/glissando, accidentals, scaled input and safe releases. UI audition is not MIDI output to a host clip. |
| Computer keys | 13 chromatic notes, highlighted typing span, octave and velocity | Same model/view plus `UI.cpp` | A/W/S/E/D/F/T/G/Y/H/U/J/K, Z/X register; repeated key-down ignored, key-up retains original pitch. Roots are C4/C3/C2/E1 for violin/viola/cello/bass. |
| Bow view | Optional signed motion/weight/lift control | `ExpressionPad.hpp`, `ExpressionPadView.hpp`, `UI.cpp` | Native forward/reverse/readback and minimum-size tests. Pointer position is not sensed pressure. Knobs restores the pre-gesture baseline. |
| Details | Sixteen physical parameters | `UI.cpp` | Separate native screen, same live piano below. Host parameter state/automation remains unchanged. |
| Output/Release | Meter and local held-note release | `UI.cpp` | Focus-loss, Escape, pointer-release and explicit Release tests. Not a host-wide MIDI panic. |
| Engine | Bow contact, strings, reciprocal body, ensemble allocation | `dsp/PhysicalModel.*`, `dsp/Engine.*` | Core/physics suites; reduced-model and synthetic-profile limitations remain. |

## Removed from the live editor

The piano roll, note-editing hotspots, four-bar/eight-second plot, bow-weight
lane, local demo transport, capture/take/undo controls, input-map diagrams and
standalone/DAW help panels are removed. No hidden note editing or demo playback
remains in `UI.cpp`. Legacy phrase/take model code is retained as experimental
research/test support, not an active UI or composition feature.

Standalone plays live; the DAW owns arrangement, transport, recording and bounce.
Native standalone recording/project/export is not implemented. The external
`tools/standalone_workflow.py` trial is historical automation, not a native recorder;
its old UI coordinates do not target this layout.

## Validation and captures

The logical editor is 1040 × 680, minimum 832 × 544. Keyboard screenshots and
audio checks come from `tests/native_keyboard_smoke.py` in
`build/live-keyboard-validation/`. Knob and optional-bow captures/checks come from
`tests/native_expression_pad_smoke.py` in `build/live-expression-validation/`.
Both probes need Linux Xvfb, xdotool, ImageMagick, JACK tools and NumPy. They use
synthetic input, not a physical controller or recruited-user study. They do not
establish Mac/DAW compatibility. Release 0.3.2 adds cross-platform CI validation;
the release's build identity and logs identify the binaries actually checked.
