# Install Strings 0.3.2 on a Mac

## Download the right build

Download `Strings-0.3.2-macos-universal.zip` from the repository's
[v0.3.2 release](https://github.com/kenotron-ms/strings-vst/releases/tag/v0.3.2).
Download `SHA256SUMS.txt` from the same release to check the archive before installing.
This is a release download, not an expiring Actions artifact.

1. Extract the release zip.
2. The inner folder contains `Strings.vst3`, `Strings.clap`, the standalone
   `Strings` executable (or `Strings.app`), examples and documentation.
   `docs/build-info.json` identifies the exact source commit and workflow run.

CI checks both Apple Silicon (`arm64`) and Intel (`x86_64`) slices. The minimum
deployment target is macOS 11. This is an experimental, ad-hoc-signed build,
**not Apple-notarized** and not a claim of compatibility with every DAW.

## Install and play

1. Quit your DAW. In Finder, choose **Go → Go to Folder**, enter
   `~/Library/Audio/Plug-Ins/`, and create `VST3` if needed.
2. Copy the **entire** `Strings.vst3` bundle into that `VST3` folder. Do not copy
   only its inner executable. CLAP hosts can instead use `Strings.clap` in
   `~/Library/Audio/Plug-Ins/CLAP/`.
3. Reopen a VST3-capable DAW, rescan plug-ins, and add Strings to an instrument
   track. **Logic Pro cannot load these formats: there is no Audio Unit yet.**
4. Play your MIDI keyboard. Start in Solo with Violin; shape Energy, Tone and
   Vibrato, then open Details for the individual physical controls.
5. With the editor focused, `A S D F G H J K` also plays notes. Version 0.3.2
   includes an 88-key A0–C8 piano, `W E T Y U` accidentals,
   `Z`/`X` typing-span changes, velocity adjustment and Release.
   Replace the older 0.3.1 build to get this keyboard and simplified editor.
   The current editor is a live instrument: no note plot, demo transport or bow-take recorder.
   Use the host's computer-MIDI keyboard, MIDI editor or external MIDI to record
   host clips: plugin clicked/typed keys audition internally and have no MIDI output.

For standalone playback, open `Strings.app` if provided; otherwise launch the
`Strings` executable from Terminal. The native keyboard works
without an external MIDI device. Computer-key input requires editor focus.
Native standalone melody recording, durable takes and WAV/MIDI export are not
implemented. See the [live instrument screen](ux-components/live-instrument.md)
for the current scope; use the host to arrange and record a track.

If macOS blocks the download, use its security approval flow only after
verifying the source and build identity. Advanced users can remove quarantine
from this specific trusted extracted bundle with
`xattr -dr com.apple.quarantine /path/to/Strings.vst3` (or the specific
standalone executable/app). Do not disable system-wide Gatekeeper checks.

## Quick REAPER test

1. After copying `Strings.vst3`, restart REAPER. If Strings does not appear,
   rescan VST plugins in REAPER's preferences.
2. Insert a virtual instrument on a new track and select the **VST3** Strings
   instrument. The full piano and Energy/Tone/Vibrato view should appear.
3. Click the plugin piano to audition. For notes that can be recorded into a
   REAPER MIDI item, use REAPER's virtual MIDI keyboard or an external MIDI input,
   arm the track and enable input monitoring. The plugin piano is audition only.
4. Record/play/stop in REAPER. Save the REAPER project and render audio there;
   Strings deliberately does not duplicate a DAW's timeline or recorder.

## Current boundaries

- Physical modeling is a reduced research implementation with synthetic body
  profiles, not a calibrated acoustic instrument or commercial-quality claim.
- There is no native note plot, demo transport or expression-take recorder.
  Record pitch and automation in your DAW.
- Game controllers require the browser/MIDI adapter described in
  `input-devices.md`; native controller discovery and DualSense haptics are not
  implemented.
- Prior Linux validation observed non-fatal DPF lifecycle warnings. Consult the
  workflow's validation logs and audio measurements for each Mac build rather
  than assuming Linux results prove Mac behavior.
