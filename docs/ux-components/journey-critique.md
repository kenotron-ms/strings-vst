# Strings native UI: heuristic user-journey critique

Date: November 8, 2026. Scope: the current working-tree native UI, including
uncommitted keyboard changes; not a claim about a released package.

## Method and evidence boundary

This is a source-informed heuristic walkthrough, not a human usability study.
The user's reports (keyboard too small, workflow unclear, unclear play/record/
pause, only four bars) are inputs to the critique, not measured findings from
additional participants. No live standalone or DAW interaction was performed
for this review. Acceptance scenarios below are instructions for the main
agent, not test results.

“Current” and “Baseline” refer to the review-start source/screenshot snapshot.
Implementation may proceed concurrently; those subsequent changes are not
revalidated here. Proposed gates describe the target, not completed fixes.

Read `src/UI.cpp`, `src/ui/PerformanceSession.hpp`,
`src/ui/PerformanceViews.hpp`, `src/ui/PlayableKeyboard.hpp`,
`src/ui/PlayableKeyboardView.hpp`, and
`docs/ux-components/native-performance.md`; additionally checked
`src/Plugin.cpp` for the MIDI/parameter boundary. Visually inspected the actual
1040 × 840 native capture `build/keyboard-validation/01-clicked-key.png`.
Source locators below are repository-relative and refer to this working tree.

The screenshot shows a calm, coherent visual palette, visible note/shortcut
labels, and three understandable expression macros. Those are useful assets
to preserve. However, the strongest visual hierarchy belongs to the knobs and
global-looking top buttons, while the playable keyboard occupies the bottom
strip. The small footer warning about local transport does not resolve that
hierarchy conflict.

### Bounded implementation direction supplied during review

The requested direction is a genuine full-width **88-key A0–C8 piano**, a
highlighted computer-typing span, real local-preview Pause/Resume, explicit
transport/capture labels, and a **seconds-based fixed eight-second demo**
instead of a four-bar-looking song. These are proposed changes, not evidence
that the inspected code or screenshot already implements them. Standalone
song recording/export and host-tempo transport remain future gaps.

Independent assessment: this is a useful audition-tool direction, but can
still mislead if a large piano plus Record implies melody capture, Pause
appears to control the DAW, or seconds labeling changes only the caption while
bar numbers remain. Also, 52 white keys across roughly 984 logical pixels
leave about 19 pixels per white key, narrower at the minimum window size.
The 88-key overview solves visible range, not automatically mouse precision.
Preserve depth, test black-key targeting and offer optional magnification
without replacing the full-range default with another one-octave strip.

## What actually exists today

| Area | Current capability and boundary |
| --- | --- |
| Standalone notes | Mouse press/drag plays one pointer note at a time. Computer keys map all 13 semitones across one octave, using `A W S E D F T G Y H U J K`; overlapping computer keys are supported. Editor focus is required. This is live playing, not note recording. |
| Keyboard geometry | Eight white-shaped keys and five black-shaped keys, 576 logical pixels wide and 76 high; white-key pitch is 72 pixels, black keys are 42 × 46. `Z`/`X` and minus/plus shift by octaves, limited to ±2. Velocity starts at 100; buttons change it by eight, clamped to 16–127. |
| Pitch/register | Base MIDI notes are 60/48/36/28 for violin/viola/cello/bass, plus the shared octave offset. The phrase and live keyboard both use this base. This is not an independently browsable full-range keyboard. |
| Local phrase | Eight pre-existing editable notes in a fixed eight-second window. Pitch is restricted to base through base + 12; start/duration drags snap to 0.25 seconds. No note insertion, deletion, MIDI import, note capture, or longer arrangement exists. Overlaps choose the most recently started active note; this preview is not a polyphonic clip editor. |
| Ruler | Sixteen beats at the displayed fixed 120 BPM: four implied 4/4 bars. Labels 1, 2, 3, 4, 5 mark boundaries; 5 is the end boundary, not a fifth playable bar. There is no scrolling, zoom, loop, tempo control, or host sync. |
| Audition | Starts/restarts the local phrase at zero using a UI steady clock. It is not host Play. Clicking the active “Playing phrase” button starts over; there is no pause/resume state. Preview notes use MIDI channel 16, computer keys channel 14, pointer keys channel 15 (human-readable numbering). |
| Record expression | Immediately starts the existing local phrase and records requested weight, speed/direction and engagement into an editor-local scratch take. It does not record played notes, audio, Tone, Vibrato, or an authoritative incoming MIDI/controller stream. Gamepad mode disables this action and says “Record MIDI in DAW.” |
| Compare/edit | Original/Take choose the source for the next audition and stop current preview. Original means current baseline controls, not an immutable pre-recording snapshot. A weight-lane drag edits the nearest stored frame; an empty lane seeds 33 frames. One context-dependent swap Undo addresses note edits or take changes, not a unified history. |
| DAW relationship | Host MIDI drives the instrument; parameters are automatable. UI `sendNote()` injects notes into DSP, not the host track's recordable MIDI input. Strings declares MIDI input only, no MIDI output: clicked/typed plugin keys and local-demo notes are audition sources, not a route to DAW MIDI clips. The native phrase neither shows the host clip nor follows host transport/tempo. Local replay writes real shared parameters and can contend with automation/CC writers. |
| Safety/lifetime | Stop, Escape and focus loss stop local playback and release UI-owned notes; they are not host panic or host Stop. End-of-phrase and an idle gap over 0.3 seconds also stop local playback. Local phrase/take objects live in the editor; no phrase/take serialization or export is present here. Closing/recreating the editor loses that scratch work. |

