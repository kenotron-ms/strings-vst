# Research-driven expressive Strings VST

Deliver a working, research-grounded physical-modeling VST3 for solo and
ensemble violin, viola, cello, and double bass, together with five distinct
Astra-designed UX prototypes and an evidence-based recommendation for composers
and sound designers shaping mood, tone, and phrasing without a real instrument.

## Required outcomes

1. **Research becomes executable physics.** Verify the relevant primary papers
   and document the equations, assumptions, and code implementing this bounded
   model: nonlinear bow stick/slip, reciprocal bridge/body coupling, two string
   polarizations, sympathetic open strings, and frequency-dependent string loss
   and dispersion. Distinguish faithful implementations from simplified models,
   original engineering choices, and unimplemented research. The old engine may
   be replaced; keep existing code only where it usefully connects components.
   Complete the seven-paper research checklist below, with an inspectable
   research-to-code matrix rather than merely citing papers in documentation.

2. **The instrument is playable.** Build and exercise the native VST3 with four
   instrument profiles, solo and ensemble allocation, independent MIDI pitch,
   and continuous bow force, signed motion, lift, contact point, and vibrato.
   Retain host automation, parameter-state recall, note release, and panic.
   Controls must affect the physical excitation or coupled model rather than
   merely animate a display or reshape an unrelated synthesized signal.

3. **Ordinary inputs support expression.** Implement a usable mouse/touchpad
   gesture route and a standard PS5/Xbox-style controller route to the native
   engine, directly or through a documented adapter. Separate pitch selection
   from bow expression. Provide mappings, deadzones/calibration, and safe release
   on cancellation, focus loss, and disconnect. Distinguish measured pressure
   from position-based force mappings. Report physical hardware tests separately
   from simulated input tests and browser-only prototypes.

4. **Five meaningful UX alternatives exist.** Have Astra create five interactive
   mockups with genuinely different performance/composition workflows, not just
   different skins. Compare them using the same tasks: find an intimate tone,
   build tension, perform and revise a phrase, shape an ensemble, and explore
   an unfamiliar sound. Identify the strongest default for a composer/sound
   designer, explain its trade-offs, and distinguish expert inspection from
   actual musician testing. Clearly label mockup-only functionality.

5. **Validation has inspectable evidence.** Exercise the engine at 44.1, 48, and
   96 kHz; cover four instruments, solo/ensemble, finite bounded output, tuning,
   note releases, parameter extremes, bow reversal/lift, sympathetic transfer,
   body passivity, contact dissipation, and allocation-free audio processing.
   Measure render performance rather than assert real-time suitability. Test
   the built VST3 in an actual host for MIDI timing, audible output, automation,
   and state recall. Add representative audio renders for listening comparison
   and test the interactive UX routes for functional and input-safety failures.

6. **The result is usable and candid.** Deliver source changes, an identified
   working VST3 build, audition audio, the five mockups, their comparison, and
   reproducible build/run/test instructions. State remaining acoustic,
   calibration, hardware, and platform limitations. State which new work is
   local versus published; do not imply the existing release contains it.

## Required research checklist

Verify bibliographic details and technical claims against the linked papers or
their original publication sources before relying on them. This list records
the user's requested research leads, not a claim that every paper, result, or
implementation has already been verified. Record any corrections explicitly.

1. **Maestre, Spa & Smith (2014), "A Bowed String Physical Model Including
   Finite-Width Thermal Friction and Hair Dynamics."** Primary architectural
   priority: assess its waveguide/finite-difference combination, finite bow
   width, thermal friction, bow-hair dynamics, and passive bridge admittance.
   Implement the selected real-time coupling and bow-contact methods, and
   identify which finite-width, thermal, or hair effects remain approximations
   or unimplemented. Prioritize fitting bridge/body response to measured
   admittance when usable measurements and their reuse rights are available.
   A synthetic passivity fixture is useful evidence for the solver, but is not
   evidence of measured acoustic calibration.
   https://citeseerx.ist.psu.edu/document?doi=8533068e186c75c8c79a430f86d3fb5ba0609f15&repid=rep1&type=pdf

2. **Desvages & Bilbao (2016), "Two-Polarisation Physical Model of Bowed Strings
   with Nonlinear Contact and Friction Forces, and Application to Gesture-Based
   Sound Synthesis."** Physical-interaction priority: two-polarization string
   motion, nonlinear contact/friction, gesture control, and energy-balanced
   numerics. Connect adopted equations to stability/dissipation tests. Document
   the treatment or omission of bow-normal contact, stopping fingers, and
   fingerboard collisions; two waveguide lanes alone do not reproduce the full
   paper's model or establish its stability theorem for our implementation.
   https://www.research.ed.ac.uk/files/28188128/Bilbao_open_access_paper.pdf

3. **Graham Percival (2013), "Physical Modelling Meets Machine Learning:
   Performing Music with a Virtual String Ensemble," PhD thesis.** Performance
   priority: study the separation between instrument simulation and the virtual
   musician, including measured instrument parameters, body impulse responses,
   and feedback-controlled bow force. Apply this separation to controllable,
   repeatable solo/ensemble phrasing. Identify whether the delivered controller
   is open-loop mapping, rule-based feedback, or learned feedback; do not label
   simple MIDI scaling as a learned virtual performer.
   https://theses.gla.ac.uk/4253/

