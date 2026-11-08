# Strings / bow desk

## Revised plan: five ways into a phrase

Build a gallery with five query routes (`?view=bow`, `mood`, `phrase`,
`conductor`, `material`). Each shares honest control state and safety release,
but changes the primary task and spatial organization:

1. Bow desk: an uninterrupted ribbon for direct gesture practice.
2. Mood field: a large semantic plane with a visible mathematical recipe.
3. Phrase studio: an editable step score with transport and repeatable contours.
4. Conductor: a seating fan and section-wide expression faders.
5. Material lab: a string/contact cross-section and physical parameter bench.

Shared palette and typography establish one instrument family; differentiation
comes from workflows, not recoloring. Mood uses an open two-dimensional field;
phrase uses horizontal time; conductor uses radial seating; material uses a
vertical signal path. Controls and explanatory text remain left-aligned.

Pre-build review: five variants of the bow desk would not answer the composer
brief. Only Bow asks the performer to imitate a bow. The other four start with
intent, repeatability, orchestration, or timbre. Semantic labels are authored
macros, not emotion detection or scientifically validated mood models. All
concepts show physical output, with no audio or fictitious output meters.

## Plan and pre-build review

The performance gesture gets the largest surface. Pitch sits beneath it as a
separate keyboard; instrument and section choices sit above. Setup lives in an
adjacent folding instruction desk, never between the player and the bow.

Palette: midnight blue `#172936`, blue felt `#244657`, silver `#b7cbd4`,
ivory `#f0ede4`, bow brass `#d6ba80`, muted coral `#e9a499`.
Type: the repository's Cormorant Garamond for instrument names and large
performance instructions; system sans-serif for controls. No network fonts.
Layout: left-aligned controls and prose, a centered bow diagram. On narrow
screens the instruction desk follows the playable surface.

```
Strings                 control-only notice       stop
Violin / Viola / Cello / Double bass      Solo / Ensemble
Instrument name                | Input source + connected status
                               | Learn the gesture
      large bow ribbon         | Mapping / calibrate
      X gesture / Y force      | Platform boundaries
live force / velocity / contact / vibrato
Independent pitch source: keyboard + MIDI concept
```

Review: a rack of knobs would repeat the native UI and conceal the important
change. A decorative violin would suggest finger tracking. Instead, the central
surface is an abstract bow ribbon crossing four strings, with honest input
values. Its physical slope and moving contact point are the single expressive
visual feature. No output meters, invented sound activity, or simulated device
connection. A fixed demonstration is deliberately omitted: every changing
performance value must follow a real interaction.

## Interaction contract

- Mouse and ordinary touchpad: hold and drag. Horizontal movement yields signed
  bow velocity, downward position sets force. Stationary gesture decays to zero.
  Force is assigned, never measured. Keyboard users can focus the bow surface,
  hold Space to engage, and use arrows to change velocity/force.
- Standard browser gamepads: RT force, LT engagement, left X signed velocity,
  right X contact, right Y vibrato. Choose a detected controller explicitly when
  multiple are present. Mappings can be edited. Nonstandard pads are listed but
  not interpreted. Neutral capture offsets axes; adjustable deadzone suppresses
  drift. This is session-only neutral calibration, not full hardware calibration.
- Pitch is independent: computer keys A W S E D F T G Y H U J K, or the on-screen
  keyboard. External MIDI keyboard input is a future integration, not connected
  in this mockup. Optional MIDI output can send the chosen pitch to a DAW.
  A controller never recognizes finger placement. Last-held pitch is displayed.
- Pointer release/cancel/lost capture releases the bow; independently held notes
  remain held. Window blur, hidden page, source changes, disconnect, and Stop
  release bow and notes. After safety release a controller
  must return its engagement trigger to rest before it can re-engage.
- Pen/touch pressure is intentionally not implemented. DualSense touchpad and
  adaptive triggers are advanced integrations unavailable via standard mapping.

## Implementation and limits

Open `index.html` directly for pointer/keyboard use. Serve the repository on
localhost for the best Gamepad API compatibility; focus the page and press a
controller button. Device exposure depends on browser, permissions, and OS.
No dependencies, browser audio engine, direct plugin connection, saved configuration,
or fabricated telemetry. Optional Web MIDI output is implemented below and stays
off by default. Instrument and ensemble select the prototype's intended
configuration only; player dots show requested section size, not active voices.
The prototype exposes signed velocity to make bow direction legible. The planned
native contract separates speed magnitude and direction; the adapter below is
a parameter contract. The optional MIDI adapter below implements the specified
CC subset, including native instrument/section selection, variation, and room.

Accessibility: native controls, visible focus, labelled live values, reduced
motion support, responsive layout, and a fully keyboard-operable bow fallback.
Continuously changing values are not live-announced; status messages are.

## Task-based comparison and recommendation