Evidence: `src/UI.cpp:18`, `src/UI.cpp:42`, `src/UI.cpp:68`,
`src/UI.cpp:111`, `src/UI.cpp:175`, `src/UI.cpp:251`, `src/UI.cpp:353`,
`src/UI.cpp:396`; `src/ui/PerformanceSession.hpp:8`,
`src/ui/PerformanceSession.hpp:23`, `src/ui/PerformanceSession.hpp:78`;
`src/ui/PerformanceViews.hpp:130`; `src/ui/PlayableKeyboard.hpp:8`;
`src/ui/PlayableKeyboardView.hpp:10`; `src/Plugin.cpp:50`.

MIDI-routing clarification additionally verified against
`src/DistrhoPluginInfo.h:11` (MIDI input enabled, no MIDI-output declaration)
and `third_party/dpf/distrho/DistrhoUI.hpp:179` (`sendNote()` is documented as
UI-to-plugin-DSP delivery). Sound from a plugin key is not evidence that the
host has recorded a MIDI note.

Documentation lag: `native-performance.md` describes the computer-key white
notes but omits the current chromatic/mouse keyboard; it also uses
“Mouse/Touch” and “Controller” where the screen has separate Mouse, Touch and
Gamepad tabs. Source and inspected screenshot take precedence in this critique.

## Severity-ranked findings

Severity is heuristic impact, not observed frequency: **S1 critical** blocks a
core journey or risks lost work; **S2 major** creates a misleading model or
material interruption; **S3 moderate** adds friction or obscures recovery.

### F1 — S1: standalone composition has no capture-and-keep path

**Journey:** standalone composer. “Play without MIDI,” a piano roll and a
prominent record button suggest that a melody can become a composition.
Actually, live keyboard notes are never inserted into the eight-note phrase,
and Record expression starts the supplied melody. Neither phrase edits nor
scratch takes have a save/export path, and editor recreation discards them.
This is a capability gap, not something a larger keyboard alone fixes.

**Do now:** name this an audition/scratch tool, state “Expression only — plays
demo notes; does not record your melody,” and put “Temporary: lost when this
editor closes” beside the phrase/take, before investment. Do not call a capture
saved because it exists in memory. **Proposed composition scope:** add note
capture, note creation/deletion and durable save/export together; until then,
direct durable composition to a DAW. See A3, A9, A12.

### F2 — S2: local transport looks like the DAW/global transport

**Journey:** both, especially DAW. Audition, Stop and Record expression sit in
the global header beside instrument selection. Their true scope is explained
only in a small distant footer. There is no Pause; clicking “Playing phrase”
restarts it, and Finish recording stops playback as well as capture. Selecting
Original or Take also stops preview. In a DAW, local Stop leaves host playback
running; host playback is not a start/stop signal for this UI preview.

**Do now:** put controls inside a visibly bounded “Local demo phrase” region;
use explicit scoped verbs and explain restart behavior. In DAW use, collapse
this region by default and prioritize host MIDI instructions. **Proposed:**
implement genuine Pause/Resume only with the explicit state contract below;
do not relabel the existing restart action as Pause. See A2, A5, A6, A10.

### F3 — S2: the keyboard reads as a footer of pads, not an instrument

**Journey:** standalone first sound and playing. In the inspected capture,
keys are almost square (72-pixel pitch versus 76-pixel depth), separated and
rounded. Black keys are broad relative to white keys. Eight white keys give
only C-to-C in the violin screenshot. The issue is depth, silhouette, visible
range and priority, not simply small click-target widths. Shrinking the whole
window to its 832 × 672 minimum reduces depth to about 61 pixels; uniform
scaling cannot repair the proportions.

