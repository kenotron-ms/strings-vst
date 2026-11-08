import argparse
from bisect import bisect_right
from collections import Counter, defaultdict, deque
from dataclasses import dataclass
import hashlib
from importlib.metadata import version
import json
import math
from pathlib import Path
import time
import wave

import mido
import numpy as np


ROOT = Path(__file__).resolve().parents[1]
PARTS = ("violin", "viola", "cello", "double-bass")
PROFILES = ("Violin", "Viola", "Cello", "Double bass")
MIX_LEVELS = (1.0, 0.8, 0.8, 0.7)


@dataclass(frozen=True)
class Note:
    track: int
    channel: int
    pitch: int
    velocity: int
    start: int
    end: int


class TempoMap:
    def __init__(self, ticks_per_beat, changes):
        if ticks_per_beat <= 0:
            raise ValueError("SMPTE time division is not supported")
        self.resolution = ticks_per_beat
        self.ticks = [0]
        self.seconds = [0.0]
        self.tempos = [500000]
        for tick, tempo in sorted(changes, key=lambda entry: entry[0]):
            if tick < 0 or tempo <= 0:
                raise ValueError("Invalid tempo event")
            if tick == self.ticks[-1]:
                self.tempos[-1] = tempo
            else:
                self.seconds.append(self.at(tick))
                self.ticks.append(tick)
                self.tempos.append(tempo)

    def at(self, tick):
        index = bisect_right(self.ticks, tick) - 1
        return self.seconds[index] + (tick - self.ticks[index]) * self.tempos[index] / (self.resolution * 1000000)


def read_score(path):
    midi = mido.MidiFile(path)
    if midi.type not in (0, 1):
        raise ValueError("Only synchronous MIDI files are supported")
    notes, metadata, controllers = [], [], Counter()
    for track_index, track in enumerate(midi.tracks):
        tick = 0
        held = defaultdict(deque)
        for message in track:
            tick += message.time
            if message.is_meta:
                if message.type in ("set_tempo", "time_signature", "key_signature"):
                    metadata.append((tick, message.copy(time=0)))
            elif message.type == "control_change":
                controllers[message.control] += 1
            elif message.type == "note_on" and message.velocity > 0:
                held[message.channel, message.note].append((tick, message.velocity))
            elif message.type in ("note_on", "note_off"):
                key = (message.channel, message.note)
                if not held[key]:
                    raise ValueError(f"Unpaired note-off in track {track_index}")
                start, velocity = held[key].popleft()
                if tick <= start:
                    raise ValueError("Zero-length or reversed note")
                notes.append(Note(track_index, message.channel, message.note, velocity, start, tick))
        if any(held.values()):
            raise ValueError(f"Unreleased MIDI notes in track {track_index}")
    if not notes:
        raise ValueError("MIDI contains no paired notes")
    changes = [(tick, message.tempo) for tick, message in metadata if message.type == "set_tempo"]
    return midi, sorted(notes, key=lambda note: (note.start, note.track, note.pitch)), metadata, TempoMap(midi.ticks_per_beat, changes), controllers


def orchestrate(notes):
    tracks = sorted({note.track for note in notes}, key=lambda track: np.median([note.pitch for note in notes if note.track == track]))
    if len(tracks) != 2:
        raise ValueError("This orchestration expects two note-containing piano-staff tracks")
    bass_track, treble_track = tracks
    groups = defaultdict(list)
    for note in notes:
        groups[note.track, note.start].append(note)
    parts = [[] for _ in PARTS]
    for (track, _), chord in groups.items():
        highest = max(note.pitch for note in chord)
        lowest = min(note.pitch for note in chord)
        for note in chord:
            if track == treble_track:
                part = 0 if note.pitch == highest and note.pitch >= 55 else 1
            else:
                part = 3 if note.pitch == lowest or note.pitch < 36 else 2
            parts[part].append(note)
    if Counter(notes) != Counter(note for part in parts for note in part):
        raise ValueError("Orchestration altered, omitted or duplicated source notes")
    return [sorted(part, key=lambda note: (note.start, note.pitch, note.end)) for part in parts]


def polyphony(notes):
    active, maximum = 0, 0
    for _, delta in sorted([(note.start, 1) for note in notes] + [(note.end, -1) for note in notes]):
        active += delta
        maximum = max(maximum, active)
    return maximum


