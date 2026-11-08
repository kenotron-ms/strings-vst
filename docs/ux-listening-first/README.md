# Strings: listen first, discover the instrument

The flat form-heavy batch in `../ux-production-storyboard/README.md` was rejected
by the user. Restore the organic performance emphasis of these earlier studies,
especially analog/controller gestures, while removing textured backgrounds.
The new corrective concept is a direction check, not an approved replacement.

Design revision requested November 7, 2026. This is a **new component and
interaction proposal**, not a claim that the native UI already implements it.
The five earlier Astra studies remain available in `../ux-performance/`.
These five new image briefs show states of one coherent product, not five skins.
The user authorized the GPT Image API fallback on November 7, 2026. Raster
concepts are saved in `images/`; these are generated design studies, not
screenshots of implemented native functionality. See `review.md` for selection
and remaining design issues.

Latest direction: **no notation requirement**. The product is an approachable,
physics-based instrument, not a score editor or violin-training simulator.
`sound-first-direction.md` supersedes the staff notation in the generated Listen
and Refine images. A MIDI or computer keyboard must be sufficient for assisted
playing; mouse/touchpad/controllers add optional expression rather than unlock
basic sound quality. This direction is a proposal, not newly implemented code.

## Start with the listening job

The first question is “What could this sound like in my cue?”, not “What is
the coefficient of friction?” Let a person hear a quiet phrase immediately,
change its character, compare the result, and only then choose how to perform it.
No note should sound automatically just because the editor or a device opens.

The default view fits one native-sized window without a permanent input desk,
physics panel, full keyboard or long instructional sidebar. One helpful sentence
and one obvious audition action are enough to begin.

```
Strings       Violin [choose]    Solo [choose]       Output / mute

                 Hear what it can do
        [Play phrase]  [Stop]    Quiet rise [choose]  Loop [off]
        phrase ribbon: opening -------- rise -------- release

                     Character surface
               restrained <-----> intense
                     soft <-----> edged

        [Keep A] [Compare B]       [Shape] [Perform] [Refine]
        Input: demo phrase         [Choose an input]    [Details]
```

All labels above are proposals. The character surface needs audition-based
mapping and should not pretend to recognize emotion or guarantee an acoustic
technique. A phrase ribbon represents actual authored/playback events, not a
decorative waveform or invented live engine telemetry.

## Component contracts

| Component | What the listener does | What it must actually do |
| --- | --- | --- |
| Instrument header | Choose violin, viola, cello or bass; solo or section | Change the real model. Reveal player count only for a section. Keep output/mute stable across modes. |
| Audition transport | Play a short phrase, hold one note, or listen to live input | Run explicit note/expression events through the same native engine. Show the active pitch source. Stop releases audition-owned notes, never unrelated DAW notes. No autoplay. |
| Phrase chooser | Hear quiet rise, sustained tone or short gestures | Use repeatable MIDI/control sequences, not unrelated audio examples. Avoid changing the sequence when comparing tone settings. |
| Character surface | Move toward a softer, more intense or more edged result | Map to inspectable bow/contact/vibrato settings. Show the recipe on demand. Bound normal performance ranges; label experimental extremes. |
| Listening comparison | Keep A, adjust B, replay the same phrase | Snapshot sound settings, preserve the musical events, and offer an explicitly indicated loudness-matched comparison. Do not change the saved output fader or silently normalize exported audio. |
| Output strip | Hear at a safe level, mute instantly, recover from a stuck input | Real output meter/clip indication and an always-available stop/panic action. Never use a fabricated meter. |
| Input chip | Use mouse, MIDI, Xbox-style pad or DualSense | Open a focused setup flow, detect capabilities and show exactly what is connected. A selected profile is not a device. |
| Shape drawer | Understand why the tone changed | Reveal bow weight, motion, contact and vibrato. Change one cause at a time; allow audition without closing the drawer. |
| Perform overlay | Learn one gesture while hearing its result | Show one highlighted control, its musical meaning and a live normalized value. Calibrate neutral, then try a note. Escape/cancel releases owned performance. |
| Phrase editor | Fix only the peak or ending of a performance | Preserve pitch and timing while editing expression. Record the actual applied controls; offer undo and compare original/edit. Do not replace a live take with a canned curve. |
| Model drawer | Explore material behavior intentionally | Expose coupling, sympathetic strings, rosin and propagation settings with defaults/reset. Keep engineering detail out of the first-use view. |

Loudness matching, integrated audition transport, continuous phrase editing,
undo and this native component hierarchy are **proposed**, not delivered by
the existing eight-step browser study. No acoustic reference recording or
automatic “realism score” is implied.

## Progressive disclosure

1. **Listen:** instrument, solo/section, one phrase, character and output.
2. **Shape:** a small drawer explains bow weight/motion/contact/vibrato; an
   expandable recipe shows what a macro changed. Keep Play and A/B available.
3. **Perform:** choose an input when needed. Teach one mapping, then let the
   person play. Connection/calibration belong here, not in every screenshot.
4. **Refine:** open phrase lanes for an actual recorded or audition sequence.
   Show note timing plus a selected expression lane, not every lane at once.
5. **Model:** disclose mechanics, ranges and diagnostics only when requested.

These are task views sharing the same musical state, not a forced onboarding
sequence. An expert can open Model directly; a beginner never needs to.
Switching views preserves the current sound, take, pitch source and comparison.

## DualSense: design a feedback loop, not a controller photograph

Sony documents DualSense adaptive trigger tension and haptic actuators. A native
input layer can investigate touch contacts and device-specific effects through
SDL. Support must be discovered on the actual device, OS and transport. The
current standard-browser adapter does **not** implement those advanced features.