**Proposed:** make the full-width A0–C8 piano (88 notes, MIDI 21–108) a primary
surface, approximately 180–220 logical pixels deep at the 1040-wide layout,
with proportionate black keys and a continuous piano silhouette. Highlight
the 13-note typing span, not fictitious computer shortcuts for all 88 notes.
Reclaim instruction/knob whitespace or collapse the demo. Full range risks
imprecise mouse selection at this width: test it and offer optional
magnification, not another tiny-octave default. These dimensions are design
targets, not measured ergonomic standards. See A1, A7.

### F4 — S2: keyboard appearance and range controls can misstate pitch

**Journey:** choosing a lower instrument or exploring register. Geometry uses
a fixed C-shaped pattern of white offsets `{0,2,4,5,7,9,11,12}`, but bass starts
at MIDI 28 (E1). Its white-shaped second key is consequently F#1, and its first
black-shaped key is F1. Pitch labels are calculated correctly, but key colors,
“White notes” instructions and piano geography disagree. Octave browsing also
transposes the demo because both use `keyboardBase()`; changing octave stops
the preview and releases held notes. The ±2 bound has no visible endpoint
state in its enabled-looking buttons.

**Do now:** fix physical key geometry from absolute pitch classes, including
non-C viewport starts; do not hide the error by relabeling pitches. Separate
keyboard viewport, typing octave and explicit demo transpose. Show low/high
note names plus MIDI numbers and endpoint feedback. Keep unknown calibrated
instrument ranges distinct from MIDI validity. See A7, A8.

### F5 — S2: four bars are a hard capacity limit disguised as a timeline

**Journey:** standalone continuation or DAW clip inspection. The ruler is
musical-looking, but time storage is seconds, duration is always eight seconds,
and the phrase has exactly eight entries. The rightmost 5 is an end boundary.
There is no hidden continuation to reveal by scrolling; nor is this the host
clip. Changing a DAW to 90 BPM does not change the displayed local 120 BPM or
preview duration. Calling this a roll without a scope/capacity label invites
expectations its editing model cannot fulfill.

**Do now:** label “Local demo · 8 seconds · fixed length,” replace the entire
bar ruler/grid with seconds (for example 0, 2, 4, 6, 8 s), and show an end
marker. Remove the 120 BPM/bar-song framing, not just its caption.
**Proposed, only with composition scope:**
separate phrase length from visible bars; store beat positions, allow 4/8/16/32
bar lengths, horizontal zoom/scroll and explicit extension. Do not just draw
more ruler numbers over the same eight-second buffer. See A5, A11, A12.

### F6 — S2: mouse-only note performance competes with expression gestures

**Journey:** standalone composer with computer keyboard/mouse. One pointer
cannot hold a piano note and drag a macro simultaneously; pointer release
ends its note. Computer-key hold plus mouse knob is possible and should be the
recommended two-hand path. Mouse/Touch/Gamepad look like exclusive note-input
modes, but they select expression views; the bottom keyboard remains, and
computer notes are still accepted outside Keys. Switching views stops preview
and can restore a gesture baseline. This is not merely a visual tab change.

**Do now:** say “Expression control: Knobs / Bow pad / Gamepad adapter,” merge
Mouse/Touch unless there is a meaningful behavioral distinction, and show
“Hold a typing key; shape with the mouse.” **Proposed:** optional clearly
indicated latch for mouse-only exploration, with a nearby release and an
Escape safety contract; never imply a latch currently exists. See A4, A10.

### F7 — S2: DAW writers and note ownership are insufficiently visible

**Journey:** DAW composer shaping an armed/playing track. A local phrase can
sound alongside host MIDI, and take replay writes the same bow parameters as
automation or controller CCs. Original/Take do not isolate an A/B comparison
from those writers. Onscreen keys highlight only UI-held keys, not received
host notes, so lack of keyboard highlight is not evidence of missing MIDI.
Computer-note handling requires editor focus and consumes unmodified mapped
letters; shortcut interaction is host-specific and untested here.

Critically, `sendNote()` sends plugin-key/demo events to DSP. With MIDI input
only and no MIDI output, these are not host-input notes promised to a DAW MIDI
recorder. “Arm the DAW and play these plugin keys” is therefore not a safe
composition instruction. Use the DAW's own computer-MIDI keyboard, MIDI
editor, or an external MIDI input routed through the host track. Host-managed
audio capture/bounce and automation are separate from MIDI-note recording.

