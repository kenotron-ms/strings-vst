from collections import Counter
import math
from pathlib import Path
import sys
import tempfile

import mido

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from render_midi_strings import Note, TempoMap, arrangement, assign_channels, controls, orchestrate, polyphony, read_score


def main():
    tempo = TempoMap(480, [(0, 500000), (480, 800000)])
    assert math.isclose(tempo.at(240), 0.25)
    assert math.isclose(tempo.at(960), 1.3)
    assert math.isclose(TempoMap(480, [(0, 600000), (0, 750000)]).at(480), 0.75)
    with tempfile.TemporaryDirectory(prefix="strings-midi-tests-") as directory:
        source = Path(directory) / "source.mid"
        midi = mido.MidiFile(type=1, ticks_per_beat=480)
        upper = mido.MidiTrack([
            mido.MetaMessage("set_tempo", tempo=500000),
            mido.Message("note_on", note=69, velocity=100),
            mido.Message("note_on", note=65, velocity=90),
            mido.Message("control_change", control=64, value=127, time=120),
            mido.Message("note_on", note=69, velocity=0, time=120),
            mido.Message("note_off", note=65),
            mido.MetaMessage("set_tempo", tempo=800000, time=240),
            mido.Message("note_on", note=72, velocity=80),
            mido.Message("note_off", note=72, time=480),
        ])
        lower = mido.MidiTrack([
            mido.Message("note_on", note=36, velocity=90),
            mido.Message("note_on", note=48, velocity=80),
            mido.Message("note_off", note=36, time=240),
            mido.Message("note_off", note=48),
        ])
        midi.tracks.extend((upper, lower))
        midi.save(source)
        original, notes, metadata, timing, source_cc = read_score(source)
        assert len(notes) == 5 and source_cc[64] == 1
        parts = orchestrate(notes)
        assert [len(part) for part in parts] == [2, 1, 1, 1]
        assert Counter(notes) == Counter(note for part in parts for note in part)
        arranged, sizes = arrangement(original, parts, metadata, timing)
        destination = Path(directory) / "strings.mid"
        arranged.save(destination)
        _, restored, new_meta, new_time, new_cc = read_score(destination)
        identity = lambda note: (note.pitch, note.velocity, note.start, note.end)
        assert Counter(map(identity, notes)) == Counter(map(identity, restored))
        assert new_cc[64] == 0 and new_cc[19] > 0
        assert new_time.at(960) == timing.at(960)
        assert [(tick, message.tempo) for tick, message in new_meta if message.type == "set_tempo"] == [(0, 500000), (480, 800000)]
        assert sizes == [4, 4, 4, 4]
        broken = mido.MidiFile()
        broken.tracks.append(mido.MidiTrack([mido.Message("note_off", note=60)]))
        broken.save(source)
        try:
            read_score(source)
        except ValueError as error:
            assert "Unpaired" in str(error)
        else:
            raise AssertionError("Unpaired note-off accepted")
    overlapping = [Note(0, 0, 60, 80, 0, 480), Note(0, 0, 60, 80, 240, 720)]
    assert polyphony(overlapping) == 2
    assert len({channel for _, channel in assign_channels(overlapping, 0)}) == 2
    curve = controls(overlapping, tempo, 1200, 0, 4)
    assert all(0 <= message.value <= 127 for _, message in curve)
    assert any(message.control == 20 and message.value == 0 for _, message in curve)
    print("PASS: variable tempo, piano split, exact MIDI note roundtrip, pedal filtering, polyphony and overlap-safe channels")


if __name__ == "__main__":
    main()
