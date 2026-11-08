# String input: performance without learning to bow

Research and design response, November 8, 2026. The target is a composer or
sound designer who knows a DAW, not necessarily a violinist. Commercial
documentation establishes available interactions, not comparative realism or
proof that our instrument already supports them. NIME studies below describe
prototypes and preliminary evaluations, not large controlled usability trials.

## Immediate visual correction

The filled blob in the previous native screen was decoration. Its contour was
not a string displacement, force boundary, instrument body, trained timbre map
or stable-bowing region. It is removed from `ExpressionPadView.hpp`.
The remaining dot and fading trail show smoothed command history; the ring is
the pointer target. Those have a purpose but still do not measure acoustic
state. This cleanup does not make mouse-bowing the chosen default workflow.

Retain the existing ivory/pine palette and type. Spend visual emphasis on
notes, editable phrasing and audible changes, not idle ornament. Do not replace
the blob with a pulsing effect unrelated to the model.

## Ableton reference for the bottom section

Use Live's **MIDI Note Editor and Note Expression lanes**, rather than music
notation or an unlabeled sparkline. The manual illustrates notes arranged by
pitch/time, selected-note expression, editable breakpoints, and show/hide lanes.
Pitch expression overlays notes; other dimensions have separate lanes. This
is the relevant interaction reference, not a requirement to copy Live's skin.

An official manual illustration was saved locally for review at
`build/ux-reference/ableton-note-expression.png`. It is an Ableton reference,
not our plugin screenshot or a production asset. Do not package it as our UI.

Suggested original component:

```text
Phrase                         bars / beats             loop range
Pitch   |       [note]-----[note]     [note]--------
        | [note]                      playhead
Energy  |         ____/------------\____
Tone    |         ----/------\----------
        |       show: bow changes / vibrato / lift
```

The design target has two visible lanes and discloses the rest. The current
0.3.1 native draft implements a smaller, explicit subset: an editable eight-note
local piano roll and one bow-weight lane, with single-action swap Undo. Tone
and Vibrato remain live/DAW-automatable controls, not editable scratch-take lanes.
It is not a full MIDI editor or learned phrase performer.
The host owns its MIDI clip: a normal plugin event stream
does not automatically give us the entire DAW piano roll. A plugin-owned phrase
or explicit MIDI import is a different implementation from a read-only view
of incoming notes; choose and label that boundary before building the editor.