**Do now:** make the host the default composition/transport owner, warn before
starting a local demo over an external performance, and label take replay as
parameter-writing. Do not fabricate host play/armed indicators without an
actual host signal. **Proposed:** explicit “Typing keyboard enabled” control,
source-aware note activity and playback ownership; verify per host rather
than promising universal shortcut pass-through. See A5, A6, A10.

### F8 — S3: comparison, undo and release terminology hide side effects

**Journey:** either composer revising and recovering. Original suggests a
frozen original performance, but selects current baseline controls; Take
omits that only bow expression was captured. Undo switches domain according
to the last note/take edit and is a swap, not a conventional stack. “Release,”
“Release gesture,” Stop and Escape overlap: the keyboard Release also stops
preview; Release gesture currently stops preview and releases keys too.
Follow controls stops preview and restores the pre-pad bow snapshot, rather
than enabling host transport follow. Focus loss silently ends local capture.

**Do now:** name the active comparison source, Undo target and release scope;
show a completion/interruption reason and the temporary-take status. Separate
gesture lift from transport stop as a deliberate behavior change, not a
label-only patch. See A2, A9, A10.

## Journey 1: composer in standalone, keyboard and mouse only

Assumption: the existing JACK/audio prerequisites are already satisfied. The
inspected UI does not offer a complete audio-device setup journey; this review
does not infer device failure or successful audio from the screenshot meter.

| Step and intention | Current path and friction | Recommended path; proposed unless marked copy-only |
| --- | --- | --- |
| 1. Open and know what to do | Knobs dominate; “Play notes” is broad, with three possible note sources in a header hint. | Show “Standalone — play with typing keys or mouse” beside the primary keyboard. State that the local phrase is a temporary demo (copy-only). |
| 2. Choose a sound/register | Instrument button cycles four instruments; preset name is paired with remote arrows. Bass geometry is misleading. | Use a discoverable instrument selector and labeled preset navigation; show viewport and typing range separately; correct absolute-pitch geometry. |
| 3. Make the first note | Click/hold a key or focus the editor and hold A; mouse release/key-up ends it. Keyboard sits below every other task. | Put the enlarged keyboard in the main playing area, with nearby fixed velocity and focus/typing status. Keep exact note and shortcut labels. |
| 4. Shape a held note | Hold a computer key and drag Energy/Tone/Vibrato with the mouse. A pointer-held piano note cannot survive moving that same pointer to a knob. | Explain the two-hand method (copy-only); offer a separately enabled mouse latch only if implemented. Knobs remain primary; pad is optional. |
| 5. Capture the melody | Record expression starts the supplied eight notes, not the melody just played. The intended note-capture journey stops here. | Immediately explain the limitation (copy-only). A future “Record notes” must capture this performance into an editable phrase, not replay the demo. |
| 6. Try expression on a reference | Click Record expression; change Energy during the demo; Finish recording stops both. Select Take then Audition, or Original then Audition. | Label “Record demo bow expression,” show elapsed/remaining time and capture coverage. Use “Live controls / Recorded bow take”; selection should not masquerade as playback. |
| 7. Revise and continue past bar four | Existing notes can move/resize within one octave/eight seconds; weight frames can change. Empty roll clicks cannot add notes. | If still a demo, make fixed limits explicit (copy-only). A composition version needs add/delete, note recording, beat-based length extension and viewport navigation together. |
| 8. Pause, stop, save and return | No pause; preview restarts from zero. UI-owned notes can be released; scratch edits disappear on editor recreation. | Never claim resume/save now (copy-only). Implement persistence/export before presenting this as a standalone composition workflow; retain Escape as local safety. |

**Honest current successful journey:** choose a sound → play live → shape with
knobs → optionally audition the fixed demo and compare temporary bow takes.
“Compose, record notes, extend and save a piece” is not currently completable.

## Journey 2: composer in a DAW, MIDI track and host transport

Assumption: Strings is loaded on an instrument/MIDI track, with monitoring and
audio routing configured in the host. Arming, clip editing, track recording
and transport belong to that host; exact controls vary by DAW.

**Recording warning:** the plugin's clicked/typed piano keys audition the DSP;
they do **not** provide MIDI output for capture into DAW clips. Host Record
does not turn that internal audition path into a recordable MIDI source.
For recordable performance notes, use the **DAW computer-MIDI keyboard** or
**external MIDI input routed into the track**; alternatively create notes
directly in the **DAW MIDI editor**. Disable plugin typing input or move focus
appropriately when using host typing input, and verify the actual recorded
clip. Parameter automation, audio recording/routing, and bounce remain
host-owned; an audible bounce is not proof of captured MIDI notes.

