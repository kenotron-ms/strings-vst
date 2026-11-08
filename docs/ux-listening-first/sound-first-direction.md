# Sound first: play notes, shape the performance

November 7, 2026. Product-direction amendment from the user, not an implemented
feature claim or a revision of an approved goal condition.

## Product promise

Produce convincing, controllable synthetic string performances through a
physical instrument model and assistive performance algorithms. A person should
not need notation literacy, violin technique, a pressure-sensitive keyboard or
an expressive controller to begin. Additional devices increase expressive
control; they must not be prerequisites for a musically useful sound.

## Represent events, not an engraved score

Use a compact **note ribbon**: horizontal position is onset, length is duration,
and vertical position is pitch. Fold to the active register; reveal note names
and a light beat/time ruler on demand. Preserve gaps, repeated notes and chords.
A current-note label and playhead provide orientation. Do not turn discrete
pitch changes into a misleading continuous pitch contour.

Below the notes, show one optional **expression sparkline** for the selected
control. It represents the actual applied expression, not an invented audio
waveform or a realism indicator. Reveal other lanes only when editing them.
Use labels/line styles as well as color. In an empty session, show how to play
or record rather than fabricate an example take.

No automatic staff engraving, transcription, rhythmic spelling or score export
is required. A full piano roll can remain in the host DAW. In plugin mode, do
not imply access to the host's entire MIDI clip: display received/captured events
or an explicitly imported phrase. Standalone capture is a separate ownership
path. The user's word “spark” has not been tied to a specific confirmed product;
this proposal adopts compact visual summaries, not an alleged Spark feature.

## Three independent layers

1. **Notes:** MIDI keyboard, computer keys, imported phrase or host playback
   supplies pitch/onset/release. Preserve the source notes unless the user
   explicitly edits them. Computer-key input uses a chosen attack level and
   register; it must not pretend to measure key velocity or pressure. Ignore
   key-repeat and text-entry shortcuts; release owned notes on focus loss.
2. **Performance:** an assisted player proposes bounded bow force, signed
   motion, engagement, contact, transitions and vibrato. A held note can develop
   without another input device. Detect phrase connections without making all
   short notes legato. Keep detached/repeated notes intentional. Rule-based
   assistance and a genuinely trained model must be labeled differently.
3. **Instrument:** applied performance drives the physical excitation and
   string/bridge/body system, not just an output-volume envelope. Engine flaws
   cannot be concealed by more elaborate graphics, reverberation or “AI” labels.

Provide **Assisted** by default and optional **Direct** physical control. Manual
expression may replace a selected assisted lane, with explicit ownership and a
smooth bounded handover. Do not let a mouse, automation and the assistant fight
over one parameter. Loss of an optional device must never leave owned input
stuck; define whether playback pauses or assisted control resumes, rather than
silently changing behavior.

## Ordinary-input workflows

- **Computer only:** enable Play keys, choose a register, play a melody, and hear
  an assisted performance. Record notes, then optionally add expression on a
  second mouse/touchpad pass. No simultaneous two-hand coordination required.
- **Basic MIDI keyboard:** play notes with attack velocity where available.
  Let assisted phrasing supply continuous controls; a mod wheel/pedal is useful
  but optional. A long held key should not require constant controller motion.
- **Extra controls:** optionally map mouse/touchpad, knobs, sticks or triggers to
  expression. State that position/velocity mappings are not measured pressure.
  Advanced DualSense feedback remains optional and separately capability-tested.
- **Recorded refinement:** audition the same notes, suggest a performance, compare
  original/suggested, and edit only the phrase peak or release. Preserve the raw
  notes and manual overrides; suggestion must be undoable and repeatable.

Live assistance must be causal or expose its bounded analysis delay. Recorded
interpretation may inspect later notes; do not promise identical live decisions
without future context. Any learned physical controller needs appropriate
training targets, validated output bounds, data rights and real-time cost checks;
an audio-synthesis checkpoint is not automatically a bow-control policy.

## Primary precedents checked

- Ableton Live: pitch/time note blocks, optional expression editors and a compact
  clip overview. Adopt the representation, not an entire DAW inside the plugin.
  https://www.ableton.com/en/manual/editing-midi/
- Ableton's computer MIDI keyboard: explicit octave and attack-velocity settings.
  https://www.ableton.com/en/manual/live-keyboard-shortcuts/
- GarageBand Strings: note bars/scale views and approachable gesture mappings.
  This is an interaction precedent, not proof of a physically modeled engine.
  https://support.apple.com/en-ke/guide/garageband-iphone/chsf2f99a20/ios
- Audio Modeling: PolySWAM supports ordinary keyboard workflows, and its GUI
  separates expression/macros from note allocation and advanced pages. Its
  documentation also exposes an analysis-window/added-latency trade-off.
  https://kb.audiomodeling.com/support/solutions/articles/206000082785-how-do-you-play-polyswam-do-i-still-need-an-expressive-controller-
  https://kb.audiomodeling.com/support/solutions/articles/206000082864-understanding-the-polyswam-gui
- MIDI-DDSP: notes/performance/synthesis hierarchy with learned suggestions and
  editable expression. Its synthesis controls are not our physical controls.
  https://midi-ddsp.github.io/
- Roberts and Wakefield, NIME 2017: animated sparklines for continuous modulation
  in gibberwocky. This is a control-display precedent, not a note-transcription
  system or evidence of bowed-string realism.
  https://www.nime.org/proceedings/2017/nime2017_paper0024.pdf

## What should determine quality

Prioritize exposed solo sound, held-note development, repeated-note attacks,
connected transitions, releases and ensemble coherence. Compare assisted output
with the same notes under plain control and with suitable acoustic references,
using level-controlled listening and composer/player assessment as well as
numerical checks. Report pitch/stability/latency defects separately; a numerical
pass or preference for louder audio does not establish realism. UI success is
whether a keyboard-only newcomer can hear, shape and revise a convincing phrase,
not whether the screen resembles sheet music or a violin.