def assign_channels(notes, preferred):
    ending = defaultdict(lambda: -1)
    assigned = []
    for note in notes:
        choices = [preferred] + [channel for channel in range(16) if channel != preferred]
        channel = next((channel for channel in choices if ending[channel, note.pitch] <= note.start), None)
        if channel is None:
            raise ValueError("More than sixteen overlapping copies of one pitch")
        ending[channel, note.pitch] = note.end
        assigned.append((note, channel))
    return assigned


def controls(notes, tempo_map, stop_tick, part, player_count):
    if not notes:
        return []
    intervals = []
    bridge_gap = round(tempo_map.resolution * 0.18)
    for note in notes:
        if intervals and note.start <= intervals[-1][1] + bridge_gap:
            intervals[-1][1] = max(intervals[-1][1], note.end)
        else:
            intervals.append([note.start, note.end])
    starts = [interval[0] for interval in intervals]
    channel_set = {channel for _, channel in assign_channels(notes, part)}
    events = [(0, mido.Message("control_change", channel=part, control=controller, value=value))
              for controller, value in ((24, part), (25, 127), (26, player_count))]
    cached = {}
    for tick in range(0, stop_tick + 1, max(1, tempo_map.resolution // 24)):
        beat = tick / tempo_map.resolution
        phrase = (beat % 12) / 12
        swell = math.sin(math.pi * phrase) ** 1.4
        energy = 0.43 + 0.17 * swell
        direction = math.tanh(6 * math.cos(math.tau * beat / 8))
        interval = bisect_right(starts, tick) - 1
        gate = interval >= 0 and tick < intervals[interval][1]
        values = {16: round(127 * energy), 17: round(127 * (0.48 + 0.20 * swell)),
                  18: round(127 * (0.34 - 0.09 * swell)),
                  19: round(64 + direction * (63 if direction >= 0 else 64)), 20: 127 if gate else 0}
        for controller, value in values.items():
            if cached.get((part, controller)) != value:
                events.append((tick, mido.Message("control_change", channel=part, control=controller, value=value)))
                cached[part, controller] = value
        vibrato = round(127 * (0.035 + 0.11 * swell))
        for channel in sorted(channel_set):
            if cached.get((channel, 1)) != vibrato:
                events.append((tick, mido.Message("control_change", channel=channel, control=1, value=vibrato)))
                cached[channel, 1] = vibrato
    return events


def arrangement(midi, parts, metadata, tempo_map):
    result = mido.MidiFile(type=1, ticks_per_beat=midi.ticks_per_beat)
    conductor = mido.MidiTrack([mido.MetaMessage("track_name", name="Tempo and meter")])
    previous = 0
    for tick, message in sorted(metadata, key=lambda entry: entry[0]):
        conductor.append(message.copy(time=tick - previous))
        previous = tick
    result.tracks.append(conductor)
    stop_tick = max(note.end for part in parts for note in part)
    sizes = []
    for part_index, notes in enumerate(parts):
        count = max(4, polyphony(notes))
        if count > 16:
            raise ValueError(f"{PARTS[part_index]} exceeds the engine's sixteen-player allocator")
        sizes.append(count)
        events = controls(notes, tempo_map, stop_tick, part_index, count)
        events.append((0, mido.Message("program_change", channel=part_index, program=40 + part_index)))
        for note, channel in assign_channels(notes, part_index):
            events.extend(((note.start, mido.Message("note_on", channel=channel, note=note.pitch, velocity=note.velocity)),
                           (note.end, mido.Message("note_off", channel=channel, note=note.pitch, velocity=0))))
        events.sort(key=lambda entry: (entry[0], 0 if entry[1].type == "control_change" else 1 if entry[1].type == "note_off" else 2))
        track = mido.MidiTrack([mido.MetaMessage("track_name", name=PROFILES[part_index])])
        previous = 0
        for tick, message in events:
            track.append(message.copy(time=tick - previous))
            previous = tick
        result.tracks.append(track)
    return result, sizes


def write_wave(path, audio, sample_rate):
    if not np.all(np.isfinite(audio)) or np.max(np.abs(audio)) >= 1:
        raise ValueError(f"Nonfinite or clipped audio: {path}")
    with wave.open(str(path), "wb") as output:
        output.setnchannels(2)
        output.setsampwidth(2)
        output.setframerate(sample_rate)
        output.writeframes(np.rint(audio.T * 32767).astype("<i2").tobytes())


def render_part(plugin_path, track, tempo_map, profile, players, duration, sample_rate):
    from pedalboard import load_plugin
    plugin = load_plugin(str(plugin_path))
    if not plugin.is_instrument:
        raise ValueError("VST3 is not an instrument")
    plugin([], duration=0.01, sample_rate=sample_rate, reset=True)
    settings = dict(instrument=profile, ensemble=1, section_players=players, articulation="Arco",
                    bow_pressure=0.5, bow_speed=0.65, bow_position=0.35, bow_direction=1.0,
                    bow_engaged=0.0, vibrato_depth=0.0, brightness=0.55, attack=22.0, release=100.0,
                    player_variation=0.16, room=0.12, output=-9.0)
    for name, value in settings.items():
        setattr(plugin, name, value)
    plugin([], duration=0.5, sample_rate=sample_rate, reset=False)
    tick = 0
    final_note_tick = 0
    messages = []
    for message in track:
        tick += message.time
        if message.type == "note_off":
            final_note_tick = max(final_note_tick, tick)
        if not message.is_meta and message.type != "program_change":
            messages.append((message.bytes(), 0.25 + tempo_map.at(tick)))
    final_note = tempo_map.at(final_note_tick)
    for channel in range(16):
        messages.append(([0xB0 | channel, 120, 0], 0.25 + final_note + 1.5))
    messages.sort(key=lambda entry: entry[1])
    blocks = []
    cursor = 0
    total = round(duration * sample_rate)
    started = time.perf_counter()
    for start in range(0, total, sample_rate):
        count = min(sample_rate, total - start)
        local = []
        while cursor < len(messages) and messages[cursor][1] < (start + count) / sample_rate:
            data, stamp = messages[cursor]
            local.append((data, max(0.0, stamp - start / sample_rate)))
            cursor += 1
        blocks.append(plugin(local, duration=count / sample_rate, sample_rate=sample_rate, buffer_size=256, reset=False))
    if cursor != len(messages):
        raise ValueError("Render truncated MIDI events")
    audio = np.concatenate(blocks, axis=1)
    if audio.shape != (2, total) or not np.all(np.isfinite(audio)):
        raise ValueError("Invalid VST3 audio output")
    if np.max(np.abs(audio[:, :round(0.20 * sample_rate)])) != 0:
        raise ValueError("VST3 played before its MIDI timestamp")
    if np.max(np.abs(audio[:, -sample_rate // 4:])) > 0.00001:
        raise ValueError("VST3 retained audio after release/panic")
    measurement = dict(profile=profile, players=players, peak=float(np.max(np.abs(audio))),
                       rms=float(np.sqrt(np.mean(audio.astype(np.float64) ** 2))),
                       render_seconds=time.perf_counter() - started, midi_messages=len(messages),
                       final_quarter_second_peak=float(np.max(np.abs(audio[:, -sample_rate // 4:]))))
    if measurement["rms"] < 0.0001 or measurement["peak"] >= 1:
        raise ValueError(f"Silent or clipped VST3 output: {profile}")
    return audio, measurement


def main():
    parser = argparse.ArgumentParser(description="Orchestrate a two-staff piano MIDI and render four actual Strings VST3 instances")
    parser.add_argument("midi", type=Path)
    parser.add_argument("--plugin", type=Path, default=ROOT / "build/bin/Strings.vst3")
    parser.add_argument("--output", type=Path, default=ROOT / "build/musescore-strings/hes-a-pirate/rendered")
    parser.add_argument("--plan-only", action="store_true")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    if (args.output / "render.json").exists():
        raise ValueError("Refusing to overwrite a previous render; choose another --output")
    midi, notes, metadata, tempo_map, source_cc = read_score(args.midi)
    parts = orchestrate(notes)
    arranged, sizes = arrangement(midi, parts, metadata, tempo_map)
    arranged_path = args.output / "hes-a-pirate-strings.mid"
    arranged.save(arranged_path)
    _, roundtrip, _, _, _ = read_score(arranged_path)
    identity = lambda note: (note.pitch, note.velocity, note.start, note.end)
    if Counter(map(identity, notes)) != Counter(map(identity, roundtrip)):
        raise ValueError("Exported arrangement did not preserve note pitches, velocities and timing")
    plan = dict(source=str(args.midi.resolve()), source_sha256=hashlib.sha256(args.midi.read_bytes()).hexdigest(),
                source_notes=len(notes), source_duration_seconds=midi.length,
                tempo_events=sum(message.type == "set_tempo" for _, message in metadata),
                original_controller_counts=dict(source_cc),
                parts=[dict(name=name, notes=len(part), max_polyphony=polyphony(part), players=players,
                            range=[min(note.pitch for note in part), max(note.pitch for note in part)] if part else None)
                       for name, part, players in zip(PARTS, parts, sizes)],
                note_preservation="Exact multiset of pitch, velocity, start tick and end tick verified after MIDI roundtrip",
                assistance="Deterministic four-bar bow-energy/contact/vibrato arcs and smooth eight-beat direction cycles; no trained model",
                controller_policy="Source piano CC64 damper and other piano CCs are not copied; written note-off durations remain unchanged",
                routing="Each string track feeds a separate Strings VST3 instance; do not merge the instrument-configuration CCs into one instance")
    (args.output / "plan.json").write_text(json.dumps(plan, indent=2) + "\n")
    print(f"Preserved {len(notes)} notes and {plan['tempo_events']} tempo events", flush=True)
    if args.plan_only:
        return
    sample_rate = 48000
    duration = math.ceil((max(midi.length, tempo_map.at(max(note.end for note in notes))) + 2.75) * sample_rate) / sample_rate
    report = dict(status="rendering", plan=plan, sample_rate=sample_rate, duration_seconds=duration,
                  host=f"Pedalboard {version('pedalboard')}", stems=[])
    report_path = args.output / "render.json"
    report_path.write_text(json.dumps(report, indent=2) + "\n")
    mixed = np.zeros((2, round(duration * sample_rate)), dtype=np.float64)
    for part_index, (name, profile, players, level) in enumerate(zip(PARTS, PROFILES, sizes, MIX_LEVELS)):
        audio, measurement = render_part(args.plugin, arranged.tracks[part_index + 1], tempo_map, profile, players, duration, sample_rate)
        stem = args.output / f"{name}.wav"
        write_wave(stem, audio, sample_rate)
        mixed += level * audio
        measurement.update(file=stem.name, mix_level=level, sha256=hashlib.sha256(stem.read_bytes()).hexdigest())
        report["stems"].append(measurement)
        report_path.write_text(json.dumps(report, indent=2) + "\n")
        print(f"Rendered {name}: {measurement['render_seconds']:.2f}s, peak {measurement['peak']:.4f}", flush=True)
    gain = 0.82 / float(np.max(np.abs(mixed)))
    master = args.output / "hes-a-pirate-physical-strings.wav"
    write_wave(master, mixed * gain, sample_rate)
    modules = {str(path.resolve()): hashlib.sha256(path.read_bytes()).hexdigest() for path in args.plugin.rglob("*.so")}
    if not modules:
        raise ValueError("No native VST3 module fingerprint found")
    report.update(status="passed", plugin_sha256=modules, master=master.name,
                  master_sha256=hashlib.sha256(master.read_bytes()).hexdigest(),
                  master_gain_db=20 * math.log10(gain), master_peak=0.82,
                  arranged_midi_sha256=hashlib.sha256(arranged_path.read_bytes()).hexdigest(),
                  limits=["Offline actual VST3 hosting, not standalone playback or live controller use",
                          "Synthetic physical profiles; no human acoustic-realism judgment",
                          "Piano damper is not held bow; register-based orchestration retains written MIDI note-off timing",
                          "Lowest D1 is below the default bass open E1, equivalent to a lower-tuned/extended register",
                          "No sampled strings, replacement synth, external reverb, pitch correction or hidden audio layer"])
    report_path.write_text(json.dumps(report, indent=2) + "\n")
    print(master, flush=True)


if __name__ == "__main__":
    main()