Primary reference: [Ableton Live 12, Editing MPE](https://www.ableton.com/en/manual/editing-mpe/),
especially Sections 12.1–12.3. Also
[Editing MIDI](https://www.ableton.com/en/manual/editing-midi/).

## Relevant projects and studies

### SWAM Solo Strings: one instrument, different gesture interpretations

The v3.0.3 manual documents ordinary MIDI controllers and three bow gesture
interpretations: Expression, Bipolar speed, and Bowing displacement. Its virtual
XY controller is another input, not the only way to play. MIDI mapping curves
allow sensitivity adjustment. This is a direct precedent for separating the
physical engine from the control strategy.

Design inference: offer an assisted expression mode first, with signed/manual
bow control as an explicit option. Do not silently switch between position,
velocity and sustained-speed mappings: they have different musical meanings.

Source: [Audio Modeling, SWAM Solo Strings manual v3.0.3](https://static.audiomodeling.com/manuals/strings/SWAM%20Solo%20Strings%20-%20User%20Manual%20-%20v3.0.3.pdf),
Play Modes / Right Hand and MIDI Mapping. This is a versioned primary manual,
not a claim that every detail is unchanged in the newest release.

### Soliste: expressive keyboard and separate gesture controller

Expressive E's Soliste covers violin, viola, cello and double bass using physical
models. Its manufacturer documents MPE and conventional MIDI input, multiple
performance modes, and dedicated Touché presets. Osmose/Touché are useful
examples of keeping pitches familiar while adding continuous gesture control.

Design inference: pressure or a wheel/pedal can describe musical effort without
making the player trace literal bow travel. Keep controller-specific presets
and response calibration, rather than one universal gesture layout.

Source: [Expressive E, Soliste](https://www.expressivee.com/139-soliste).
Manufacturer claims of expressivity are not independent listening evidence.

### GeoShred / GeoSWAM: touch expression and note-transition rules

moForte's GeoSWAM documentation separates continuous dynamic expression from
note-on velocity. It uses overlapping notes and velocity thresholds to select
legato/portamento transitions. Expression can come from a separate pad or
finger gestures; supported pressure depends on the actual device.

Design inference: a useful instrument can interpret note relationships instead
of demanding continuous manual bow simulation. Preserve the original notes and
let the player adjust transition behavior; do not label authored rules as AI.

Source: [moForte, GeoSWAM controls](https://www.moforte.com/geoShredAssets7000/help/effects/swam.html).

### Cymatic, NIME 2003: gaming peripherals driving physical excitation

Howard, Rimell and Hunt's *Force Feedback Gesture Controlled Physical Modelling
Synthesis* uses a Sidewinder force-feedback joystick and iFeel mouse. For its
bowed-string example, force and motion have separate mappings; joystick twist
and throttle provide other dimensions. It is a particularly close precedent
for affordable non-instrument hardware controlling physical synthesis.

Design inference: a gamepad is a credible experimental input, but this paper
does not validate our PS5/Xbox mapping. Its motorized joystick and tactile
mouse are not interchangeable with ordinary gamepad rumble or trigger effects.

Source: [NIME 2003 primary paper, pp. 95–98](https://www.nime.org/proceedings/2003/nime2003_095.pdf),
especially Section 3 and Figure 5.

### Young and Serafin, NIME 2003: realism of the control matters too

*Playability Evaluation of a Virtual Bowed String Instrument* drives a physical
model from a sensor bow. Its discussion identifies tactile/ergonomic limitations
of tablet control and reports preliminary bow-controller experiments. It does
not establish an ordinary mouse as an equivalent replacement for a bow.

Design inference: matching parameter names is not enough to make an input feel
natural. For non-violinists, assisted phrasing may be a better design goal than
recreating all the work of learning a bowing technique.

Source: [NIME 2003 primary paper, pp. 104–108](https://www.nime.org/proceedings/2003/nime2003_104.pdf).

### Haptic bowing, NIME 2012: resistance and vibration are different

*Bowing a Vibration-Enhanced Force Feedback Device* augments a Phantom Omni
with a separate vibrating handle and studies their frequency responses. This
separates motion-opposing forces from higher-frequency vibration feedback.

Design inference: controller vibration might provide an informative cue, but
it is not evidence that a controller reproduces bow/string resistance. Treat
haptics as an optional independently tested layer, never necessary for sound.

Source: [IDMIL primary paper](https://www.idmil.org/wp-content/uploads/2022/06/NIME12_Giordano_37.pdf).

## Recommended input hierarchy — proposed, not delivered

1. **Keyboard first.** MIDI or computer-key notes remain the pitch source.
   An assisted performer generates continuous excitation and transitions from
   note timing, overlap and a small set of phrase controls. Separate the
   performer rules from the physical engine and make their behavior inspectable.
2. **One expressive control.** Wheel, expression pedal, channel pressure or a
   drawn Energy lane controls a bounded bow-force/speed recipe. A separate Tone
   lane controls contact/brightness. Their calibrated relationships need listening
   tests; Energy is not just output gain or an asserted physiological measurement.
3. **Optional controller pass.** After laying down notes, use a gamepad to
   record expression without playing pitches simultaneously. Trial triggers
   for effort/engagement and a stick for tone or vibrato. Signed bow motion
   belongs in an advanced mapping, not a mandatory default.
4. **Optional MPE performance.** Support per-note expression only after voice
   routing, pitch ranges, release semantics and host recording are validated.
   The current global bow controls must not be marketed as complete MPE support.
5. **Mouse for revision first.** Draw/reshape phrase lanes and audition the
   result. Keep the XY gesture as an optional performance experiment, not the
   largest compulsory control on every screen.

No device purchase should be necessary to get a coherent sustained phrase.
These are design recommendations derived from the precedents, not measured
rankings of controllers or a claim that all proposed features exist today.

## Small comparison before choosing the next default

Use the same MIDI phrase with keyboard-only assistance, wheel/pedal control,
gamepad overdub and mouse-lane editing. Ask composers to create an intimate
opening, build tension, then revise only the phrase peak. Measure time to first
acceptable take, revision time, accidental releases, perceived control and
listening preference. Log assistance settings and input-to-audio latency.
Only a real device test can establish controller feel; synthetic safety tests
cannot. Do not substitute a pretty input visualization for that evaluation.
