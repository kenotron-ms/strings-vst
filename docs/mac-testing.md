# Try the research/performance build on a Mac

## Download the right build

The older tagged release does not contain the new physical engine and knob-led
editor. Until a new release is tagged, use the repository's **Actions** tab:

1. Open **Build, validate and release** and select a successful **push** run on
   `main` for the merged commit you want to test, not an unmerged PR run.
2. In **Artifacts**, download `Strings-macos-universal`. GitHub requires you to
   sign in to download workflow artifacts; these downloads expire.
3. Extract the downloaded artifact, then extract the enclosed
   `Strings-0.3.1-macos-universal.zip`.
4. The inner folder contains `Strings.vst3`, `Strings.clap`, the standalone
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
   Vibrato, then open Instrument details for the individual physical controls.
5. With the editor focused, `A S D F G H J K` also plays notes. Audition plays
   the editable local phrase; it is not synchronized to your DAW transport.

For standalone playback, open `Strings.app` if provided; otherwise launch the
`Strings` executable from Terminal. The native keyboard and Audition work
without an external MIDI device. Computer-key input requires editor focus.

If macOS blocks the download, use its security approval flow only after
verifying the source and build identity. Advanced users can remove quarantine
from this specific trusted extracted bundle with
`xattr -dr com.apple.quarantine /path/to/Strings.vst3` (or the specific
standalone executable/app). Do not disable system-wide Gatekeeper checks.

## Current boundaries

- Physical modeling is a reduced research implementation with synthetic body
  profiles, not a calibrated acoustic instrument or commercial-quality claim.
- Local phrase edits and expression takes are scratch tools: they are not
  persisted as a DAW MIDI clip. Record pitch and automation in your DAW.
- Game controllers require the browser/MIDI adapter described in
  `input-devices.md`; native controller discovery and DualSense haptics are not
  implemented.
- Prior Linux validation observed non-fatal DPF lifecycle warnings. Consult the
  workflow's validation logs and audio measurements for each Mac build rather
  than assuming Linux results prove Mac behavior.