| Step and intention | Current path and friction | Recommended path; proposed unless marked copy-only |
| --- | --- | --- |
| 1. Load and identify ownership | Plugin header presents local transport before distinguishing it from the host. | Default to “DAW MIDI performance”; explain “Use your DAW to play/record notes and automation” (copy-only). Put local demo behind an explicit section. |
| 2. Enter notes or monitor a MIDI source | Host notes drive the instrument, but the bottom keyboard does not show them. Plugin clicked/typed keys only inject notes into DSP; they have no MIDI-output route into host clips. | Use the DAW MIDI editor, DAW computer-MIDI keyboard or external MIDI input through the track for composition (copy-only). Label plugin keys “Audition only — not recorded as DAW MIDI”; a true incoming-MIDI indicator requires processor-to-UI data. |
| 3. Press host Play | Host MIDI sounds according to its track. Local ruler/playhead does not become the host clip and remains fixed at 120 BPM. | Keep the demo clearly independent (copy-only). A future host-position display must read genuine host timing and be labeled as position, not editable clip content. |
| 4. Shape or automate expression | Knobs change automatable physical parameters; local take replay can write competing values. | Guide automation recording through host automation mode (copy-only); explicitly expose whether live controls or local take replay are writing parameters. |
| 5. Press Record | Host MIDI Record captures configured host-input events, not the plugin's internal `sendNote()` audition path; plugin Record expression instead starts its own reference phrase. | Arm/record in the host with its computer-MIDI keyboard or external MIDI input; inspect the resulting clip. Keep bow-parameter automation and audio capture/bounce as separate host operations (copy-only). Never imply plugin keys or local capture create host MIDI clips. |
| 6. Stop/pause and switch windows | Plugin Stop does not stop host MIDI; focusing the DAW ends local playback/capture but does not control host transport. | Label “Stop demo,” show “Local capture ended: editor lost focus,” and keep global panic in the host (copy-only feedback changes). Do not fight host focus/shortcuts. |
| 7. Save and reopen | Host parameter recall is distinct from editor-local phrase/take retention; saving a project does not make the local scratch objects persistent. | Warn before local scratch work (copy-only). Either serialize versioned scratch data explicitly or keep it labeled transient; test actual host save/reopen separately. |

**Honest current successful journey:** enter notes in the DAW MIDI editor, or
record the DAW computer-MIDI keyboard/external MIDI input through the host
track → inspect the host clip → play host transport → shape Strings with
live controls/host automation → save/bounce in the host. Plugin keys audition
only; neither they nor the optional demo roll supply recordable host MIDI.

## Recommended terminology and transport contract

### Immediate terminology; retain current capabilities honestly

| Current wording | Recommended wording and disclosure |
| --- | --- |
| Audition / Playing phrase | “Play demo from start” / “Restart demo.” Both currently restart at zero; show separate text “Demo playing.” |
| Stop | “Stop demo.” Explain that this also releases UI-held notes under the current implementation, but never stops host transport. |
| Record expression / Finish recording | “Record demo bow expression” / “Stop recording demo.” State “Notes, Tone and Vibrato are not recorded”; ending capture currently ends demo playback. |
| Original / Take | “Live controls” / “Recorded bow take.” Show selection separately from Stopped/Playing/Recording state. Explain that Live controls is not a frozen original snapshot. |
| Undo | “Undo note edit” or “Undo bow take/edit,” according to the real target; if retaining a swap, distinguish its redo state rather than promising a history stack. |
| Local phrase | “Local demo — 8 seconds — temporary.” Use seconds on the ruler; state fixed duration and “Not your DAW clip,” not bars/tempo. |
| Plugin piano in a DAW | “Audition keys — not recorded into DAW MIDI clips.” Direct note recording to the DAW computer-MIDI keyboard or external MIDI input; direct editing to the host MIDI editor. |
| Keys / Mouse / Touch / Gamepad | Heading “Expression control”; “Knobs / Bow pad / Gamepad adapter.” Input modality must not imply MIDI is disconnected. |
| Follow controls | “Restore pre-pad bow controls” while that is its actual behavior; never use “Follow” as an unexplained host-sync promise. |
| Release gesture / Release | Existing behavior needs an explicit “Stop demo + release UI notes” scope until split. Proposed distinct actions: “Lift bow” versus “Release UI notes.” |

### Proposed state machine, not current implementation

