# MuseScore → MIDI → physical Strings VST3

This workflow uses a real MuseScore export as musical input to **four actual
instances of the built Strings VST3**. It does not synthesize a stand-in tone,
use MuseScore's piano audio or substitute the earlier original demo melody.
The source arrangement is onnikoivisto's community piano arrangement of
*He's a Pirate*. Download provenance is retained locally beside its source.
The downloaded score, derived MIDI and audio are local build artifacts, not
added to the public source repository or included in the earlier packages.

## Conversion

The source is `build/score-sources/hes-a-pirate/hes-a-pirate-onnikoivisto.mscz`.
Its two staves contain 87 measures. The distro's MuseScore 3.2.3 refused this
newer score format; the official 3.6.2 AppImage successfully exports it.
An isolated configuration and temporary Xvfb display keep this conversion
separate from a user's normal editor setup. `-s` disables MuseScore synthesis:
only the MIDI export, not a piano recording, becomes input to Strings.

```sh
QT_QPA_PLATFORM=xcb LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a \
  /tmp/strings-musescore-3.6.2/squashfs-root/AppRun -s \
  -o build/score-sources/hes-a-pirate/hes-a-pirate.mid \
  build/score-sources/hes-a-pirate/hes-a-pirate-onnikoivisto.mscz
```

MuseScore's export contains **1,285 paired notes**, 480 ticks/quarter and nine
tempo events, about 78.54 seconds. Three tied continuations explain why the
source's 1,288 noteheads yield fewer note attacks. The original MIDI, including
its piano-controller data, is retained unchanged.

## Orchestration and assistance

`tools/render_midi_strings.py` separates the treble and bass staves by register.
The treble chord's top voice goes to violin and the remaining notes to viola.
The bass chord's lowest voice and notes below cello range go to double bass;
the other accompaniment notes go to cello. No pitch transposition is used.
The lowest D1 is below the default bass's open E1: this exercises a lower-tuned
or extended register, not an assertion of a calibrated standard four-string bass.

Before rendering, the four-track MIDI is reloaded and checked against the
original for **every pitch, velocity, onset tick and note-off tick**, including
duplicates. Tempo/meter metadata is copied. Per-part peak polyphony must fit
the engine's player allocator. Overlapping repetitions of the same pitch use
different MIDI channels so one note-off does not erase another held copy.

Assistance is transparent and deterministic: four-bar energy/contact/vibrato
arcs and smooth eight-beat bow-direction cycles control the real physical bow
parameters using MIDI CCs. This is authored open-loop phrasing, not a trained
performer, inferred professional bowing or emotion recognition. The model's
nonlinear contact, two-polarization strings and reciprocal body coupling still
generate the audio. No physical model or calibrated preset was changed for this
song.

Piano damper CC64 and other piano-specific controller settings are **not**
copied into the string performance. The original written MIDI note-off timing
is retained rather than interpreting held piano sustain as a held bow. This
changes piano sustain semantics; it does not drop the score's note events.

## Render and inspect

Dependencies: Python with NumPy, Mido 1.3.3 and Pedalboard 0.9.26, plus the built
Linux VST3. The existing test environment is `/tmp/strings-test-env`.

```sh
python tests/midi_strings_tests.py
python tools/render_midi_strings.py \
  build/score-sources/hes-a-pirate/hes-a-pirate.mid --plan-only
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a python tools/render_midi_strings.py \
  build/score-sources/hes-a-pirate/hes-a-pirate.mid
python tests/midi_strings_audio_audit.py \
  build/musescore-strings/hes-a-pirate/rendered
```

The tool refuses to overwrite an existing `render.json`; use a new `--output`
directory for another take. A failed attempt's artifacts are not silently
relabelled as a successful session.

Outputs in `build/musescore-strings/hes-a-pirate/rendered/`:

- `hes-a-pirate-physical-strings.wav`: 48 kHz stereo master, PCM 16-bit.
- `violin.wav`, `viola.wav`, `cello.wav`, `double-bass.wav`: untouched VST3 stems.
- `hes-a-pirate-strings.mid`: four-part performance, physical expression CCs
  and original tempo map. In a DAW, route **each track to a separate Strings
  instance**; merging instrument-configuration CCs into one instance conflicts.
- `plan.json`: role assignment, note counts, ranges and controller policy.
- `render.json`: actual host, VST3 fingerprint, render times, stems and mix gain.
- `audio-audit.json`: independent raw-MIDI event comparison and audio checks.

The master is only a fixed stem balance plus constant gain to peak 0.82. Native
room is part of each instance; no external reverb, sampled strings, alternate
synth, pitch correction or hidden layer is added. Original MIDI onsets have a
250 ms audio lead-in and the render retains a release tail. Processing is
offline; no live hardware, standalone-file import or human listening study is
claimed. The numerical audit checks identity, nonclipping, silence, duration
and mix reconstruction, not professional realism or full audio transcription.

## Completed local run — November 8, 2026

The conversion, MIDI unit tests, four-part VST3 render and independent audio
audit all passed. The exported performance retains all 1,285 source note pairs
and all nine tempo events. Part allocation is 273 violin, 320 viola, 346 cello
and 346 double-bass notes. Peak simultaneous note counts are 1/3/1/1; four
players in each instance cover them without exceeding the allocator.

The final WAV is 81.294 seconds including lead-in and release, stereo 48 kHz,
PCM 16-bit. Master gain is a constant +6.359 dB and peak is approximately 0.82.
The audit independently compares raw MIDI note events, checks every stem and
the master, and reconstructs the mix within its PCM quantization allowance.
This verifies the rendered files, not a human judgment of acoustic realism.

Actual VST3 module SHA-256:
`17e5370ffe6bdc07484971c347c2b5896cee35254dc300b83630ac397cb91869`.
Final WAV SHA-256:
`1bd85b3babe4b61a17d8c04827d613e4fd802064e71da84c38b9f26bed9605c1`.
`conversion.json`, the exporter logs and `render.log` are retained in the
parent `build/musescore-strings/hes-a-pirate/` folder. The renderer filters the
MIDI's General MIDI program labels before sending events to Strings; actual
profiles are set explicitly and through the engine's configuration CCs.

The host emits the previously documented nonfatal DPF lifecycle assertions and
active-component deletion warnings. They are retained in `render.log`; this
is not a warning-free host certification. All render/export processes and their
temporary Xvfb displays exited. No score, audio or credential was uploaded.

The pre-existing 0.3.1 package inventory predates these new render tools. This
local follow-up does not imply a new release or updated archive certification.