Proposed starting mapping:

| Control | Musical job | Reason / fallback |
| --- | --- | --- |
| Left stick X | Signed bow motion | Center means rest. Teach reversal by listening; calibrate center/deadzone. |
| R2 / right trigger | Requested bow weight | An analog position, not a force sensor. Optionally add light adaptive resistance when supported. |
| L2 / left trigger | Bow clutch: hold to engage, release to lift | Independent, predictable release. Offer toggle/keyboard alternatives for accessibility. |
| Right stick X / Y | Pitch bend / vibrato depth | Never silently change the selected note. MIDI or a selected audition/scale-note source supplies pitch. |
| DualSense touchpad X / Y | Contact position / brightness | Continuous position, not measured fingertip pressure. Activate only when touch data is actually available. |
| Selected scale-note mode | Face buttons choose four mapped pitches; D-pad changes register | Optional gamepad-only exploration. A limited note layout, not a full violin fingerboard. Default audition mode avoids requiring two hands on a pad plus a MIDI keyboard. |

Offer an alternative **touch bow** preset: horizontal finger motion on the
touchpad drives signed bow velocity instead of the left stick. Show source
ownership and explicitly disable the competing velocity mapping; do not let
two inputs fight. Finger lift releases touch-owned excitation. Xbox/mouse users
retain the same musical controls through other mappings.

Feedback is an output of the simulation, not an additional sound control:

- Optional, gently bounded R2 resistance can represent a low-pass/compressed
  contact-load estimate. It is not a calibrated reproduction of a violin bow.
- Optional low-intensity haptic texture can represent slip activity or onset.
  Quiet sustained playing should not become constant distracting rumble.
- Show “Feedback off”, “Available”, “Testing” or “Unavailable” based on facts.
  No fake touch trails or haptic-ready badges in the disconnected state.
- Off by default; explicit test, intensity cap, disable control and comfort
  warning. Stop effects on release, panic, focus/device loss and teardown where
  the transport still permits delivery. Never keep pushing a held trigger.
- Handle devices/effects outside the audio callback. Do not copy raw audio
  samples into motors, or claim the musical loop is a passive haptic servo.
- Preserve mapping and access to sound when feedback is unsupported. Never
  require adaptive triggers for playing or infer pressure from a generic field.

## Visual direction and self-review

Use warm paper `#F2F0EA`, ink `#252B2B`, pine `#32594F`, sage `#A9B9AA`
and a restrained rosin accent `#C3A46C`. Humanist sans for controls; a small
serif instrument name provides musical identity without enormous headings.
Keep clear focus, readable labels and a quiet, left-aligned hierarchy.

The memorable element is the playable phrase/character surface, not a bank of
knobs, violin photograph or dashboard cards. Review changed the earlier plan:
remove the permanent right input desk, reduce repeated telemetry, hide the
keyboard until needed, and keep listening/compare available in every state.
Do not merely recolor the five earlier tall browser pages.

## Image-generation handoff

| State | Selected image |
| --- | --- |
| Listen | `images/01-listen-v2.png` |
| Shape | `images/02-shape.png` |
| Perform | `images/03-perform-dualsense.png` |
| Refine | `images/04-refine-phrase.png` |
| Model | `images/05-model-details.png` |

`images/overview.png` is a labeled contact sheet of those five outputs, not a
sixth generated interface. `generation.json` records model/settings and hashes.
The original Listen draft is retained for comparison. The selected revision
fixes its axis pairs but changes navigation: see the explicit review caveats.

`prompts/01-listen.txt` through `prompts/05-model-details.txt` are self-contained GPT Image
briefs. They request designed raster product mockups, not running screenshots.
Use a consistent 1536x1024 or 2048x1152 landscape canvas and inspect legibility,
control count, view hierarchy and device capability labels after generation.
Do not place generated images in the app as functioning controls.

Generation uses the bundled imagegen CLI with `gpt-image-2`, high quality and a
1536x1024 canvas. `prompts/01-listen-revision.txt` records the targeted correction
to the first Listen draft. Do not substitute hand-coded screen renders for
requested image-model output.

On this machine, the private launcher `~/.local/bin/strings-imagegen` reads only
`OPENAI_API_KEY` from the existing `~/.amplifier/keys.env` at execution time.
The credential file stays outside this repository, remains mode 600 and is not
copied into the launcher, command arguments, prompts, manifests or images.
The launcher calls the unmodified bundled CLI, removes OpenAI endpoint
overrides and redacts full-key and key-like error text. Isolated dependencies
live in `~/.cache/strings-imagegen/venv`; the launcher is local configuration,
not a portable project dependency. No global shell startup file was changed.

For example, with that local setup:

```bash
strings-imagegen generate --model gpt-image-2 --quality high \
  --size 1536x1024 --no-augment \
  --prompt-file docs/ux-listening-first/prompts/01-listen.txt \
  --out output/imagegen/listen-new.png
```

This performs a billed API request. Keep keys out of prompts and do not paste
them into chat. Fake-key tests verified output redaction, rejection of public
credential files and missing-key handling; the real-key dry run and image calls
did not print or copy the key.

## Primary capability sources checked November 7, 2026

- Sony DualSense product documentation:
  https://www.playstation.com/en-us/accessories/dualsense-wireless-controller/
- SDL touchpad contacts and normalized positions:
  https://wiki.libsdl.org/SDL3/SDL_GetGamepadTouchpadFinger
- SDL device-specific output packets:
  https://wiki.libsdl.org/SDL3/SDL_SendGamepadEffect

An API with a pressure argument does not prove that a particular controller
reports useful analog pressure. Hardware tests and transport support must be
recorded separately from this design proposal.
