# Five workflows, one composer's job

This comparison concerns the original five Astra browser studies, not the
current native editor. The selected organic performance concept is implemented
in [native-performance.md](../ux-components/native-performance.md), with an
ivory/pine palette and a local expression-take workflow. The recommendations
below remain design trade-offs, not claims that every proposed page is native.

**Recommendation: Mood Field as the front door, Phrase Studio for revision.**
Use Material Lab as the deeper sound-design page, Conductor for section shaping,
and Bow Desk as an optional performance surface. This is an expert inspection
of Astra's five working prototypes and their control paths, **not a musician
study, listening-panel result, or proof that a mood label sounds convincing**.

## Same five tasks

| Study | Find an intimate tone | Build tension | Perform and revise a phrase | Shape an ensemble | Explore an unfamiliar sound |
| --- | --- | --- | --- | --- | --- |
| Bow Desk | Indirect: learn force/contact first | Continuous physical build, requires coordination | Strong live motion; a recorded take can replay, but individual points cannot be edited | Shared section selector, little orchestration guidance | Gesture experimentation, limited mechanical detail |
| Mood Field | Best entry: semantic starting recipes and a two-axis field | Direct journey between character points; inspect physical values underneath | Capture/replay a journey; no timeline correction | Shared section selector, not a seating workflow | Broad character exploration, less causal precision |
| Phrase Studio | Must author a contour rather than choose a tone | Strong repeatable swell across eight steps | Best revision: explicit pitch, force, signed motion, tempo, reverse and export | Section count available; no independent part lanes | Temporal experiments, little body/friction detail |
| Conductor | Close/chamber recipe and low energy | Ensemble energy/unity are musically legible | Hold-to-conduct is immediate; detailed revision needs another workflow | Best: section size, unity and distance with visible requested seats | Section texture, not deep individual-instrument design |
| Material Lab | Precise but demands physical vocabulary | Can change force/contact/rosin independently | Continuous settings and captured takes; no phrase timeline | Shared section selector only | Best: body coupling, sympathetic strings, friction and contact in one place |

These rankings follow the controls actually present, not just the appearance.
All five can emit notes and physical/configuration CCs through the opt-in MIDI
adapter. Captured traces were rendered through the same native VST3. The browser
does not run audio, sense emotion, measure force or read real instrument motion.

## Why not make the most violin-like interface the default?

A composer often knows “a restrained opening, then a brittle rise” before
knowing bow force or contact position. Mood Field reduces that translation cost.
It exposes the recipe rather than hiding it behind a magical emotion control.
Its weakness is that the labels are hypotheses: they need acoustic calibration
and musician listening before they become trustworthy musical shortcuts.

Phrase Studio answers the next question: “change only the peak, not the whole
take.” This is more useful for revisions than forcing repeated mouse bowing.
Its eight discrete eighth-note steps are currently a sketchpad, not a production
DAW editor: no continuous curves, tempo synchronization, multi-part score,
arbitrary duration, import or undo history is implemented.

For a dedicated experimental sound designer, Material Lab may be the better
initial page. It makes cause and effect inspectable and avoids claiming that
every unusual timbre corresponds to a conventional string technique.

## Keep and change before a production native redesign

- Keep the ink-blue/ivory/brass identity, large performance area, visible input
  availability, signed motion readout and independent pitch source.
- Keep a persistent input desk and panic across all workflows; show actual
  engine response separately from requested control values.
- Integrate Mood and Phrase in one persistent native session. **The current
  five routes are independent**; moving between them does not transfer takes.
- Use a compact native layout: these tall browser studies require scrolling,
  and Phrase's eight columns are cramped on small viewports even without overflow.
- Give controller mappings an always-visible mini legend during performance.
  Device calibration cannot replace an understanding of the musical mapping.
- Add audition-based macro calibration, continuous phrase curves and host tempo
  synchronization before treating this as a professional composition workflow.

## Evidence and limits

`tests/ux_bridge_smoke.py` exercises all five routes, keyboard-held expression,
MIDI output, focus-loss panic, mobile overflow and synthetic gamepad reversal /
disconnect / re-arm. `tests/ux_interaction_smoke.py` checks actual pointer motion,
stationary decay, take recording/replay/export, score edits and unknown mappings.
`tests/native_expression_pad_smoke.py` captures the current native editor's
pointer performance, recording/replay and cancellation into a WAV through a
JACK dummy device. `tests/native_gesture_smoke.py` forwards to this current
probe rather than using retired editor coordinates. Screenshots are actual
renders of those interfaces, not generated product photographs.

These checks prove implemented interactions and routing under the stated test
conditions. They do not establish comfort over a long session, controller feel,
human pressure sensing, live OS MIDI compatibility, emotion fit or acoustic
realism. A real controller and composer audition remain the next human tests.
