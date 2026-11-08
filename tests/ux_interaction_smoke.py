import functools
import http.server
import json
import os
import threading

from playwright.sync_api import expect, sync_playwright

from ux_bridge_smoke import DEVICE_FIXTURE, QuietHandler, REPORTS, ROOT, arm


def main():
    REPORTS.mkdir(parents=True, exist_ok=True)
    server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), functools.partial(QuietHandler, directory=str(ROOT)))
    thread = threading.Thread(target=server.serve_forever, daemon=True)
    thread.start()
    checks = []
    try:
        with sync_playwright() as playwright:
            options = {"headless": True}
            if os.environ.get("PLAYWRIGHT_CHROMIUM_EXECUTABLE"):
                options["executable_path"] = os.environ["PLAYWRIGHT_CHROMIUM_EXECUTABLE"]
            browser = playwright.chromium.launch(**options)
            page = browser.new_page(viewport={"width": 1440, "height": 1080}, accept_downloads=True)
            page.add_init_script(DEVICE_FIXTURE)
            url = f"http://127.0.0.1:{server.server_port}/docs/ux-performance/index.html"
            page.goto(url + "?view=mood")
            assert page.evaluate("testMessages.length") == 0, "MIDI was not opt-in"
            page.locator("#record").click()
            page.locator("#gesture").focus()
            page.keyboard.down("g")
            page.keyboard.down("Space")
            page.wait_for_timeout(450)
            page.keyboard.up("Space")
            page.keyboard.up("g")
            page.locator("#record").click()
            assert page.locator("#replay").is_enabled()
            with page.expect_download() as download:
                page.locator("#export").click()
            take_path = REPORTS / "mood-control-take.json"
            download.value.save_as(take_path)
            take = json.loads(take_path.read_text())
            assert take["schema"] == "strings-control-study/v1" and take["concept"] == "mood"
            assert any(sample["engaged"] and sample["note"] == 67 for sample in take["samples"])
            assert all(0 <= sample["force"] <= 1 and -1 <= sample["velocity"] <= 1 for sample in take["samples"])
            checks.extend(["MIDI off by default", "actual control recording and JSON export"])
            arm(page)
            page.locator("#replay").click()
            page.wait_for_timeout(900)
            trace = page.evaluate("testMessages")
            assert any(message["data"][:2] == [0x90, 67] for message in trace)
            assert page.locator("#live-force").inner_text() == "0%"
            checks.append("recorded take replays notes through armed MIDI and releases")
            page.goto(url + "?view=phrase")
            page.locator("[data-step-force='3']").fill("61")
            page.locator("[data-step-speed='3']").fill("-55")
            with page.expect_download() as download:
                page.locator("#score-export").click()
            score_path = REPORTS / "edited-control-score.json"
            download.value.save_as(score_path)
            score = json.loads(score_path.read_text())
            assert score["steps"][3]["force"] == 0.61 and score["steps"][3]["velocity"] == -0.55
            checks.append("phrase peak and signed motion edits survive export")
            page.goto(url + "?view=bow")
            surface = page.locator("#gesture").bounding_box()
            page.keyboard.down("g")
            page.mouse.move(surface["x"] + 100, surface["y"] + 200)
            page.mouse.down()
            page.mouse.move(surface["x"] + 350, surface["y"] + 200, steps=10)
            page.wait_for_timeout(900)
            assert page.locator("#live-velocity").inner_text() in ("+0%", "0%")
            page.mouse.up()
            expect(page.locator("#live-force")).to_have_text("0%")
            checks.append("pointer movement decays at rest and releases on up")
            page.keyboard.up("g")
            page.evaluate("""testPad={id:'Unknown synthetic layout',index:0,mapping:'',connected:true,
                axes:[1,0,0,0],buttons:Array.from({length:17},()=>({value:1,pressed:true}))};
                window.dispatchEvent(new Event('gamepadconnected'));""")
            page.locator("#input-source").select_option("gamepad")
            page.wait_for_timeout(200)
            assert "nonstandard" in page.locator("#device-status").inner_text()
            assert page.locator("#live-force").inner_text() == "0%"
            checks.append("unknown gamepad mapping does not excite")
            page.evaluate("""testPad={id:'Calibrated synthetic standard pad',index:0,mapping:'standard',connected:true,
                axes:[.1,-.05,.1,0],buttons:Array.from({length:17},()=>({value:0,pressed:false}))};
                window.dispatchEvent(new Event('gamepadconnected'));""")
            page.locator("#mapping").evaluate("element => element.open = true")
            page.locator("#calibrate").click()
            expect(page.locator("#calibration-status")).to_contain_text("Neutral captured")
            page.wait_for_timeout(100)
            page.evaluate("testPad.axes[0]=.11;testPad.buttons[6].value=.7;testPad.buttons[7].value=.6")
            expect(page.locator("#live-force")).to_have_text("60%")
            assert page.locator("#live-velocity").inner_text() in ("+0%", "0%")
            page.evaluate("testPad.axes[0]=.6")
            page.wait_for_function("parseInt(document.getElementById('live-velocity').textContent, 10) > 0")
            page.evaluate("testPad=null;window.dispatchEvent(new Event('gamepaddisconnected'))")
            expect(page.locator("#live-force")).to_have_text("0%")
            checks.append("neutral calibration, stick deadzone and disconnect release")
            browser.close()
        report = {"status": "passed", "checks": checks, "hardware": "synthetic browser events"}
        (REPORTS / "ux-interactions.json").write_text(json.dumps(report, indent=2))
        print(json.dumps(report, indent=2))
    finally:
        server.shutdown()
        server.server_close()
        thread.join()


if __name__ == "__main__":
    main()
