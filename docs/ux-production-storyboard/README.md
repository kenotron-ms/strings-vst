# Compose with Strings: production-oriented storyboard

**Rejected visual study.** The user found the flat five-screen batch too
form-heavy: production feasibility had displaced the organic, analog-input
performance idea. Retain the DAW workflow and standard component constraints,
but do not use these five screen images as the selected visual direction.
`prompts/06-organic-performance.txt` records one corrective direction check:
gesture-led, controller-centered and still free of textured backgrounds.

November 7, 2026. Workflow study for a DAW-literate composer. The original
five-screen images are rejected visual alternatives; they do not supersede
the earlier organic performance direction. None of these concepts is a
screenshot of implemented native controls, a trained performer or tested
hardware. No audio-engine behavior is changed by this design exercise.

## The session

A composer is building an eight-bar, restrained string line that rises into
the next section. They understand harmony, MIDI and automation; they do not
want to learn violin technique to get a convincing performance.

| Beat | Composer action / listening question | Surface | Musical result to preserve |
| --- | --- | --- | --- |
| 1. Find the instrument | Insert Strings on a DAW instrument track. Choose violin, solo, assisted playing. Play a few notes. “Does this exposed sound belong in the cue?” | Play | No automatic sound on load. Useful held-note development without a pedal or extra controller. |
| 2. Lay down notes | Record the melody into the DAW using MIDI or enabled computer keys. Correct harmony/timing in the DAW. | DAW; Play stays available | DAW owns the source MIDI. Pitch entry is independent of continuous expression. |
| 3. Capture context | Open Phrase, arm capture, and run the host loop once. “What did the instrument receive?” | Phrase | Explicit capture of received events; no imaginary access to the host's clip. Notes remain unchanged. |
| 4. Suggest a performance | Request connected phrasing for the captured passage. Audition Original / Suggested over the same notes. “Are the transitions and rise convincing?” | Phrase | Editable bow/expression suggestions. Preserve detached notes; do not quantize or rewrite pitches. |
| 5. Add intent | Open Expression and arm only the intensity lane. Perform a second pass with a mouse, wheel or gamepad; keep other lanes assisted. | Expression; optional Inputs dialog | A recorded physical-control layer, not replacement notes. No need to play the melody and controller simultaneously. |
| 6. Repair one moment | Select the phrase peak, lower its bow-weight curve, soften the release, compare and undo if needed. | Refine state within Expression | Local edits preserve timing/pitch and unrelated expression; no repeated full take required. |
| 7. Build the arrangement | Duplicate/create DAW tracks for viola, cello and bass. Choose section mode/player count where wanted. Check transitions in context. | DAW; shared Play controls | Separate parts remain in the DAW. No new score editor, mixer or orchestral arrangement screen inside the plugin. |
| 8. Commit and return | Save the project; use host automation or a reproducible captured phrase/control layer. Reopen and audition before bouncing through the DAW. | Persistent footer / host | Save the sound, expression, source ownership and interpretation revision. Never double-trigger host notes and phrase playback. |

## Five screen images, three destinations

1. **Play:** instrument/mode, assisted/direct behavior, a small note ribbon,
   musical expression macros and optional physical detail. Main home screen.
2. **Phrase:** explicit event capture, compact pitched-duration blocks,
   suggested connections and Original/Suggested/Apply/Undo. No staff notation.
3. **Expression:** a second-pass gesture surface and one armed expression lane.
   Replaying captured notes is explicit, and host-owned notes remain distinct.
4. **Refine:** the same Expression destination in an editing state, not a new
   global tab. Note ribbon above one editable expression curve; other lanes
   collapsed. Original and edit are separate revisions of the same phrase.
5. **Inputs:** a focused modal, not a permanent side desk. Computer keys,
   MIDI, mouse/touchpad and standard controller profiles share mapping,
   calibration, ownership and release semantics. Feedback is optional/off.

The only top-level tabs are **Play / Phrase / Expression**. Inputs is a header
button. Instrument-specific physics belongs in an optional Play drawer, not a
fourth parallel workflow. Section mode conditionally reveals player count.
All views preserve the sound and current take.

## Native component budget

Target one **1040x720** resizable native plugin surface. Use a single system-sans
type family, 14–16px working labels, flat cool-neutral fills and one blue accent.
No textures, paper, gradients, transparency tricks, shadows, instrument
illustrations, bitmap knobs or decorative plots. Native-scalable primitives:

- Shared header: instrument/mode dropdowns, Inputs button, output value and
  panic. Shared tabs and footer show phrase source and control ownership.
- Buttons, segmented choices, standard sliders/numeric fields, dropdowns,
  checkboxes and one modal component. Consistent alignment and hit targets.
- Note ribbon: rounded rectangles on light time/pitch guides. Actual event
  durations, gaps and simultaneous notes, not an audio waveform.
- Curve editor: a small vector polyline with editable points, time ruler,
  selection, original dashed curve and an applied solid curve.
- Gesture pad: plain rectangle with a point and two axes. No controller image
  required; readable mapping rows describe sticks/triggers/touch instead.

The generated images are spatial proposals, not pixel-perfect specifications.
This component contract wins over model-invented labels or chrome. Review
contradictions explicitly rather than treating image pixels as working behavior.

## Input and assistant contracts

- Assisted keyboard-only performance is the default. A sustained key may have
  authored dynamics without requiring continuous hand motion. Plain computer
  keys have a selected attack value; no pressure/velocity sensing is implied.
- The assistant may be rule-based first; show its actual implementation type.
  Do not label heuristics as trained AI or claim acoustic realism from a badge.
- Live suggestions use past/current events with declared bounded latency.
  Captured-phrase interpretation can use later notes; these are distinct modes.
- One owner per applied lane: assistance, manual take or host automation.
  Arm only a chosen lane for overdubbing. Preserve raw and applied controls;
  musical macros ultimately drive physical excitation, not just output gain.
- Mouse/touchpad position is mapped intent, not measured pressure. Standard
  controller triggers/sticks are optional analog inputs. Detect device and
  mapping support; touch/feedback require separate capability checks.
- Panic, cancellation, blur/disconnect and teardown release owned notes and
  effects. Do not send competing notes from host and captured playback. Store
  mappings per device/profile; show what is armed before writing a take.
- Saving/reopening must retain notes, controls, interpretation revision and
  sound state. Host tempo/loop alignment and external automation ownership
  need real integration tests before this workflow is described as delivered.

## Delivery and implementation boundary

`prompts/` records the shared component style and each screen request.
`images/` contains generated raster concepts; `overview.png` arranges the five
images into the storyboard. `generation.json` records hashes/model/settings.

This pass is a design artifact only. The current native plugin does not yet
implement phrase capture/interpretation, the new continuous editor or this
complete shared shell. No new model training, audio validation or physical
controller test is being claimed. Use the existing engine evidence separately.

## Implementation order

1. Build the shared shell and keyboard-only assisted Play path against the
   real engine. Validate held tones, transitions, releases and control ownership
   before polishing visuals.
2. Implement explicit capture/phrase identity and repeatable interpretation;
   store note events independently of suggested controls. Test host loop/seek,
   no double-triggering, undo, state restore and unaffected original notes.
3. Add a single-lane second-pass record/edit path with one shared time base.
   Validate handover to assistance and deterministic replay before adding lanes.
4. Add device profiles/calibration to that existing path, not a second parallel
   expression engine. Test disconnect, focus loss, cancellation and panic.
5. Evaluate realism with exposed audio and composer listening; improve the
   physical model and performer together. Do not use UI completion as a proxy
   for acoustic quality.
