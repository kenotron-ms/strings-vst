(() => {
  'use strict';
  class MidiOutputBridge {
    constructor(container) {
      this.access = null;
      this.output = null;
      this.armed = false;
      this.lastNote = null;
      this.lastSend = -Infinity;
      this.cache = new Map();
      this.lastError = '';
      const section = document.createElement('section');
      section.className = 'midi-bridge';
      section.setAttribute('aria-label', 'Optional MIDI output');
      section.innerHTML = `<h2>Send to your instrument</h2><p>Optional real MIDI output. The browser stays silent. Route an OS virtual MIDI port to the Strings plugin in your DAW.</p><button id="midi-connect" type="button">Request MIDI outputs</button><label for="midi-output">MIDI output port<select id="midi-output" disabled><option value="">No access requested</option></select></label><label class="midi-arm-label"><input id="midi-arm" type="checkbox" disabled> Arm external MIDI output</label><p id="midi-status" role="status">Off. Nothing is sent.</p><button id="midi-panic" type="button" disabled>Panic / disarm output</button><details><summary>Routing & messages</summary><p>Select a virtual MIDI output here, then select its matching input in your DAW. Enable MIDI input for the native Strings instrument. This page cannot verify DAW routing or that sound is audible.</p><p>Channel 1 only. CC16 force; CC17 speed; CC18 contact; CC19 direction; CC20 bow gate; CC21 body coupling; CC22 sympathy; CC23 rosin; CC1 vibrato. Pitch sends note-on/off independently. No MIDI input or SysEx is requested.</p><p>Focus loss and Stop send release and all-notes-off on this channel. Port changes disarm. A disconnected cable or port can prevent the release reaching your DAW; use the DAW panic control if needed.</p></details>`;
      container.insertBefore(section, container.querySelector('.capture'));
      section.querySelector('details').insertAdjacentHTML('beforeend', '<p>Research build 0.2 also receives CC24 instrument (0–3), CC25 ensemble, CC26 players (2–16), CC27 variation, and CC28 room. UI contact increases toward the bridge; CC18 is inverted to match the native distance-from-bridge parameter. Version 0.1 does not implement this expression protocol.</p>');
      this.select = section.querySelector('#midi-output');
      this.arm = section.querySelector('#midi-arm');
      this.status = section.querySelector('#midi-status');
      this.connect = section.querySelector('#midi-connect');
      this.panicButton = section.querySelector('#midi-panic');
      this.connect.addEventListener('click', () => this.request());
      this.select.addEventListener('change', () => this.selectPort());
      this.arm.addEventListener('change', () => this.setArmed(this.arm.checked));
      this.panicButton.addEventListener('click', () => this.disarm('Output disarmed. Release messages sent where available.'));
      if (!navigator.requestMIDIAccess) {
        this.connect.disabled = true;
        this.setStatus('Web MIDI unavailable here. Use a supporting browser on HTTPS or localhost. Local controls still work.');
      }
    }

    setStatus(message) {
      if (this.status.textContent !== message) this.status.textContent = message;
    }

    async request() {
      this.connect.disabled = true;
      this.setStatus('Requesting MIDI output access. No SysEx.');
      try {
        if (this.access) this.access.onstatechange = null;
        this.access = await navigator.requestMIDIAccess({ sysex: false });
        this.access.onstatechange = () => this.refreshPorts();
        this.refreshPorts();
        this.connect.textContent = 'Refresh MIDI outputs';
      } catch (error) {
        this.setStatus(`MIDI access unavailable (${error.name || 'request failed'}). Nothing is sent.`);
      } finally {
        this.connect.disabled = false;
      }
    }

    refreshPorts() {
      const ports = [...this.access.outputs.values()].filter(port => port.state === 'connected');
      const previousId = this.output?.id || this.select.value;
      let disconnected = false;
      if (this.output && !ports.some(port => port.id === this.output.id)) {
        disconnected = true;
        this.disarm('MIDI output disconnected. Release delivery cannot be confirmed. Check the DAW if a note remains held.');
        this.output = null;
      }
      this.select.replaceChildren(new Option('Choose an output port', ''));
      ports.forEach(port => this.select.add(new Option(port.name || 'Unnamed MIDI output', port.id)));
      this.select.value = ports.some(port => port.id === previousId) ? previousId : '';
      this.select.disabled = ports.length === 0;
      this.arm.disabled = !this.select.value;
      if (!this.armed && !this.output && !disconnected) {
        this.setStatus(ports.length ? 'Outputs available. Select a port, then explicitly arm output.' : 'No connected MIDI outputs. Create an OS virtual MIDI route, then refresh.');
      }
    }

    selectPort() {
      this.disarm('Port changed. Output remains off until armed.');
      this.output = this.access?.outputs.get(this.select.value) || null;
      this.arm.disabled = !this.output;
      this.cache.clear();
    }

    async setArmed(enabled) {
      if (!enabled) {
        this.disarm('Output disarmed. Release messages sent where available.');
        return;
      }
      const port = this.output;
      if (!port || port.state !== 'connected') {
        this.disarm('Choose a connected MIDI output first.');
        return;
      }
      this.arm.disabled = true;
      try {
        await port.open();
        if (this.output !== port || !this.arm.checked || port.state !== 'connected') return;
        this.cache.clear();
        this.lastNote = null;
        this.lastSend = -Infinity;
        this.armed = true;
        this.panicButton.disabled = false;
        this.setStatus(`Armed · ${port.name || 'MIDI output'} · channel 1. DAW route and audio are unverified.`);
        this.updateNotice();
      } catch (error) {
        this.disarm(`Could not open output (${error.name || 'port error'}). Nothing is sent.`);
      } finally {
        this.arm.disabled = !this.output || this.output.state !== 'connected';
      }
    }

    updateNotice() {
      const notice = document.querySelector('.notice');
      notice.replaceChildren(document.createTextNode(this.armed ? 'External MIDI output armed' : 'Control prototype · browser silent'));
      const detail = document.createElement('span');
      detail.textContent = this.armed ? 'Channel 1 · DAW route / audio unverified' : 'Optional MIDI output is off · no native audio connection';
      notice.append(detail);
    }

    panic() {
      if (!this.armed || !this.output) {
        if (this.arm.checked) {
          this.arm.checked = false;
          this.setStatus('Pending arming canceled by release. Arm again when ready.');
        }
        return;
      }
      try {
        if (this.lastNote !== null) this.output.send([0x80, this.lastNote, 0]);
        for (const [controller, value] of [[16, 0], [17, 0], [20, 0], [123, 0], [120, 0]]) {
          this.output.send([0xb0, controller, value]);
        }
        this.cache.clear();
        this.cache.set(16, 0);
        this.cache.set(17, 0);
        this.cache.set(20, 0);
      } catch (error) {
        this.lastError = `Release delivery failed (${error.name || 'port error'}). Check the DAW panic control.`;
        this.armed = false;
        this.arm.checked = false;
        this.panicButton.disabled = true;
        this.setStatus(this.lastError);
        this.updateNotice();
      }
      this.lastNote = null;
    }

    disarm(message) {
      this.lastError = '';
      this.panic();
      this.armed = false;
      this.arm.checked = false;
      this.lastNote = null;
      this.cache.clear();
      this.panicButton.disabled = true;
      this.setStatus(this.lastError || message);
      this.updateNotice();
    }

    update(state, midiNote, now) {
      if (!this.armed || !this.output || now - this.lastSend < 1000 / 60) return;
      this.lastSend = now;
      const normalized = value => Math.round(Math.max(0, Math.min(1, Number.isFinite(value) ? value : 0)) * 127);
      const direction = Math.sign(state.velocity);
      const instruments = ['Violin', 'Viola', 'Cello', 'Double bass'];
      const controls = [[24, Math.max(0, instruments.indexOf(state.instrument))], [25, state.ensemble ? 127 : 0], [26, Math.max(2, Math.min(16, Math.round(state.players)))], [27, normalized(state.variation)], [28, normalized(state.room)], [16, normalized(state.force)], [17, normalized(Math.abs(state.velocity))], [18, normalized(1 - state.contact)], [19, normalized((direction + 1) / 2)], [20, state.engaged ? 127 : 0], [21, normalized(state.bodyCoupling)], [22, normalized(state.sympathetic)], [23, normalized(state.rosin)], [1, normalized(state.vibrato)]];
      try {
        if (midiNote !== this.lastNote) {
          if (this.lastNote !== null) this.output.send([0x80, this.lastNote, 0]);
          this.lastNote = null;
        }
        for (const [controller, value] of controls) {
          if (this.cache.get(controller) !== value) {
            this.output.send([0xb0, controller, value]);
            this.cache.set(controller, value);
          }
        }
        if (midiNote !== null && this.lastNote !== midiNote) {
          this.output.send([0x90, midiNote, 100]);
          this.lastNote = midiNote;
        }
      } catch (error) {
        this.disarm(`MIDI send failed (${error.name || 'port error'}). Output disarmed; check the DAW for held notes.`);
      }
    }
  }
  window.StringsMidiOutputBridge = MidiOutputBridge;
})();
