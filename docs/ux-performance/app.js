(() => {
  'use strict';
  const views = {
    bow: { title: 'Bow desk', subtitle: 'Find the gesture. Keep the feeling.', description: 'One hand gives a phrase its breath. Hold a note below, then drag across the strings.', tag: 'Direct performance' },
    mood: { title: 'Mood field', subtitle: 'Start with a feeling.', description: 'Move between four authored characters. Hear nothing here; inspect the physical recipe your gesture would send.', tag: 'Intent → expression' },
    phrase: { title: 'Phrase studio', subtitle: 'Make the feeling repeatable.', description: 'Sketch eight moments of force and motion. Play, refine, and keep a contour you can return to.', tag: 'Compose over time' },
    conductor: { title: 'Conductor', subtitle: 'Shape a section as one.', description: 'Choose the players, then guide their energy, unity, and distance. Pitch remains an independent part.', tag: 'Ensemble performance' },
    material: { title: 'Material lab', subtitle: 'Find the grain of a sound.', description: 'Work from the string outward. Change contact, force, and motion with the physical recipe in view.', tag: 'Physical sound design' }
  };
  const route = new URLSearchParams(location.search).get('view');
  const view = views[route] ? route : null;
  const main = document.querySelector('#main');
  const byId = id => document.getElementById(id);
  const clamp = (value, low = 0, high = 1) => Math.max(low, Math.min(high, value));
  const percent = value => `${Math.round(value * 100)}%`;
  const range = (id, label, value, min = 0, max = 100, hint = '') => `<div class="control-row"><label for="${id}">${label}<output id="${id}-value">${value}</output></label><input id="${id}" type="range" min="${min}" max="${max}" value="${value}">${hint ? `<p>${hint}</p>` : ''}</div>`;
  const noteNames = ['C4', 'C♯4', 'D4', 'D♯4', 'E4', 'F4', 'F♯4', 'G4', 'G♯4', 'A4', 'A♯4', 'B4', 'C5'];
  const noteKeys = ['a', 'w', 's', 'e', 'd', 'f', 't', 'g', 'y', 'h', 'u', 'j', 'k'];
  const state = { force: 0, velocity: 0, contact: .35, vibrato: .22, bodyCoupling: .5, sympathetic: .3, rosin: .5, variation: .25, room: .13, engaged: false, source: 'pointer', instrument: 'Violin', ensemble: view === 'conductor', players: 8 };
  let owner = null;
  let pointerId = null;
  let lastPoint = null;
  let lastMotion = 0;
  let keyBow = false;
  let neutral = [0, 0, 0, 0];
  let padIndex = null;
  let padArmed = false;
  let padSignature = '';
  let recordStart = null;
  let recording = [];
  let take = [];
  let replayStart = null;
  let replayCursor = 0;
  let phraseStart = null;
  let phraseStep = -1;
  let previousFrame = 0;
  let takeMetadata = null;
  const heldNotes = new Map();
  const steps = Array.from({ length: 8 }, (_, index) => ({ force: [18, 35, 65, 83, 70, 48, 31, 12][index], velocity: [25, 40, 62, 70, -65, -50, -35, -20][index], note: [0, 2, 4, 7, 9, 7, 4, 2][index] }));

  if (!view) {
    main.innerHTML = `<section class="intro"><div><h1>Five ways<br>into a phrase.</h1><p>A string instrument for the feeling you have in mind.</p></div><p>Start with a gesture, a mood, a contour, a section, or a material. Five working interaction studies for composers and sound designers. All controls are local; none produce sound.</p></section><div class="gallery">${Object.entries(views).map(([id, concept]) => `<a class="concept" href="?view=${id}" data-concept="${id}"><div class="preview ${id}" aria-hidden="true">${id === 'phrase' ? [20,40,65,90,78,55,35,18].map(height => `<i style="height:${height}px"></i>`).join('') : id === 'conductor' ? '◦ ◦ ◦ ◦ ◦' : ''}</div><h2>${concept.title}</h2><p>${concept.subtitle} ${id === 'bow' ? 'Mouse, touchpad, or controller.' : id === 'mood' ? 'A transparent character-morphing field.' : id === 'phrase' ? 'Edit, loop, and export a control score.' : id === 'conductor' ? 'Guide the collective expression.' : 'Explore the mechanics behind the tone.'}</p></a>`).join('')}</div><p class="gallery-note">No violin technique required. No pressure-sensing mouse claims. No invented device data. <a href="design.md">Read the rationale and task comparison</a>.</p>`;
    byId('stop').disabled = true;
    return;
  }

  document.title = `Strings — ${views[view].title}`;
  document.body.dataset.view = view;
  const bowContent = `<div id="gesture" class="surface bow-surface" tabindex="0" role="group" aria-label="Bow surface. Hold Space and use arrows, or hold pointer and drag." aria-describedby="gesture-help"><span class="axis-top">← Up-bow / Down-bow →</span><div class="bow-lines" aria-hidden="true"><i></i><i></i><i></i><i></i></div><div id="bow-ribbon" class="bow-ribbon" aria-hidden="true"></div><div id="surface-dot" class="surface-dot rest" aria-hidden="true"></div><span class="surface-note">Downward = more force · Horizontal motion = signed velocity</span></div><p id="gesture-help" class="surface-help">Hold and drag to bow. Force comes from Y position, not measured pressure. Keyboard: focus the surface, hold Space; arrows change motion and force.</p><div class="range-pair">${range('bow-contact','Contact point',35,0,100,'0 = fingerboard · 100 = bridge')}${range('bow-vibrato','Vibrato depth',22)}</div>`;
  const moodContent = `<div id="gesture" class="surface mood-surface" tabindex="0" role="group" aria-label="Mood field. Use the two sliders below for keyboard input; hold Space here to engage." aria-describedby="gesture-help"><div class="mood-grid" aria-hidden="true"></div><span class="mood-axis tl">Suspended</span><span class="mood-axis tr">Urgent</span><span class="mood-axis bl">Intimate</span><span class="mood-axis br">Resolute</span><div id="surface-dot" class="surface-dot rest" aria-hidden="true"></div></div><p id="gesture-help" class="surface-help">Hold and move to perform. Release to lift the bow. These are authored macro names, not measured emotions. Focus field + Space also engages.</p><div class="range-pair">${range('mood-weight','Light → weighty',50)}${range('mood-unrest','Settled → restless',50)}</div><div class="macro-recipe"><span>Force = 15% + 70% weight</span><span>Speed = 15% + 70% unrest</span><span>Contact = 20% + 60% weight</span><span>Vibrato = 5% + 65% unrest</span></div><div class="preset-line"><button data-mood="15,15">Intimate</button><button data-mood="20,85">Suspended</button><button data-mood="90,90">Urgent</button><button data-mood="85,20">Resolute</button></div>`;
  const phraseContent = `<div class="transport"><button id="phrase-play" class="primary">Play control score</button><button id="phrase-stop">Stop score</button><label for="tempo">Tempo <input id="tempo" type="number" min="30" max="240" value="80"></label><label><input id="loop" type="checkbox" checked> Loop</label><button id="score-export">Export score</button></div><div class="score"><div class="score-key"><span>Eight eighth-notes / one bar of 4/4</span><span>Force ↑ · Signed motion ↔</span></div><div class="score-grid">${steps.map((step,index) => `<div class="step" data-step="${index}"><span class="step-number">${index+1}</span><div class="bar" aria-hidden="true"><div class="fill" style="height:${step.force}%"></div></div><label for="step-force-${index}">Force <output id="step-force-${index}-value">${step.force}%</output></label><input id="step-force-${index}" data-step-force="${index}" type="range" min="0" max="100" value="${step.force}"><label for="step-speed-${index}">Motion <output id="step-speed-${index}-value">${step.velocity}</output></label><input id="step-speed-${index}" data-step-speed="${index}" type="range" min="-100" max="100" value="${step.velocity}"><label for="step-note-${index}">Pitch</label><select id="step-note-${index}" data-step-note="${index}">${noteNames.map((name,note) => `<option value="${note}" ${note === step.note ? 'selected' : ''}>${name}</option>`).join('')}</select></div>`).join('')}</div></div><div class="phrase-info"><p>Editable control score. Stepped force and velocity; no audio playback.</p><p id="score-position">Stopped</p></div><div class="preset-line"><button id="phrase-arch">Restore arch</button><button id="phrase-reverse">Reverse contour</button><label><input id="score-pitch" type="checkbox" checked> Include score pitch</label></div>`;
  const conductorContent = `<div class="conductor-layout"><div id="stage" class="stage" aria-label="Requested section size"></div><div class="fader-stack">${range('energy','Energy',55,0,100,'Force 10–90% · speed 15–80%')}${range('unity','Unity',75,0,100,'Player variation = 100% − unity')}${range('distance','Distance',20,0,100,'Room send 0–65%; conceptual macro')}</div></div><div class="conductor-action"><button id="engage" class="primary hold-button">Hold to conduct</button><span class="hint">Hold pointer or Space/Enter.<br>Choose pitch below; release to rest.</span></div><div class="preset-line"><button data-section="4,30,90,15">Chamber / close</button><button data-section="12,70,65,45">Section / expansive</button><button data-section="16,90,45,70">Mass / restless</button></div><p class="surface-help">Seats show the requested players, not active voices. Unity and distance are proposed section controls; no player or acoustic simulation runs here.</p>`;
  const materialContent = `<div class="material-layout"><div class="string-section" aria-hidden="true"><small>Fingerboard</small><div class="wire"></div><div id="contact-marker" class="contact-marker"></div><div class="bridge"></div><small>Bridge / body</small></div><div class="material-controls">${range('material-force','Bow force',45,0,100,'Assigned force; not device pressure')}${range('material-speed','Signed bow velocity',40,-100,100,'Negative = up-bow · positive = down-bow')}${range('material-contact','Contact point',35,0,100,'0 = fingerboard · 100 = bridge')}${range('material-vibrato','Vibrato depth',15)}</div></div><div class="conductor-action"><button id="engage" class="primary hold-button">Hold to excite string</button><span class="hint">A recipe becomes a control signal while held.<br>No sound or body simulation in this browser.</span></div><div class="preset-line"><button data-material="20,30,15,8">Feather</button><button data-material="65,35,82,10">Grain</button><button data-material="45,70,45,40">Bloom</button></div>`;
  const content = { bow: bowContent, mood: moodContent, phrase: phraseContent, conductor: conductorContent, material: materialContent };
  main.innerHTML = `<nav class="view-nav" aria-label="Concepts"><a href="index.html">All studies</a>${Object.entries(views).map(([id,item]) => `<a href="?view=${id}" ${view === id ? 'aria-current="page"' : ''}>${item.title}</a>`).join('')}</nav><section class="setup" aria-label="Instrument configuration"><fieldset><legend>Instrument</legend><div class="segmented">${['Violin','Viola','Cello','Double bass'].map((name,index) => `<button data-instrument="${name}" aria-pressed="${index===0}">${name}</button>`).join('')}</div></fieldset><div class="section-control"><fieldset><legend>Players</legend><div class="segmented"><button id="solo" aria-pressed="${!state.ensemble}">Solo</button><button id="ensemble" aria-pressed="${state.ensemble}">Ensemble</button></div></fieldset><label for="players">Section <output id="players-value">8</output><input id="players" type="range" min="2" max="16" value="8" ${state.ensemble?'':'disabled'}></label></div></section><div class="workspace"><section class="performance" aria-label="${views[view].title}"><div class="view-heading"><div><h1>${views[view].title}</h1><p class="page-description">${views[view].description}</p></div><span class="edition">${views[view].tag}</span></div>${content[view]}<div class="telemetry" aria-label="Live control values">${[['force','Force'],['velocity','Signed velocity'],['contact','Contact'],['vibrato','Vibrato']].map(([id,label])=>`<div><label for="live-${id}">${label}</label><output id="live-${id}">0%</output><meter id="meter-${id}" min="${id==='velocity'?-1:0}" max="1" value="0" aria-label="${label}"></meter></div>`).join('')}</div><div id="live-state" class="live-state">Bow at rest</div><section class="pitch" aria-label="Independent pitch source"><div class="pitch-header"><div><h2>Pitch is its own part.</h2><span class="pitch-source">Computer / on-screen keyboard</span></div><output id="pitch-value">No note held</output></div><div class="piano">${noteNames.map((name,index)=>`<button class="key ${name.includes('♯')?'black':''}" data-note="${index}" aria-label="${name}, computer key ${noteKeys[index].toUpperCase()}"><span>${name}</span><kbd>${noteKeys[index].toUpperCase()}</kbd></button>`).join('')}</div><p>Hold A W S E D F T G Y H U J K for C4–C5, or hold a key here. Last-held note takes priority. External MIDI keyboard is an independent pitch source in the proposed plugin; Web MIDI is not connected here. Gamepads do not sense finger placement.</p></section></section><aside class="desk"><h2>Your input desk</h2><label for="input-source">Expression source<select id="input-source"><option value="pointer">Mouse / touchpad / keyboard</option><option value="gamepad">Gamepad · standard mapping</option></select></label><label for="device">Controller<select id="device"><option value="">No controller detected</option></select></label><div id="device-status" class="status" role="status">Pointer ready. Hold a gesture to begin.</div><p id="device-caption" class="device-caption">Controller discovery requires a focused page and a controller button press.</p><details open><summary>Learn this concept</summary><p>${view==='bow'?'First hold a pitch. Drag horizontally to move the bow, downward to add force. Lift to release. A normal touchpad works like a mouse.':view==='mood'?'Pick a character or position in the field. Hold to send its recipe. Sliders let you set the same point by keyboard. Record a journey to make it repeatable.':view==='phrase'?'Set force, signed motion, and pitch for each step, then play the score. Disable score pitch to supply your own notes. Gamepad or manual expression stops the score before taking control.':view==='conductor'?'Choose Solo or Ensemble and section size. Set energy, unity, and distance, then hold to conduct while supplying notes independently.':'Choose a material recipe, then adjust the four physical inputs. Hold to excite the string. Contact is a location along the string, not a finger position.'}</p></details><details id="mapping"><summary>Controller mapping & calibration</summary><p>Standard PS5 DualSense / Xbox layout. LT engages; RT sets force. Device-specific names depend on browser support.</p><div class="mapping-grid"><label for="force-map">Force<select id="force-map"><option value="7">RT / R2</option><option value="6">LT / L2</option></select></label><label for="engage-map">Bow engagement<select id="engage-map"><option value="6">LT / L2</option><option value="7">RT / R2</option></select></label>${[['velocity-map','Signed velocity',0],['contact-map','Contact',2],['vibrato-map','Vibrato',3]].map(([id,label,selected])=>`<label for="${id}">${label}<select id="${id}">${['Left X','Left Y','Right X','Right Y'].map((name,index)=>`<option value="${index}" ${index===selected?'selected':''}>${name}</option>`).join('')}</select></label>`).join('')}</div><div class="calibration">${range('deadzone','Neutral deadzone',12,0,35)}<button id="calibrate">Capture neutral sticks</button><span id="calibration-status">Rest both sticks before capture. Session only.</span></div></details><details><summary>Browser boundaries</summary><p>Mouse and ordinary touchpad force is assigned, not sensed. Pen/touch pressure is not used. DualSense touchpad and adaptive triggers need advanced native integration; they are not supported by this standard browser mapping.</p><p>No audio, MIDI input, native plugin connection, measured emotion, or simulated controller telemetry.</p></details><div class="capture"><h3>Keep a gesture</h3><div class="transport"><button id="record">Record controls</button><button id="replay" disabled>Replay</button><button id="export" disabled>Export</button></div><output id="take-status">No take yet · up to 20 seconds</output><p>Records actual controls and pitch locally. Replay sends visual control values only. Export is JSON for a future engine adapter.</p></div></aside></div>`;

  document.querySelector('.capture p').textContent = 'Records actual controls and pitch locally. Replay also sends MIDI when output is explicitly armed. Export preserves the control take as JSON.';
  document.querySelector('.pitch > p').textContent = 'Hold A W S E D F T G Y H U J K for C4–C5, or hold a key here. Last-held note takes priority. Optional Web MIDI output sends these notes to the native plugin. External MIDI input is not implemented in this browser study.';
  document.querySelectorAll('#mapping + details p')[1].textContent = 'No embedded audio engine, MIDI input, measured emotion, or device-specific controller features. Optional Web MIDI output below can drive the native plugin through a configured DAW route.';
  if(view==='material'){
    document.querySelector('.material-layout').insertAdjacentHTML('afterend',`<section class="resonance-bench" aria-label="Body and string character"><h2>Let the instrument answer.</h2><p>Shape the proposed coupled string and body response. These are editable recipes, not an acoustic simulation.</p><div class="body-controls">${range('body-coupling','Body coupling',50,0,100,'Energy exchange with bridge / body')}${range('sympathetic','Sympathetic resonance',30,0,100,'Unplayed strings joining the tone')}${range('rosin','Rosin character',50,0,100,'Proposed friction character control')}</div></section>`);
  }
  if(view==='conductor')document.querySelector('.conductor-action').insertAdjacentHTML('afterend','<p id="section-recipe" class="surface-help"></p>');
  byId('mapping').insertAdjacentHTML('beforeend','<p>Gamepad controls physical expression directly in every study. It does not move the mood macros or recognize notes. Controller contact is centered at 50%; right-stick Y magnitude sets vibrato.</p>');

  const midi = new window.StringsMidiOutputBridge(document.querySelector('.desk'));
  function setStatus(message) { if(byId('device-status').textContent!==message)byId('device-status').textContent = message; }
  function pitch() { return [...heldNotes.values()].at(-1) ?? null; }
  function noteOn(token, note) { heldNotes.delete(token); heldNotes.set(token, note); }
  function noteOff(token) { heldNotes.delete(token); }
  function releaseBow() { state.force = 0; state.velocity = 0; state.engaged = false; owner = null; keyBow = false; }
  function stopPhrase() { phraseStart = null; phraseStep = -1; heldNotes.delete('score'); document.querySelectorAll('.step').forEach(element=>element.classList.remove('playing')); if(byId('score-position')) byId('score-position').textContent='Stopped'; }
  function finishRecording() {
    if (recordStart === null) return;
    const duration = Math.min(20000, performance.now() - recordStart);
    recording.push({...controlSample(),time: Math.round(duration), force:0, velocity:0, engaged:false, note:null});
    take = recording.slice(); recordStart = null;
    byId('record').textContent='Record controls'; byId('record').setAttribute('aria-pressed','false');
    byId('replay').disabled = take.length < 2; byId('export').disabled = take.length < 2;
    byId('take-status').textContent=`Take saved · ${(duration/1000).toFixed(1)} seconds · local only`;
  }
  function safetyRelease(message) {
    releaseBow(); pointerId=null; lastPoint=null; replayStart=null; stopPhrase(); heldNotes.clear(); padArmed=false;
    midi.panic();
    finishRecording(); if(message) setStatus(message);
  }
  function manualTakeover(newOwner) {
    replayStart=null; heldNotes.delete('replay'); stopPhrase(); owner=newOwner;
  }
  function sliderValue(id) { return Number(byId(id).value)/100; }
  function applyRecipe(engaged = state.engaged) {
    if(view==='bow'){state.contact=sliderValue('bow-contact');state.vibrato=sliderValue('bow-vibrato');}
    if(view==='mood'){
      const weight=sliderValue('mood-weight'), unrest=sliderValue('mood-unrest');
      state.contact=.2+.6*weight;state.vibrato=.05+.65*unrest;
      if(engaged){state.force=.15+.7*weight;state.velocity=.15+.7*unrest;}
      byId('surface-dot').style.left=`${weight*100}%`;byId('surface-dot').style.top=`${(1-unrest)*100}%`;
    }
    if(view==='conductor'){
      state.variation=1-sliderValue('unity');state.room=.65*sliderValue('distance');
      if(engaged){state.force=.1+.8*sliderValue('energy');state.velocity=.15+.65*sliderValue('energy');}
      byId('section-recipe').textContent=`Player variation ${percent(state.variation)} · room send ${percent(state.room)}. Requested settings, not measured ensemble behavior.`;
    }
    if(view==='material'){
      state.contact=sliderValue('material-contact');state.vibrato=sliderValue('material-vibrato');
      state.bodyCoupling=sliderValue('body-coupling');state.sympathetic=sliderValue('sympathetic');state.rosin=sliderValue('rosin');
      if(engaged){state.force=sliderValue('material-force');state.velocity=sliderValue('material-speed');}
    }
  }
  function updateRanges(){document.querySelectorAll('input[type=range]').forEach(input=>{const output=byId(`${input.id}-value`);if(output) output.textContent=input.dataset.stepForce!==undefined?`${input.value}%`:input.value;});}
  function setRange(id,value){byId(id).value=value;updateRanges();}
  function renderStage(){if(!byId('stage'))return;const count=state.ensemble?state.players:1;byId('stage').innerHTML=`${Array.from({length:count},()=>'<span class="player" aria-hidden="true"></span>').join('')}<span class="stage-caption">${count} ${count===1?'player':'players'} requested / ${state.instrument}</span>`;}
  function setEnsemble(value){state.ensemble=value;byId('solo').setAttribute('aria-pressed',String(!value));byId('ensemble').setAttribute('aria-pressed',String(value));byId('players').disabled=!value;renderStage();}
  document.querySelectorAll('[data-instrument]').forEach(button=>button.addEventListener('click',()=>{safetyRelease('Instrument changed. Bow released.');state.instrument=button.dataset.instrument;document.querySelectorAll('[data-instrument]').forEach(item=>item.setAttribute('aria-pressed',String(item===button)));renderStage();}));
  byId('solo').onclick=()=>{safetyRelease();setEnsemble(false);};byId('ensemble').onclick=()=>{safetyRelease();setEnsemble(true);};
  byId('players').addEventListener('input',event=>{safetyRelease();state.players=Number(event.target.value);renderStage();});
  document.querySelectorAll('input[type=range]').forEach(input=>input.addEventListener('input',()=>{updateRanges();if(owner!=='gamepad'&&owner!=='replay')applyRecipe();}));
  byId('stop').onclick=()=>safetyRelease('All controls released. Return triggers to rest before playing again.');
  window.addEventListener('blur',()=>safetyRelease('Focus lost. All controls released.'));
  document.addEventListener('visibilitychange',()=>{if(document.hidden)safetyRelease('Page hidden. All controls released.');});
  window.addEventListener('pagehide',()=>safetyRelease());
  byId('input-source').onchange=event=>{safetyRelease();state.source=event.target.value;setStatus(state.source==='pointer'?'Pointer and keyboard ready.':'Waiting for a standard controller. Press a controller button.');};

  const surface=byId('gesture');
  function moveSurface(event,initial=false){
    const rect=surface.getBoundingClientRect();const horizontal=clamp((event.clientX-rect.left)/rect.width);const vertical=clamp((event.clientY-rect.top)/rect.height);
    if(view==='bow'){
      state.force=vertical;
      const now=performance.now();
      state.velocity=initial||!lastPoint?0:clamp(((horizontal-lastPoint.horizontal)/Math.max(.008,(now-lastPoint.time)/1000))*1.7,-1,1);
      lastPoint={horizontal,time:now};lastMotion=now;
      byId('surface-dot').style.left=`${horizontal*100}%`;byId('surface-dot').style.top=`${vertical*100}%`;byId('bow-ribbon').style.left=`${10+horizontal*80}%`;
    } else {setRange('mood-weight',Math.round(horizontal*100));setRange('mood-unrest',Math.round((1-vertical)*100));applyRecipe(true);}
  }
  if(surface){
    surface.addEventListener('pointerdown',event=>{
      if(event.button!==0||state.source!=='pointer'||pointerId!==null)return;
      event.preventDefault();surface.focus();manualTakeover('pointer');pointerId=event.pointerId;surface.setPointerCapture(pointerId);state.engaged=true;moveSurface(event,true);
    });
    surface.addEventListener('pointermove',event=>{if(event.pointerId===pointerId)moveSurface(event);});
    const end=event=>{if(event.pointerId===pointerId){pointerId=null;lastPoint=null;releaseBow();}};
    surface.addEventListener('pointerup',end);surface.addEventListener('pointercancel',end);surface.addEventListener('lostpointercapture',end);
    surface.addEventListener('keydown',event=>{
      if(state.source!=='pointer')return;
      if(event.code==='Space'){event.preventDefault();if(!keyBow){manualTakeover('keyboard');keyBow=true;state.engaged=true;if(view==='bow'){state.force=.5;state.velocity=.3;}else applyRecipe(true);}}
      if(event.key.startsWith('Arrow')){event.preventDefault();if(view==='bow'&&keyBow){if(event.key==='ArrowLeft')state.velocity=clamp(state.velocity-.1,-1,1);if(event.key==='ArrowRight')state.velocity=clamp(state.velocity+.1,-1,1);if(event.key==='ArrowUp')state.force=clamp(state.force-.05);if(event.key==='ArrowDown')state.force=clamp(state.force+.05);}else if(view==='mood'){const id=['ArrowLeft','ArrowRight'].includes(event.key)?'mood-weight':'mood-unrest';const delta=['ArrowRight','ArrowUp'].includes(event.key)?5:-5;setRange(id,clamp(Number(byId(id).value)+delta,0,100));applyRecipe();}}
    });
    surface.addEventListener('keyup',event=>{if(event.code==='Space'){event.preventDefault();releaseBow();}});
    surface.addEventListener('blur',()=>{if(keyBow||pointerId!==null){pointerId=null;releaseBow();}});
  }
  const engage=byId('engage');
  if(engage){
    const start=()=>{if(state.source!=='pointer')return;manualTakeover('hold');state.engaged=true;applyRecipe(true);};
    engage.addEventListener('pointerdown',event=>{if(event.button!==0)return;engage.setPointerCapture(event.pointerId);start();});
    ['pointerup','pointercancel','lostpointercapture','blur'].forEach(type=>engage.addEventListener(type,()=>{if(owner==='hold')releaseBow();}));
    engage.addEventListener('keydown',event=>{if(['Space','Enter'].includes(event.code)){event.preventDefault();if(!event.repeat)start();}});
    engage.addEventListener('keyup',event=>{if(['Space','Enter'].includes(event.code)){event.preventDefault();if(owner==='hold')releaseBow();}});
  }
  document.querySelectorAll('.key').forEach(button=>{
    button.addEventListener('pointerdown',event=>{if(event.button!==0)return;event.preventDefault();button.focus();button.setPointerCapture(event.pointerId);noteOn(`pointer-${event.pointerId}`,Number(button.dataset.note));});
    ['pointerup','pointercancel','lostpointercapture'].forEach(type=>button.addEventListener(type,event=>noteOff(`pointer-${event.pointerId}`)));
    button.addEventListener('keydown',event=>{if(['Space','Enter'].includes(event.code)){event.preventDefault();noteOn(`button-${button.dataset.note}`,Number(button.dataset.note));}});
    button.addEventListener('keyup',event=>{if(['Space','Enter'].includes(event.code)){event.preventDefault();noteOff(`button-${button.dataset.note}`);}});
    button.addEventListener('blur',()=>noteOff(`button-${button.dataset.note}`));
  });
  window.addEventListener('keydown',event=>{
    if(event.key==='Escape'){safetyRelease('All controls released.');return;}
    if(event.ctrlKey||event.altKey||event.metaKey||event.repeat||/^(INPUT|SELECT|TEXTAREA)$/.test(event.target.tagName))return;
    const index=noteKeys.indexOf(event.key.toLowerCase());if(index!==-1){event.preventDefault();noteOn(`key-${event.code}`,index);}
  });
  window.addEventListener('keyup',event=>noteOff(`key-${event.code}`));
  document.querySelectorAll('[data-mood]').forEach(button=>button.onclick=()=>{safetyRelease();const values=button.dataset.mood.split(',');setRange('mood-weight',values[0]);setRange('mood-unrest',values[1]);applyRecipe(false);});
  document.querySelectorAll('[data-material]').forEach(button=>button.onclick=()=>{safetyRelease();button.dataset.material.split(',').forEach((value,index)=>setRange(['material-force','material-speed','material-contact','material-vibrato'][index],value));applyRecipe(false);});
  document.querySelectorAll('[data-section]').forEach(button=>button.onclick=()=>{safetyRelease();const values=button.dataset.section.split(',').map(Number);state.players=values[0];setRange('players',values[0]);setEnsemble(true);['energy','unity','distance'].forEach((id,index)=>setRange(id,values[index+1]));applyRecipe(false);});

  function download(data,name){const url=URL.createObjectURL(new Blob([JSON.stringify(data,null,2)],{type:'application/json'}));const link=document.createElement('a');link.href=url;link.download=name;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);}
  function metadata(){return {schema:'strings-control-study/v1',controlOnly:true,concept:view,instrument:state.instrument,ensemble:state.ensemble,players:state.ensemble?state.players:1,units:{time:'milliseconds',force:'normalized 0..1',velocity:'signed normalized -1..1; native BowSpeed = abs(velocity), BowDirection = sign(velocity)',contact:'0 fingerboard .. 1 bridge',vibrato:'normalized 0..1',bodyCoupling:'normalized 0..1',sympathetic:'normalized 0..1',rosin:'normalized 0..1',variation:'normalized 0..1',room:'normalized 0..1',engaged:'boolean gate',note:'MIDI note number or null'}};}
  function controlSample(){return {force:state.force,velocity:state.velocity,contact:state.contact,vibrato:state.vibrato,bodyCoupling:state.bodyCoupling,sympathetic:state.sympathetic,rosin:state.rosin,variation:state.variation,room:state.room,engaged:state.engaged,note:pitch()};}
  byId('record').onclick=()=>{
    if(recordStart!==null){finishRecording();return;}
    safetyRelease();recording=[];takeMetadata=metadata();recordStart=performance.now();byId('record').textContent='Finish take';byId('record').setAttribute('aria-pressed','true');byId('replay').disabled=true;byId('export').disabled=true;
  };
  byId('replay').onclick=()=>{if(!take.length)return;safetyRelease();replayStart=performance.now();replayCursor=0;owner='replay';setStatus('Replaying captured controls. No audio.');};
  byId('export').onclick=()=>download({...takeMetadata,samples:take.map(sample=>({...sample,note:sample.note===null?null:60+sample.note}))},`strings-${view}-take.json`);

  if(view==='phrase'){
    function refreshScore(){steps.forEach((step,index)=>{setRange(`step-force-${index}`,step.force);setRange(`step-speed-${index}`,step.velocity);byId(`step-note-${index}`).value=step.note;document.querySelector(`[data-step="${index}"] .fill`).style.height=`${step.force}%`;});}
    document.querySelectorAll('[data-step-force]').forEach(input=>input.addEventListener('input',()=>{steps[Number(input.dataset.stepForce)].force=Number(input.value);refreshScore();}));
    document.querySelectorAll('[data-step-speed]').forEach(input=>input.addEventListener('input',()=>{steps[Number(input.dataset.stepSpeed)].velocity=Number(input.value);}));
    document.querySelectorAll('[data-step-note]').forEach(input=>input.addEventListener('change',()=>{steps[Number(input.dataset.stepNote)].note=Number(input.value);}));
    byId('phrase-play').onclick=()=>{releaseBow();stopPhrase();replayStart=null;heldNotes.delete('replay');phraseStart=performance.now();owner='score';padArmed=false;setStatus('Playing control score. No audio.');};
    byId('phrase-stop').onclick=()=>safetyRelease('Score stopped.');
    byId('phrase-arch').onclick=()=>{safetyRelease();steps.forEach((step,index)=>{step.force=[18,35,65,83,70,48,31,12][index];step.velocity=[25,40,62,70,-65,-50,-35,-20][index];});refreshScore();};
    byId('phrase-reverse').onclick=()=>{safetyRelease();steps.reverse();refreshScore();};
    byId('score-export').onclick=()=>download({...metadata(),tempo:clamp(Number(byId('tempo').value)||80,30,240),subdivision:'eighth',loop:byId('loop').checked,steps:steps.map(step=>({force:step.force/100,velocity:step.velocity/100,note:byId('score-pitch').checked?step.note+60:null}))},'strings-control-score.json');
  }

  function gamepads(){try{return navigator.getGamepads?Array.from(navigator.getGamepads()).filter(Boolean):[];}catch{return [];}}
  function discoverPads(){
    const pads=gamepads();const signature=pads.map(pad=>`${pad.index}:${pad.id}:${pad.mapping}`).join('|');
    if(signature===padSignature)return;padSignature=signature;
    const previous=padIndex;
    if(!pads.some(pad=>pad.index===padIndex))padIndex=pads[0]?.index??null;
    byId('device').replaceChildren();
    if(!pads.length){const option=new Option('No controller detected','');byId('device').add(option);}else pads.forEach(pad=>byId('device').add(new Option(`${pad.index+1}. ${pad.id}${pad.mapping==='standard'?'':' (nonstandard)'}`,String(pad.index))));
    byId('device').value=padIndex===null?'':String(padIndex);
    if(previous!==padIndex){safetyRelease();neutral=[0,0,0,0];byId('calibration-status').textContent='Rest both sticks before capture. Session only.';}
    if(state.source==='gamepad')setStatus(pads.length?'Controller detected. Release LT to arm.':'Controller disconnected. Bow and notes released.');
  }
  byId('device').onchange=()=>{safetyRelease();padIndex=byId('device').value===''?null:Number(byId('device').value);neutral=[0,0,0,0];byId('calibration-status').textContent='Controller changed. Neutral capture reset.';};
  window.addEventListener('gamepadconnected',discoverPads);
  window.addEventListener('gamepaddisconnected',()=>{safetyRelease('Controller disconnected. Bow and notes released.');discoverPads();});
  ['force-map','engage-map','velocity-map','contact-map','vibrato-map','deadzone'].forEach(id=>byId(id).addEventListener('change',()=>{safetyRelease('Mapping changed. Return engagement trigger to rest.');}));
  byId('calibrate').onclick=()=>{
    const pad=gamepads().find(item=>item.index===padIndex);
    safetyRelease();
    if(!pad||pad.mapping!=='standard'){byId('calibration-status').textContent='Connect a standard controller first.';return;}
    if(pad.axes.slice(0,4).some(value=>Math.abs(value)>.35)){byId('calibration-status').textContent='Stick too far from rest. Release sticks and retry.';return;}
    neutral=pad.axes.slice(0,4);byId('calibration-status').textContent='Neutral captured. Deadzone applied around this center.';
  };
  function axis(pad,index){const raw=clamp((pad.axes[index]??0)-(neutral[index]??0),-1,1);const zone=sliderValue('deadzone');return Math.abs(raw)<=zone?0:Math.sign(raw)*(Math.abs(raw)-zone)/(1-zone);}
  function pollPad(){
    if(state.source!=='gamepad'||document.hidden||!document.hasFocus())return;
    const pad=gamepads().find(item=>item.index===padIndex);
    if(!pad){setStatus(navigator.getGamepads?'No controller exposed. Focus this page and press a controller button.':'Gamepad API unavailable in this browser. Use pointer input.');return;}
    if(pad.mapping!=='standard'){setStatus('Connected, nonstandard mapping. Input disabled; use a standard controller or pointer.');return;}
    const trigger=pad.buttons[Number(byId('engage-map').value)]?.value??0;
    if(trigger<.06)padArmed=true;
    const active=padArmed&&trigger>.08;
    if(active){
      if(owner!=='gamepad')manualTakeover('gamepad');state.engaged=true;
      state.force=clamp(pad.buttons[Number(byId('force-map').value)]?.value??0);
      state.velocity=axis(pad,Number(byId('velocity-map').value));
      state.contact=(axis(pad,Number(byId('contact-map').value))+1)/2;
      state.vibrato=Math.abs(axis(pad,Number(byId('vibrato-map').value)));
    }else if(owner==='gamepad')releaseBow();
    setStatus(`Connected · ${active?'bow engaged':padArmed?'ready; hold engagement trigger':'return engagement trigger to rest'}`);
    byId('device-caption').textContent=pad.id;
  }
  function frame(now){
    requestAnimationFrame(frame);
    if(now-previousFrame<32)return;previousFrame=now;
    discoverPads();pollPad();
    if(owner==='pointer'&&view==='bow'&&now-lastMotion>90){state.velocity*=.65;if(Math.abs(state.velocity)<.005)state.velocity=0;}
    if(phraseStart!==null){
      const tempo=clamp(Number(byId('tempo').value)||80,30,240);const elapsed=now-phraseStart;const index=Math.floor(elapsed/(30000/tempo));
      if(index>=8&&!byId('loop').checked){stopPhrase();releaseBow();}else{
        const current=index%8,step=steps[current];state.engaged=step.force>0;state.force=step.force/100;state.velocity=step.velocity/100;
        if(byId('score-pitch').checked)noteOn('score',step.note);else noteOff('score');
        if(current!==phraseStep){phraseStep=current;document.querySelectorAll('.step').forEach((element,position)=>element.classList.toggle('playing',position===current));byId('score-position').textContent=`Step ${current+1} / 8`;}
      }
    }
    if(replayStart!==null){
      const elapsed=now-replayStart;
      while(replayCursor<take.length-1&&take[replayCursor+1].time<=elapsed)replayCursor++;
      const sample=take[replayCursor];Object.assign(state,{force:sample.force,velocity:sample.velocity,contact:sample.contact,vibrato:sample.vibrato,bodyCoupling:sample.bodyCoupling,sympathetic:sample.sympathetic,rosin:sample.rosin,variation:sample.variation,room:sample.room,engaged:sample.engaged});
      if(sample.note===null)noteOff('replay');else noteOn('replay',sample.note);
      if(elapsed>=take[take.length-1].time){replayStart=null;noteOff('replay');releaseBow();setStatus('Replay complete. Bow released.');}
    }
    if(recordStart!==null){
      const elapsed=now-recordStart;recording.push({...controlSample(),time:Math.round(Math.min(20000,elapsed))});
      byId('take-status').textContent=`Recording ${(elapsed/1000).toFixed(1)} / 20 seconds`;
      if(elapsed>=20000)finishRecording();
    }
    ['force','velocity','contact','vibrato'].forEach(id=>{byId(`live-${id}`).textContent=`${id==='velocity'&&state[id]>0?'+':''}${percent(state[id])}`;byId(`meter-${id}`).value=state[id];});
    const currentNote=pitch();byId('pitch-value').textContent=currentNote===null?'No note held':`${noteNames[currentNote]} / MIDI ${currentNote+60}`;
    document.querySelectorAll('.key').forEach(button=>button.classList.toggle('held',Number(button.dataset.note)===currentNote));
    byId('live-state').textContent=`${state.engaged?'Bow engaged':'Bow at rest'} · ${state.instrument} ${state.ensemble?`/ ${state.players} requested players`:'/ solo'}${owner?` · ${owner==='score'?'control score':owner==='replay'?'recorded controls':owner==='gamepad'?'connected gamepad':'live input'}`:''}`;
    if(byId('surface-dot'))byId('surface-dot').classList.toggle('rest',!state.engaged);
    if(byId('contact-marker'))byId('contact-marker').style.top=`${75+state.contact*185}px`;
    if(engage)engage.setAttribute('aria-pressed',String(state.engaged));
    midi.update(state,currentNote===null?null:currentNote+60,now);
  }
  updateRanges();applyRecipe(false);renderStage();requestAnimationFrame(frame);
})();