| Composer task | Best starting study | Why it helps | Cost / remaining question |
| --- | --- | --- | --- |
| Capture an intimate, unsettled mood quickly | Mood field | Start with two understandable dimensions; inspect the exact force, speed, contact, and vibrato recipe. Presets offer a first gesture in one click. | Labels are subjective. A real engine and listening tests must establish whether those names match the sound. |
| Repeat a swell or revise a cue after feedback | Phrase studio | Eight editable steps include pitch, force, and signed motion; loop, change tempo, reverse, export. No violin technique required. | Steps are intentionally discrete; a production version needs curves, musical-time automation, and DAW synchronization. |
| Lead a chamber group or broad section | Conductor | Requested seats, energy, unity, and distance express orchestration decisions. Hold-to-conduct makes phrase boundaries explicit. | Player variation and room are requested parameters, not a rendered ensemble. Per-player timing and balance remain future work. |
| Discover a grainy, fragile, or resonant timbre | Material lab | Physical contact and friction controls sit beside body coupling, sympathetic resonance, and rosin. Named starting recipes reduce blank-page work. | Mechanical settings are less immediate than mood; their audible response is not validated here. |
| Perform a phrase continuously with hands | Bow desk | Spatial gesture communicates motion and force directly; gamepad offers separate continuous axes and triggers. | Requires coordination and a separate pitch source; less suitable as the first experience for a non-string player. |

Recommendation for the eventual product: lead with Mood field and a visible
"keep this phrase" path into Phrase studio. This prioritizes musical intent and
repeatability. Make Material lab the deeper tone editor, Conductor the section
workspace, and Bow desk an optional performance surface. Keep all five studies
independent here so an evaluator can compare their learning costs fairly.

Suggested evaluation tasks, each with a fresh route:

1. In under a minute, prepare a quiet phrase that grows uneasy. Explain which
   controls changed, record five seconds, and replay it.
2. Revise only the peak of an eight-step swell, then reverse the bow direction
   on the release without changing the notes.
3. Turn a solo into a close four-player section; increase collective energy
   while preserving unity.
4. Prepare a grainy bridge-side recipe, then soften it without changing pitch.
5. With pointer and with controller, release expression intentionally; switch
   focus mid-gesture and confirm that force returns to zero.

Compare time to first intentional phrase, explanation of the mapping, precision
of revision, ability to repeat, and perceived physical effort. Audio quality and
emotion fit cannot be assessed from these control-only prototypes.

## Engine bridge proposal

The parent implementation owns audio, DSP, parameter names, and validation. No
browser view claims professional acoustic equivalence or measured calibration.
Two-polarization traveling waves, a passive modal bridge/body, and four coupled
strings belong to that engine; this prototype does not simulate them visually.

| Browser control | Proposed engine parameter / interpretation |
| --- | --- |
| `force` | `BowForce` (or existing `BowPressure`), normalized 0–1 |
| `velocity` | `BowSpeed = abs(velocity)`; `BowDirection = sign(velocity)`, in −1…1 |
| `engaged` | `BowEngaged`, boolean; stop always sets false and force/speed to zero |
| `contact` | UI 0 = fingerboard, 1 = bridge; adapter sends `BowPosition = 1 - contact`, because the native parameter measures distance from bridge |
| `vibrato` | `Vibrato`, normalized depth; no Hz or cents claim |
| `bodyCoupling` | `BodyCoupling`, normalized request |
| `sympathetic` | Sympathetic resonance, normalized request |
| `rosin` | `Rosin`, normalized friction-character request |
| `variation`, `room` | Player variation and room send, normalized requests |
| `note` | Independent MIDI note number or null; last-note priority in this study |

Exports use `strings-control-study/v1`, `controlOnly: true`, explicit units, and
the instrument/section configuration captured when recording begins. Takes have
millisecond timestamps at approximately 30 Hz, include physical controls and
pitch, and end with an explicit release sample. Recording stops at 20 seconds.
Replay follows those samples and returns to rest. Recording and export remain
local, with no upload. Score exports use eighth-note steps, tempo, looping, and
optional pitch; they are separate from captured gesture takes.

Actual integration should use an adapter that clamps ranges, schedules automation
against the audio clock, sends note-on/off separately, handles direction changes,
and guarantees release on session loss. The browser frame timer is not an audio
clock. Configuration changes during a take end that take, so one take does not
silently change instrument. Full calibration would measure device range and
response; this prototype only captures neutral offset and applies a deadzone.

## Routes and evaluation selectors

Serve the repository root, then open `/docs/ux-performance/index.html`.
Direct `file:` use works for pointer and keyboard; Gamepad API exposure varies.

