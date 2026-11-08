import functools
import http.server
import json
import os
from pathlib import Path
import threading
import wave

import numpy as np
from pedalboard import load_plugin
from playwright.sync_api import sync_playwright


ROOT = Path(__file__).resolve().parents[1]
REPORTS = ROOT / "build/research-validation"

DEVICE_FIXTURE = """
window.testMessages = [];
window.testZero = performance.now();
window.testPad = null;
window.testPort = {
  id: 'synthetic-test-port', name: 'Synthetic test MIDI output',
  state: 'connected', connection: 'open',
  open: async function() { return this; },
  send: function(data) {
    window.testMessages.push({data:Array.from(data),time:(performance.now()-window.testZero)/1000});
  }
};
window.testAccess = { outputs:new Map([[window.testPort.id,window.testPort]]), onstatechange:null };
Object.defineProperty(navigator,'requestMIDIAccess',{value:async()=>window.testAccess});
Object.defineProperty(navigator,'getGamepads',{value:()=>window.testPad?[window.testPad]:[]});
"""


class QuietHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, *_):
        pass


def audio_from_trace(name, trace):
    plugin = load_plugin(str(ROOT / "build/bin/Strings.vst3"))
    plugin.room = 0
    events = [(message["data"], max(0, message["time"])) for message in trace]
    duration = max(2, events[-1][1] + 1.0)
    audio = plugin(events, duration=duration, sample_rate=48000, buffer_size=256)
    assert np.all(np.isfinite(audio)) and np.max(np.abs(audio)) < 1
    rms = float(np.sqrt(np.mean(audio ** 2)))
    assert rms > 0.0005, f"{name}: browser expression did not produce audible native audio: {rms}"
    assert np.max(np.abs(audio[:, -24000:])) < 0.00001, f"{name}: panic left an audible tail"
    path = REPORTS / f"{name}.wav"
    with wave.open(str(path), "wb") as output:
        output.setnchannels(2)
        output.setsampwidth(2)
        output.setframerate(48000)
        output.writeframes((audio.T * 32767).astype("<i2").tobytes())
    return {"rms": rms, "peak": float(np.max(np.abs(audio))), "audio": str(path.relative_to(ROOT))}


def arm(page):
    page.locator("#midi-connect").click()
    page.locator("#midi-output").select_option("synthetic-test-port")
    page.locator("#midi-arm").check()
    page.wait_for_timeout(100)
    page.evaluate("window.testMessages=[];window.testZero=performance.now()")


def require_release(page):
    page.evaluate("window.dispatchEvent(new Event('blur'))")
    page.wait_for_timeout(150)
    trace = page.evaluate("window.testMessages")
    assert any(message["data"][0] == 0x90 for message in trace), "No browser note-on"
    for controller in (16, 17, 20):
        values = [message["data"][2] for message in trace if message["data"][:2] == [0xB0, controller]]
        assert values and values[-1] == 0, f"CC{controller} failed to release"
    assert any(message["data"] == [0xB0, 120, 0] for message in trace), "No panic on blur"
    return trace


def main():
    REPORTS.mkdir(parents=True, exist_ok=True)
    screenshots = ROOT / "docs/ux-performance/screenshots"
    screenshots.mkdir(exist_ok=True)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(QuietHandler, directory=str(ROOT)))
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    results = []
    try:
        with sync_playwright() as playwright:
            options = {"headless": True}
            if os.environ.get("PLAYWRIGHT_CHROMIUM_EXECUTABLE"):
                options["executable_path"] = os.environ["PLAYWRIGHT_CHROMIUM_EXECUTABLE"]
            browser = playwright.chromium.launch(**options)
            context = browser.new_context(viewport={"width": 1440, "height": 1080}, reduced_motion="reduce")
            context.add_init_script(DEVICE_FIXTURE)
            for view in ("bow", "mood", "phrase", "conductor", "material"):
                page = context.new_page()
                errors = []
                page.on("pageerror", lambda error: errors.append(str(error)))
                page.goto(f"http://127.0.0.1:{server.server_port}/docs/ux-performance/index.html?view={view}")
                arm(page)
                if view == "phrase":
                    page.locator("#phrase-play").click()
                elif view in ("bow", "mood"):
                    page.locator("#gesture").focus()
                    page.keyboard.down("g")
                    page.keyboard.down("Space")
                else:
                    page.locator("#engage").focus()
                    page.keyboard.down("g")
                    page.keyboard.down("Space")
                page.wait_for_timeout(1800)
                page.screenshot(path=str(screenshots / f"{view}.png"), full_page=True)
                trace = require_release(page)
                page.keyboard.up("Space")
                page.keyboard.up("g")
                assert not errors, errors
                (REPORTS / f"{view}-midi.json").write_text(json.dumps(trace, indent=2))
                results.append({"view": view, "messages": len(trace), **audio_from_trace(view, trace)})
                page.set_viewport_size({"width": 390, "height": 844})
                assert page.evaluate("document.documentElement.scrollWidth <= innerWidth"), f"{view}: mobile overflow"
                page.close()
            page = context.new_page()
            page.goto(f"http://127.0.0.1:{server.server_port}/docs/ux-performance/index.html?view=bow")
            arm(page)
            page.evaluate("""window.testPad={id:'Synthetic standard pad',index:0,mapping:'standard',connected:true,
              axes:[0,0,0,0],buttons:Array.from({length:17},()=>({value:0,pressed:false,touched:false}))};
              window.dispatchEvent(new Event('gamepadconnected'));""")
            page.locator("#input-source").select_option("gamepad")
            page.locator("#gesture").focus()
            page.keyboard.down("g")
            page.wait_for_timeout(150)
            page.evaluate("testPad.axes=[.55,0,.3,-.2];testPad.buttons[6].value=.7;testPad.buttons[7].value=.85")
            page.wait_for_timeout(1000)
            assert page.locator("#live-force").inner_text() == "85%"
            page.evaluate("testPad.axes[0]=-.55")
            page.wait_for_timeout(500)
            before_blur = page.evaluate("testMessages.length")
            trace = require_release(page)
            page.wait_for_timeout(200)
            after = page.evaluate("index => testMessages.slice(index)", before_blur)
            assert not any(item["data"][:2] == [0xB0, 20] and item["data"][2] > 0 for item in after), "Held trigger re-armed after blur"
            page.evaluate("testPad.buttons[6].value=0")
            page.wait_for_timeout(100)
            page.evaluate("testPad.buttons[6].value=.7")
            page.keyboard.up("g")
            page.keyboard.down("g")
            page.wait_for_timeout(300)
            page.evaluate("testPad=null;window.dispatchEvent(new Event('gamepaddisconnected'))")
            page.wait_for_timeout(150)
            trace = page.evaluate("testMessages")
            assert page.locator("#live-force").inner_text() == "0%"
            directions = [item["data"][2] for item in trace if item["data"][:2] == [0xB0, 19]]
            assert 0 in directions and 127 in directions, "Gamepad reversal did not reach MIDI"
            results.append({"view": "synthetic-gamepad", "messages": len(trace), **audio_from_trace("synthetic-gamepad", trace)})
            (REPORTS / "synthetic-gamepad-midi.json").write_text(json.dumps(trace, indent=2))
            browser.close()
    finally:
        server.shutdown()
        server.server_close()
        thread.join()
    report = {"status": "passed", "scope": "Synthetic browser MIDI/Gamepad API -> actual VST3 offline host; not physical hardware or OS MIDI routing", "results": results}
    (REPORTS / "ux-bridge.json").write_text(json.dumps(report, indent=2))
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
