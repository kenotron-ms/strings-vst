# Expressive inputs — implemented and proposed

These features belong to the **local 0.3 research/performance build**, not published 0.1.0.
Pitch and expression are independent: a held note chooses the stopped string;
force, signed motion, contact and lift drive its bow junction.

## Working routes

| Input | Implemented route | Boundary |
| --- | --- | --- |
| MIDI / computer keyboard + knobs | Native Keys front panel: Energy assigns force/speed, Tone assigns contact/brightness, Vibrato controls pitch modulation; drag/Shift/scroll/reset | Authored open-loop macros, not a trained performer. Pitch uses DAW MIDI or focused A–K keys. Scratch takes capture bow commands only, not Tone/Vibrato. |
| Mouse / ordinary touchpad | Native direct expression pad: independent pitch, held X position sets signed speed, Y assigns weight; release lifts | Position is assigned, not pressure sensed. Stationary held positions sustain their command; center rests. Focus loss releases owned notes/bow. |
| Browser pointer / keyboard | Five interactive studies; on-screen/computer pitch, editable controls, record/replay/export | No embedded DSP. Optional armed Web MIDI drives the native plugin via a DAW/virtual port. |
| Standard Xbox / DualSense gamepad | Browser Gamepad API: LT engagement, RT force, left X signed velocity, right X contact, right Y vibrato | Neutral capture/deadzone, mapping check and safe re-arm. Synthetic gamepad tested; a physical controller has not been exercised here. |
| DAW MIDI / automation | Notes, pitch bend, sustain, pressure, expression and CC16–28 | Global expression controls, not full MPE or arbitrary MIDI learn. |

Browser control values and exported takes use normalized requests, not newtons
or measured bow speed. Native bow position runs from bridge (0) toward
fingerboard (1); the browser's contact scale is inverted at its MIDI adapter.
See `ux-performance/design.md` for the complete channel-1 CC contract.

The native performance screen is documented in `ux-components/native-performance.md`.
Its filled dot/trail use smoothed engine command outputs, independently of the
pointer target. Controller mode shows the adapter mapping; it does not claim
native device discovery. Its external MIDI stream should be recorded in the
DAW, not the native pointer scratch-take recorder. This changes only the native pad's old relative-motion behavior;
the five earlier browser studies keep their own documented gesture semantics.

## Run the controller studies

1. Build the **new** `Strings.vst3`, load it on a DAW instrument track, disable
   other live MIDI input on that track, and use a safe monitor level.
2. Create a dedicated virtual MIDI port using your OS's MIDI routing facility.
   Route that port's channel 1 into the track. Do not create a MIDI feedback loop.
3. Serve the repository root: `python3 -m http.server 8765 --bind 127.0.0.1`.
   Open `http://localhost:8765/docs/ux-performance/` in a Web MIDI capable browser.
4. Connect MIDI explicitly, choose the dedicated output, then arm it. Permission,
   virtual-port setup and actual browser-to-DAW live routing are OS/host-specific
   and were not physically verified here. The automated fixture captures the
   same adapter bytes and renders them through the actual VST3 offline.
5. Supply pitch with the study's keys; use pointer expression or choose a
   standard gamepad, rest sticks and capture neutral. Return triggers/sticks to
   rest after panic/focus loss before re-engaging.
6. Stop/disarm before changing routes. Use DAW panic if delivery is interrupted.
   Stop the local server with Ctrl-C when done.

The output is opt-in and off by default. CCs are cached and rate-limited.
MIDI expression is not automatically recorded as host automation. DPF notification
requests are best effort; Pedalboard's parameter readback stayed cached after CC
edits. Record/replay the MIDI stream itself and test the chosen DAW's behavior.
Release, focus loss, page hiding, device loss and source changes send bow zeros,
note-off, all-notes-off and panic when a port is available. A disconnected port
cannot receive panic; the DAW's own panic remains necessary in that failure.
Tested safety cases and captured traces are in `research-validation.md`.

## Not implemented

Pressure sensing for pens/trackpads, raw multitouch, adaptive DualSense triggers,
controller haptics, native SDL hotplug, arbitrary MIDI learn and gamepad-only
scale/fingerboard modes remain future work. A standard Gamepad profile does not
expose all device-specific features. Unknown mappings are disabled rather than
guessed. Do not mistake a generic PointerEvent pressure value for a sensor.

The five studies are alternative workflows, not five completed native editors.
Their gamepad mapping is direct physical expression, not emotion recognition.
Mood is an explicit open-loop recipe. No controller knows the composer's intent.

## Primary capability references

- W3C Gamepad: https://www.w3.org/TR/gamepad/
- W3C Pointer Events: https://www.w3.org/TR/pointerevents3/
- W3C Web MIDI: https://www.w3.org/TR/webmidi/
- SDL3 optional future integration: https://wiki.libsdl.org/SDL3/CategoryGamepad