- **Stopped → Play:** start the fixed local demo at zero. Offer a separate
  “Return to start” if cursor seeking is implemented; do not imply seeking
  exists now or overload an active Play as restart.
- **Playing → Pause:** release demo-owned sounding notes, freeze demo position,
  suspend take parameter writes, and restore baseline. **Paused → Resume:**
  reapply the selected expression source and retrigger only notes active at
  the frozen position. Never emit a backlog of elapsed note-ons.
- **Stop:** end local playback/capture, release local owned notes and reset
  demo cursor to zero. Host-owned notes and host transport are untouched.
- **Record bow take:** start a new local capture from zero against the demo,
  with visible target, duration and replacement warning/undo. For the first
  version, disable Pause while recording and explain why; implement segmented
  recording only as a separate feature. End/Stop retains a labeled partial or
  complete take, not a fictitious saved file.
- **Source selection:** Live controls/Recorded bow take selects the next
  playback source without silently starting it. Disable changes while playing
  or make a deliberate, visible transition; do not silently stop transport.
- **Interrupted:** focus loss or idle-stall safety ends local capture/playback
  and releases UI-owned notes; retain the partial take and show the reason.
  This is not Pause and must not resume automatically on refocus.
- **Host:** independent unless an explicit future sync mode is implemented.
  Host Pause/Stop never means that the plugin has permission to move transport,
  overwrite clips, or claim an all-channel panic.

Expose a single local status line: source, state, position/length, capture
coverage and retention, for example “Demo · Recording bow expression ·
2.3 / 8.0 s · Temporary.” Put warnings there, not in a distant research footer.

## Keyboard, range and timeline behavior

### Keyboard proposal

1. Use the requested full-width A0–C8, 88-key piano as the default overview:
   52 white and 36 black keys. Allow collapse in DAW mode, not a misleading
   alternate pitch range. Preserve key depth at 832 × 672 using collapsible
   secondary regions. Test narrow black keys explicitly; offer optional
   magnified detail or a wider-window affordance while keeping a clear
   full-range overview. Key count alone does not prove mouse usability or
   physical-keyboard scale.
2. Derive white/black identity and boundaries from absolute MIDI pitch classes.
   Include partial edge keys correctly for non-C starts. Label C notes plus
   selected/held pitches; show MIDI numbers to disambiguate octave naming.
3. Separate **view range** (A0–C8, or an explicitly magnified detail), **typing
   octave** (the 13 shortcuts), and **demo transpose**. Browsing should not
   transpose phrase notes. Highlight the typing span and keep shortcut labels
   within it; show the selected/held pitch separately rather than crowding
   every key. Do not imply 88 independently mapped computer shortcuts.
4. Move the typing span within A0–C8 with clear endpoint policy, disabled
   controls and “Reset to instrument register.” Keep all 13 typing pitches
   visible and unique; do not clamp different shortcuts onto one edge pitch.
   An 88-key piano does not display MIDI 0–20 or 109–127. Show a string
   instrument's recommended range only after defining/validating it; neither
   the old ±2 limit nor the full piano is proof of a calibrated sounding
   range. Keep sounding versus written pitch explicit if transposition is added.
5. Distinguish UI-held notes from incoming host MIDI activity if implemented.
   Preserve note-off ownership across range/instrument/focus changes and clear
   stale highlights. Add explicit typing enablement in DAWs; do not steal
   transport shortcuts. Fixed velocity is not sensed pressure; label it as such.

### Timeline proposal: choose one honest product scope

**Near-term audition tool (the bounded direction):** keep the eight-second
demo, draw and label it entirely in seconds, place scoped transport beside
it, and collapse it for DAW use. Replace bar numbers and beat-grid styling
with second/subsecond ticks. Show “0.0 / 8.0 s,” an end marker, and “Demo notes
— not your recording.” There is no scrollable song beyond the end. No fake
bar-extension control, dead scrollbar or note-record promise. Pause/Resume
applies only to this demo; it does not imply a song sequencer.

### Explicit gap storyboards, not delivered workflows

**Standalone song:** play an original melody → future “Record notes” action
(currently absent) → review captured notes in a real editable phrase
(currently absent) → extend the song and shape expression (currently bounded
to the demo) → save/export and reopen (currently absent). The honest bounded
journey ends at live audition or a temporary demo bow take, not a recorded
song. A larger piano, Pause and seconds labeling do not close these gaps.