4. **Luan & Scavone (2025), "Physics-Informed Deep Learning for Nonlinear
   Friction Model of Bow-string Interaction," arXiv:2505.18950 / DAFx25.**
   Evaluate whether learned friction has a defensible role in this instrument.
   Record the paper's actual model scale, training domain, high-force behavior,
   inference cost, and stability limits. Either deliver and test a bounded
   experiment or document a technical deferral in favor of the validated
   numerical contact solver. Do not imply a small-system demonstration is a
   validated full bowed-string instrument.
   https://arxiv.org/abs/2505.18950

5. **Zheleznov, Bilbao, Wright & King (2026), "Stable Differentiable Modal
   Synthesis for Learning Nonlinear Dynamics," arXiv:2601.10453.** Evaluate its
   stable discretization and differentiable parameter-fitting approach. Record
   what its demonstrated data/model supports and whether it transfers to our
   bridge/body or string calibration. Implement and test an adopted method or
   explain a concrete deferral; do not claim a differentiable or trained engine
   when only a conventional modal solver has been implemented.
   https://arxiv.org/abs/2601.10453

6. **Tablas de Paula, Smith, Välimäki & Reiss (2026), "Four Decades of Digital
   Waveguides," arXiv:2604.12878.** Use the review to assess the chosen string
   representation, dispersion/loss filters, parameter fitting, and real-time
   computational trade-offs. Document the resulting architecture decisions and
   alternatives rejected, with measured cost for the implemented engine.
   https://arxiv.org/abs/2604.12878

7. **Chabassier, Chaigne & Joly (2014), "Time Domain Simulation of a Piano.
   Part 1: Model Description."** Whole-instrument reference: study reciprocal
   excitation/string/bridge/body/air coupling, then state which principles apply
   to a bowed instrument and which piano-specific mechanisms do not. Document
   our observation/radiation approximation and missing air/body interactions.
   This paper is not documentation of Pianoteq's proprietary implementation.
   https://www.numdam.org/item/M2AN_2014__48_5_1241_0/

For each entry, record source verification, relevant equations or sections,
adopted method, code location, test or experiment, implementation fidelity, and
remaining limitations. Use explicit dispositions: IMPLEMENTED, SIMPLIFIED,
EVALUATED/DEFERRED with technical rationale, or SOURCE-BLOCKED with retrieval
evidence. Do not count an inaccessible source as reviewed. An inaccessible
paper does not block independent implementation grounded in accessible sources.

Start with entries 1–3 for engine architecture, physical interaction, and
expressive control. Prioritize measured bridge/body coupling and controllable
bow gestures before adding neural components. The latter are research options,
not mandatory dependencies or a reason to postpone a working instrument.

## Terminal outcomes

Track each numbered outcome as PASS, FAIL with reproducible evidence, or BLOCKED
with a named external dependency and the evidence establishing it. Resolve
independent outcomes even when another is blocked. Show evidence as it becomes
available, and finish with a per-outcome ledger and artifact paths.

Exit with SUCCESS when the required outcomes pass. Alternatively, exit with an
explicit NON-SUCCESS report when a concrete blocker conclusively prevents the
remaining outcome from being reached in the available environment and no
independent required work remains actionable. Name the blocker and the precise
residual work; do not label a failed, untested, or blocked feature as delivered.
Difficulty, lack of acoustic validation, and passing smoke tests alone are not
evidence of commercial-grade realism or impossibility.

Record completion and evidence locally. If an authorized remote publication or
status update is attempted but refused, record the refusal and local artifacts
instead; do not retry indefinitely or claim remote success. Tear down temporary
servers/processes, or explicitly hand them over to the user with their address,
purpose, and shutdown instructions before exiting.

## Scope-outs for this iteration

- No claim of parity with a commercial instrument without comparative evidence.
- No copying or dependency on proprietary Pianoteq/SWAM internals or assets.
- No obligation to reproduce every cited paper, implement neural models, or
  build a full finite-element instrument/air simulation.
- No requirement for unavailable hardware to pass as physically tested; name
  its verification gap without blocking unrelated implementation or tests.
- No automatic assumption that standard gamepad access includes DualSense
  adaptive triggers, raw touchpad data, or pressure-sensitive laptop trackpads.
- No open-ended listening campaign, multi-day soak, or wait for outside approval
  as a condition of reaching a documented terminal outcome.

## Known — speed aid, not completion evidence

- The public repository is kenotron-ms/strings-vst, locally
  /home/ken/strings-vst. The existing v0.1.0 release is integration scaffolding,
  not evidence that the new model or UX has shipped.
- A new PhysicalModel implementation and its Engine integration are in progress;
  the initial engine regression suite passes. New physics-specific tests,
  acoustic assessment, host validation, and input integration remain necessary.
- Astra has been assigned the five UX alternatives. Existing prototype files
  are work in progress, not five completed or validated designs.
- Published models and measured calibration are different deliverables; current
  synthetic body parameters must not be described as measured instruments.
