from pathlib import Path
import struct
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from standalone_workflow import DURATION, MELODY, OFFSETS, capture_errors, score, variable_length, write_midi


def main():
    parts = score()
    assert len(parts) == 4 and len(MELODY) == 16 and 22 < DURATION < 24
    assert [part["name"] for part in parts] == ["violin", "viola", "cello", "double-bass"]
    for part in parts:
        for note in part["notes"]:
            assert note["midi"] == part["base"] + OFFSETS[note["key"]]
            assert 0 <= note["start"] < DURATION and 0 < note["duration"]
            assert note["start"] + note["duration"] <= DURATION + 1e-6
        for key in OFFSETS:
            ordered = sorted((note for note in part["notes"] if note["key"] == key), key=lambda note: note["start"])
            for before, after in zip(ordered, ordered[1:]):
                assert before["start"] + before["duration"] <= after["start"], "Overlapping identical keys would lose note-offs"
    assert variable_length(0) == b"\x00" and variable_length(127) == b"\x7f"
    assert variable_length(128) == b"\x81\x00" and variable_length(16384) == b"\x81\x80\x00"
    assert not capture_errors("no message buffer overruns\nCannot lock memory")
    assert len(capture_errors("JackEngine::XRun: client = Strings\nJackAudioDriver: Process error")) == 2
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "cue.mid"
        write_midi(path, parts)
        data = path.read_bytes()
        assert data[:14] == b"MThd" + struct.pack(">IHHH", 6, 1, 4, 480)
        position, tracks = 14, 0
        while position < len(data):
            assert data[position:position + 4] == b"MTrk"
            length = struct.unpack(">I", data[position + 4:position + 8])[0]
            track = data[position + 8:position + 8 + length]
            assert len(track) == length and track.endswith(b"\x00\xff\x2f\x00")
            position += 8 + length
            tracks += 1
        assert position == len(data) and tracks == 4
    print("PASS: original cue bounds, playable keyboard scheduling and four-track MIDI structure")


if __name__ == "__main__":
    main()