**Host-tempo performance:** open a DAW clip → future explicit host-sync mode
(currently absent) → consume actual host tempo/position/play state rather
than UI clock → handle pause, seek and loop transitions without duplicate or
stuck notes → record through the host → save the project. Until implemented,
host MIDI is the composition source and the local eight-second demo remains
independent. Host sync alone would not grant MIDI-clip access. Do not present
these storyboard steps as existing controls.

The safe **current host composition storyboard** is DAW computer-MIDI keyboard
or external MIDI → armed host track → recorded host MIDI clip (or direct
entry in the host MIDI editor) → Strings DSP → host audio output/bounce, with
parameter automation owned by the host. Plugin clicked/typed keys and local
demo follow a different path: UI `sendNote()` → Strings DSP → audio. There is
no MIDI-output arrow back to the DAW clip. Any future plugin-to-host MIDI
recording would require a new output/routing capability and host-specific
verification, not merely clearer labels or tempo synchronization.

**If standalone composition is a requirement:** first add note insertion,
deletion, note capture and persistence. Store notes and expression in beat
time, provide explicit tempo/time signature, snapping and phrase-length
controls, then offer four visible bars by default with scroll/zoom, a visible
total-length indicator, “Extend by 4 bars,” Fit phrase and optional playhead
follow. A 16-bar phrase must remain 16 bars when only four are visible.
Playback beyond the viewport must not truncate notes; loops need explicit
start/end, wrap note-off and capture policies. Note storage/polyphony and take
capacity must genuinely support the advertised length; extending the old
fixed 512-frame capture buffer silently is not a sufficient design.

**In a DAW:** host clips remain authoritative. Do not imply arbitrary clip
inspection/import or tempo sync exists. A future sync option must consume
host tempo, position, play state, seeks and loops, define automation ownership,
and survive editor focus changes via an appropriate non-UI scheduler. The
current UI-idle preview clock is not suitable for a promised host-synchronous
performance. Host-position display alone does not provide clip access.

## Acceptance scenarios for the main agent

These are reproducible checks to run, not validations completed by this
critique. **Baseline** verifies current behavior, including limitations;
**Proposed** is a future acceptance gate and may intentionally fail today.
Capture native screenshots and note events/audio where relevant. Use a named
DAW/version for host cases; do not substitute standalone smoke results for
DAW transport integration or claim successful audio from pixels alone.