| Route | Main interaction selectors |
| --- | --- |
| `index.html` | `[data-concept="bow"]` and corresponding concept names |
| `index.html?view=bow` | `#gesture`, `#bow-contact`, `#bow-vibrato` |
| `index.html?view=mood` | `#gesture`, `#mood-weight`, `#mood-unrest`, `[data-mood]` |
| `index.html?view=phrase` | `#phrase-play`, `#phrase-stop`, `#step-force-0` … `7`, `#step-speed-0` … `7`, `#step-note-0` … `7`, `#tempo`, `#loop` |
| `index.html?view=conductor` | `#energy`, `#unity`, `#distance`, `#engage`, `#players` |
| `index.html?view=material` | `#material-force`, `#material-speed`, `#material-contact`, `#material-vibrato`, `#body-coupling`, `#sympathetic`, `#rosin`, `#engage` |

Shared selectors: `#input-source`, `#device`, `#mapping`, `#calibrate`,
`#deadzone`, `#record`, `#replay`, `#export`, `#stop`, `[data-note="0"]`,
`#live-force`, `#live-velocity`, `#live-contact`, `#live-vibrato`,
`#pitch-value`, `#device-status`. `body[data-view]` identifies the active study.

## Optional real Web MIDI output

All five studies include `midi.js`. Nothing requests MIDI permission or sends
messages automatically. The user explicitly clicks `#midi-connect`, chooses
`#midi-output`, and checks `#midi-arm`. Access uses
`navigator.requestMIDIAccess({sysex:false})`; there is no MIDI input listener or
SysEx. The header distinguishes browser-silent default mode from armed external
MIDI output. Access failure leaves local controls functional.

The route is browser gamepad/pointer/keyboard → physical control values → Web
MIDI output → OS virtual MIDI route → DAW MIDI input → native Strings plugin.
The user must supply the virtual route and DAW configuration. The browser cannot
confirm that the native plugin is connected or producing sound. Use localhost
or HTTPS in a Web MIDI supporting browser. Browser support and OS routing vary.

Channel 1 mapping:

| Message | Value |
| --- | --- |
| CC16 | Force, 0–127 |
| CC17 | Absolute velocity / speed, 0–127 |
| CC18 | Native bow position, 0 bridge to 127 fingerboard; inverted from the study's contact scale |
| CC19 | Signed direction mapped to 0–127: negative 0, stationary 64, positive 127 |
| CC20 | Bow engagement, 0 or 127 |
| CC21 | Body coupling, 0–127 |
| CC22 | Sympathetic resonance, 0–127 |
| CC23 | Rosin, 0–127 |
| CC24 | Instrument index: violin 0, viola 1, cello 2, double bass 3 |
| CC25 | Solo 0 / ensemble 127 |
| CC26 | Requested players, integer 2–16 |
| CC27 | Player variation, 0–127; inverse of conductor unity |
| CC28 | Room, 0–127 |
| CC1 | Vibrato, 0–127 |
| Note on / off | Independent pitch, monophonic last-note priority, velocity 100 |

The bridge caches unchanged CC values and caps writes at 60 updates/second;
the current shared visual/control loop supplies approximately 30 updates/second.
Changes in pitch turn the old note off before starting the new one. Force and
engagement remain independent of pitch. Recorded takes and score playback also
feed the bridge only when armed. Instrument choice, section size, unity, and
room use the research engine's CC24–28 contract. These CCs are global parameter
edits, not per-note MPE. The visualizer itself contains no audio DSP.

Global release, blur, page hiding, input-source changes, and gamepad disconnect
send immediate note-off, CC16/17/20 = 0, CC123 all-notes-off, and CC120
all-sound-off. These affect channel 1 on the selected output; use a dedicated
instrument route. MIDI port changes/disconnects and send failures disarm output.
`#midi-panic` sends release and disarms. A lost connection may prevent delivery;
the UI explicitly directs the user to the DAW panic control in that case.
Source changes preserve an already armed output but reset expression and pitch.

Additional synthetic Chromium checks verified no permission request or sends
before opt-in, `sysex:false`, selection without sending, explicit arm, exact CC
bytes, unchanged-value caching, keyboard note on/off, blur panic, controller
force/direction → MIDI output, and disconnected-port disarming. These use test
fixtures, not a physical controller or real MIDI endpoint. No real DAW or audio
verification was performed by this prototype task.

## Verification and critique

Headless Chromium loaded the gallery and all five routes without runtime errors.
Keyboard expression, independent computer-key pitch, global blur release, and
390px-wide layouts were exercised on every route. A captured take replayed and
released. A synthetic standard gamepad fixture verified trigger force, signed
stick velocity, stop/re-arm gating, and disconnect release. This is software
input testing, not a real Xbox/DualSense hardware compatibility claim.

Design critique: each workflow now has a different working center, but the
shared physical readouts and input desk intentionally preserve orientation.
The material view has the highest control density; it should remain an advanced
destination. The mood view is easiest to approach, but its audible usefulness
must be evaluated against the real engine. There are no sound-quality findings
from this prototype. Parent review owns independent screenshots, listening
evaluation after integration, and the final product recommendation.