| ID / status | Steps | Expected evidence/pass condition |
| --- | --- | --- |
| A1 — Baseline + Proposed: keyboard silhouette | Inspect at 1040 × 840 and 832 × 672. Count keys; click white/black centers and boundaries across the full width, including A0 and C8. | Baseline: eight white shapes, five black shapes, 76 logical-pixel depth. Proposed: 88 keys (52 white/36 black), A0=21 through C8=108, full-width piano-like depth, legible typing/selected-note labels, no clipping; visuals and hit tests agree at both sizes. Record narrow-key targeting failures rather than assuming 88 keys solves usability. |
| A2 — Baseline + Proposed: restart versus pause | Start demo, wait approximately three seconds, click its active Play action; then stop and play again. For a pause implementation, pause at three seconds, wait two seconds and resume. | Baseline: both starts return to zero; no pause exists. Proposed: Play does not silently restart; Pause freezes position and releases demo notes; Resume starts at the frozen position, not zero or five seconds, without queued bursts. |
| A3 — Baseline: what recording captures | Hold/type a melody different from the supplied phrase while recording expression; change Energy, Tone and Vibrato. Stop; select Take and audition. | Original eight phrase notes remain; typed notes were not captured. Bow expression replays, but Tone/Vibrato are not restored as recorded take data. Do not describe concurrently audible typed notes as note capture. |
| A4 — Baseline + Proposed: two-hand performance | Hold A and shape Energy with the mouse; release A. Next, hold a piano key with the pointer and attempt to reach a knob. | Baseline: computer hold permits simultaneous shaping; pointer release ends its note. Proposed help explains this; any optional latch is explicit, visibly on/off and reliably cleared by Escape. |
| A5 — Baseline + Proposed: DAW ownership and tempo | In a named DAW, make an eight-bar MIDI clip at 90 BPM. Play, pause and stop the host with local demo off; then deliberately start the local demo. | Baseline: host notes sound independently; local ruler stays four bars/120 BPM/eight seconds, with no host clip rendering. Proposed default keeps local demo opt-in and states scope; no unsupported host-state indicator. If sync is later added, test seeks/loops and 90→120 BPM separately. |
| A6 — Baseline + Proposed: recording/automation ownership | Arm host MIDI recording; use the DAW computer-MIDI keyboard or external MIDI input through the track, not plugin keys. Inspect the clip. Separately record host bow-parameter automation while adjusting Energy; safely try local take replay against automation. | Record evidence separately for host-input MIDI notes and parameter automation; local capture must not claim either host recording operation. Proposed UI warns before overlapping writers and identifies local take writes; local Stop leaves host transport and host-owned notes untouched. |
| A7 — Baseline + Proposed: bass and octave edges | Select bass at default offset; inspect the second white shape and first black shape. Test A0/C8 endpoints and move the typing span to both limits. | Baseline exposes F#1 (30) on a white shape and F1 (29) on a black shape. Proposed uses correct absolute-pitch colors/layout, labels and emitted notes; all 13 typing notes stay visible, unique, in range and aligned with the highlighted span. Piano limits are not calibrated string-instrument limits. |
| A8 — Baseline + Proposed: range is not transposition | Edit a phrase note, start preview, then use octave down/up; repeat while holding a computer/pointer note. In the proposed UI, use view-scroll separately from typing octave/demo transpose. | Baseline: octave action stops preview, releases keys and changes phrase register. Proposed: scrolling never changes stored phrase pitches; any typing/demo transpose has distinct labels and no stuck notes. |
| A9 — Baseline + Proposed: lifetime and recovery | Edit a note and capture a take; exercise the contextual Undo. Close/recreate the editor; separately save/reopen a host project. | Baseline: scratch phrase/take do not survive editor recreation; Undo is a domain-specific swap. Proposed: either conspicuous temporary-work disclosure or tested persisted restoration, with explicit Undo target. Host parameter recall alone is not evidence of phrase/take persistence. |
| A10 — Baseline + Proposed: interruption and release | Sustain UI notes, start local capture, then Escape, focus away, change instrument/preset, or simulate an idle stall over 0.3 seconds; test each separately and release physical inputs afterward. Also try Release gesture, keyboard Release and Follow controls. | Baseline: document actual cancellation/restoration scopes; no lingering UI note after its release. An idle stall stops preview, not necessarily an independently held typing note. Proposed: scoped labels, interruption reason/partial-take status, no auto-resume, balanced parameter gestures. Host-owned notes must not be mistaken for stuck UI notes or silently panicked. |
| A11 — Baseline + Proposed: seconds replace fake song scope | In current demo, try adding/deleting notes or scrolling past the end; inspect label 5. In the bounded revision, inspect every ruler tick and play/pause/resume through the end. | Baseline: no added/deleted notes or hidden continuation; 5 is only the bar-end boundary. Proposed: 0–8 seconds, no bar numbers/120 BPM song framing, demo-only labels and expression-only capture; paused wall time does not consume playback duration. End-of-demo stops/releases locally. No song export or host sync implied. |
| A12 — Proposed composition gate | Capture a new melody, add/delete notes, extend to 16 bars, place a note in bar 12, show only four bars, scroll/zoom, play through the viewport edge, then save/reopen/export. | Notes and expression beyond the viewport remain intact/audible at correct beat positions; total length stays 16 bars; durable data round-trips. If this cannot pass, retain audition-tool terminology and do not mark standalone composition delivered. |
| A13 — Baseline + Proposed: plugin audition is not host MIDI input | In a named DAW, disable other MIDI/typing sources and arm host MIDI recording. Click/hold plugin piano keys and type with plugin focus; inspect the host clip rather than judging sound. Separately record with the DAW computer-MIDI keyboard or external MIDI input; also test host automation and an audio bounce independently. | No plugin MIDI-output path is advertised or assumed: internal audition must not be claimed as captured host notes. Host-routed input should create verifiable clip notes when correctly configured. Record any host-specific special interception separately, not as a portable plugin capability. Proposed warning says plugin keys audition only; host automation or audible bounce does not prove MIDI-note capture. |

## Implementation priority

1. **Clarify transport and record scope now:** local demo controls beside the
   demo, honest restart wording, expression-only coverage, temporary-work notice.
2. **Make playing primary:** deep full-width 88-key A0–C8 piano, correct bass
   geometry, highlighted typing span and separate demo transpose; test narrow
   key precision and offer optional magnification if needed.
3. **Separate the two contexts:** standalone first-sound guidance versus
   host-owned DAW MIDI/transport/automation with an opt-in local demo.
4. **Remove the fake four-bar song:** seconds-only 0–8 s demo now; storyboard
   song capture/export and host-tempo transport explicitly as unimplemented.
5. **Make state and recovery inspectable:** implement genuine local Pause/Resume,
   clear Live controls/Recorded bow take, scoped release/Undo, interruption
   feedback and no false saved-state or host-sync promises.
