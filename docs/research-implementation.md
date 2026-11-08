# Research implementation ledger — 0.2 research build

The current 0.3 native performance implementation retains this bounded physical
model and its paper-by-paper dispositions. See
`ux-components/native-performance.md` for current UI/control behavior, the
sample-rate-independent time constants and new independent friction-residual
checks. New local reports/hashes live in `build/performance-validation/`.

This is an independently implemented reduced physical model, not a reproduction
of all seven papers, an acoustically calibrated digital twin, or a claim of
commercial parity. Primary PDFs were retrieved and their relevant sections read
on November 7, 2026. Papers are linked, not redistributed with the plugin.

## What replaced the old engine

Each player now owns four strings, each with two transverse traveling-wave
lanes. One string is stopped to the requested pitch; the other three retain
their open pitches. Three delay paths per lane represent the nut round trip
and the two bridge travel directions. Fractional linear interpolation, a lossy
nut reflection, and a first-order allpass approximate propagation, damping,
and dispersion. Pitch control compensates their phase delay and approximates
the loaded body's reflection phase. These filters are reduced models, not a
spatially resolved stiff-string PDE.

Eight mass/spring/damper body modes share a reciprocal two-polarization bridge
with all four strings. The symmetric modal participation matrices are positive
semidefinite outer products. This replaces the old output-only resonators:
bridge motion actually returns into the string delay lines, including the
unplayed strings. Sympathetic level is a transformer ratio, used in both force
collection and reflected velocity; it is not just a post-effect level.

Bow force is solved at the string junction, rather than applied as an amplitude
envelope to an independent oscillator. The contact solver enforces a static
friction bound, or finds a kinetic force with a bounded bisection solve. Bow
lift removes the force; zero speed produces no initial excitation. Signed bow
motion and tremolo reversals act on this contact. Pizzicato is a short force
pulse, not a sampled recording or a detailed finger-release collision.

The body-mode frequencies, masses, Q values, directions, string impedances,
and rosin/heat scaling are currently synthetic engineering profiles. Recognizable
registers and different resonant responses do not establish instrument identity
or perceptual realism. A weighted body-velocity/bridge-force observation and
simple room are used instead of solving radiated air pressure/directivity.

## Equations and test boundaries

For incoming bow waves `n,b`, string impedance `Z`, and force `F`, the junction
velocity is `v = n + b + F/(2Z)`. The force root is solved consistently with
`slip = v_bow - v`; outgoing waves receive the same `F/(2Z)` injection.
The kinetic friction curve adopts Desvages/Bilbao Equation 17a:

`mu(s) = sign(s) [0.4 exp(-|s|/0.01) + 0.45 exp(-|s|/0.1) + 0.35]`.

Its static limit is `1.2`. A rosin multiplier and a normalized, lumped heating
heuristic modify this curve. The heuristic uses dissipated slip power and a
cooling term; it is not Maestre's spatial heat equation, a calibrated temperature,
or a material measurement. The sign convention gives `F * slip >= 0` up to
floating-point tolerance. Bisection is bracketed by the static/kinetic bounds;
there is no sample-by-sample wave clipping to hide an unstable contact solver.

Each body mode integrates `m v' + r v + k x = a.F` with the trapezoidal rule.
With midpoint velocity `v_bar`, its discrete balance is
`E_next - E = dt [(a.F) v_bar - r v_bar^2]`, where
`E = (m v^2 + k x^2)/2`. Modal histories and positive instantaneous admittances
form a symmetric 2x2 bridge solve. String loads are included implicitly in that
solve, not inserted as a delayed feedback gain.

`tests/physical_tests.cpp` checks this mode/bridge energy identity at three rates,
decay without drive, nonnegative friction dissipation, force bounds, reciprocal
sympathetic transfer, and sustained audible excitation. These are component
properties: they do **not** constitute Desvages/Bilbao's global FDTD stability
proof for this different system, especially during delay retuning or gestures.
Changing pitch/contact moves effective boundaries and can perform work.

## Seven-paper matrix

### 1. Maestre, Spa & Smith — SIMPLIFIED / IMPLEMENTED architectural subset

Verified: *A Bowed String Physical Model Including Finite-Width Thermal Friction
and Hair Dynamics*, ICMC/SMC 2014, pp. 1305–1311; seven-page primary paper.
The source is ICMC/SMC, not DAFx. Relevant: Section 2's traveling waves and
two-polarization coupling; Section 5 / Equation 11's passive admittance matrix.

Code: `StringLane`, `PassiveBody::scatter`, `BodyMode` in
`src/dsp/PhysicalModel.cpp`. Positive modal outer products and reciprocal
scattering realize the architectural principle; trapezoidal oscillator states
are our implementation, not the paper's fitted reflectance filter coefficients.
Tests: bridge power balance and open-string energy transfer.

Sections 3–4's distributed thermal diffusion, finite bow width, separate hairs,
and bow-hair dynamics are **not implemented**. Our lumped heat variable is not
an equivalent substitute. Neither the paper's measured admittance fit nor
its full hybrid DW/FDTD bow contact has been reproduced.

Primary archive: https://zenodo.org/records/850848
DOI: 10.5281/zenodo.850848

### 2. Desvages & Bilbao — IMPLEMENTED friction curve; SIMPLIFIED physical model

Verified: *Applied Sciences* 6(5), 135, published May 10, 2016, 32 pages.
Relevant: Section 2.5 / Equations 15–18 (normal-force-dependent dry friction,
relative slip, and the static/kinetic friction curve); Sections 3–4's energy
accounting and numerical experiments.

Code: `BowContact::solve` adopts Equation 17a's two-exponential kinetic curve
and static bound, with explicitly additional rosin/heat scaling. `StringLane`
and `PassiveBody` provide two transverse lanes and body-mediated cross-coupling.
Tests: dissipated slip power, static bounds, bow lift, reversals, and fixed-body
energy accounting. Gesture controls are separated from the physical state.

Not reproduced: their FDTD string discretization, bow mass/force controller,
normal bow/string collision, dynamic stopping finger, neck/fingerboard
collisions, or full-system energy theorem. Our imposed bow velocity and fixed
bow-axis projection do not simulate those contact dynamics.

Primary PDF: https://mdpi-res.com/d_attachment/applsci/applsci-06-00135/article_deploy/applsci-06-00135.pdf
DOI: 10.3390/app6050135

### 3. Percival — SIMPLIFIED control architecture; learned feedback DEFERRED

Verified: Graham Keith Percival, University of Glasgow PhD thesis, 2013,
*Physical Modelling Meets Machine Learning: Performing Music with a Virtual
String Ensemble*, 216 PDF pages. Relevant: Chapters 3 (measurements), 5
(intelligent feedback control), and 6 (performance generation).

Adopted: separate instrument physics from the performer/control layer.
`docs/ux-performance/app.js` makes transparent expressive recipes, repeatable
control takes, and an editable phrase score; `midi.js` sends that performance
to the native `Engine::midi`. Ensemble players have separate physical states.
Tests: captured browser MIDI traces rendered through the actual VST3; divisi,
note release, automation, and repeatable control export.

Our performer is **open-loop authored mapping**, not Percival's trained SVM
quality classifier, closed-loop bow-force correction, or learned performer.
The thesis's measured constants/IR recordings are not bundled or represented
as our data. Its instrument measurements cover violin/viola/cello, not double
bass. Learned feedback is deferred until a listening-labelled training set and
audible quality criterion exist; a stable model alone cannot supply those labels.

Primary record: https://theses.gla.ac.uk/4253/

### 4. Luan & Scavone — EVALUATED / DEFERRED

Verified: arXiv:2505.18950, posted May 25, 2025; full eight-page PDF retrieved.
Relevant: Section 2's one-degree-of-freedom bowed mass/spring model, the PINN
and PI-DeepONet formulations, and the results/discussion. The work is not a
four-string instrument model. It reports force-dependent performance and
optimization conditioning limitations, with observation-data-assisted hybrids.

Training-domain check (Section 4.1 / Table 1, rechecked November 8): the
oscillator is fixed at 100 Hz, bow velocity 0.2 m/s and friction parameter
`a=100`. Separate experiments use mass-normalized bow forces `10, 100, 1000`
m/s², not those values in newtons. PINNs use zero initial conditions and
time windows of 0.1, 0.03 or 0.01 s; PI-DeepONets sample initial `(p,q)` in
`[-0.35,0.35]` for the two lower forces and `[-2,2]` for the highest, with
0.01-s windows. This is not training over arbitrary changing violin gestures.
The highest-force unsupervised operator fails; short-window causal PINNs and
an observation-assisted hybrid address different parts of that difficulty.
Table 1 reports 14.22–38.76 hours of training on an RTX 4080, not audio
inference timings. Section 5 explicitly excludes inference from the study:
there is no measured plugin speedup or real-time budget to adopt. Residual
optimization also supplies no global passivity guarantee for our coupled model.

No neural contact code, weights, training, or speedup is claimed. Our bounded
numerical solver is testable across the requested force range without an
unvalidated learned surrogate. A future experiment should reproduce the small
system first and compare error, force-range generalization, dissipation, and
inference cost against that solver. No inference-cost or training-domain claim
is extrapolated from this paper to the current VST.

Primary record: https://arxiv.org/abs/2505.18950

### 5. Zheleznov, Bilbao, Wright & King — EVALUATED / DEFERRED

Verified: arXiv:2601.10453; January 15, 2026 submission, March 15 revision;
ten-page PDF. Relevant: scalar auxiliary variable (SAV) solver construction,
neural ODE combination, and synthetic nonlinear transverse-string experiments.

Our linear passive body integration is not their SAV nonlinear solver. There
is no differentiable computation graph or trained nonlinear dynamics in this
build. Deferred because the current bridge is a linear modal admittance, while
the paper addresses learning nonlinear modal dynamics. Adopting its learning
method needs an identified nonlinear subsystem and fitting/validation data;
labelling ordinary modal integration "differentiable synthesis" would be false.

Primary record: https://arxiv.org/abs/2601.10453
DOI recorded by arXiv: 10.17743/jaes.2026.0269

### 6. Tablas de Paula, Smith, Välimäki & Reiss — IMPLEMENTED representation choices

Verified: arXiv:2604.12878, April 14, 2026; 21-page PDF. Relevant: Sections
1.1, 2.1, 2.2 and 2.4 on traveling waves, wave impedance/scattering, and strings.

Code: `WaveDelay`, `StringLane`, and reciprocal bridge scattering. We choose
waveguides over a full spatial FDTD grid for four strings per ensemble player;
first-order allpass dispersion and one-pole loss are deliberately low-order.
Measured render timings and the allocation test quantify this implementation,
not a general claim that waveguides always beat finite differences. Fractional
interpolation and time-varying delays remain accuracy limits. Parameter-fitting
or machine-learning methods reviewed in the paper are not automatically adopted.
The recorded Release runs on an i7-8700K-labelled four-vCPU Linux environment
rendered eight-second phrases about 30x real time for solo, 4.8x for eight players,
and 2.4x for sixteen. These include file writing and release/silence, not worst-case
audio-device deadlines. Inspect `build/research-validation/performance.json`
for the exact run and `tools/research_demos.py` to reproduce it.

Primary record: https://arxiv.org/abs/2604.12878
DOI recorded by arXiv: 10.17743/jaes.2026.0283

### 7. Chabassier, Chaigne & Joly — SIMPLIFIED whole-instrument principle

Verified: *ESAIM: M2AN* 48(5), 1241–1278 (2014); 38-page PDF.
Relevant: the model's excitation/string/bridge/soundboard/air decomposition and
reciprocal coupling, not its piano hammer or plate/air discretization.

Code: separate `BowContact`, strings, `PassiveBody`, and observation/room paths.
The bridge/body affects string motion rather than merely coloring its output.
Tests demonstrate that coupling changes dynamics and transfers energy.
Our observation mix is not a pressure field, measured body radiation, a plate
model, or an air solver. Piano hammer, rim, soundboard mesh, and vibroacoustic
field computation are not ported to this bowed instrument. Nothing here claims
knowledge of Pianoteq internals.

Primary record: https://www.numdam.org/item/M2AN_2014__48_5_1241_0/
DOI: 10.1051/m2an/2013136

## Measured calibration status

The seven sources establish relevant methods; they do not automatically grant
us their measurement files or fitted coefficients. No reusable raw complex
bridge-admittance dataset has been incorporated. Published response plots are
not relabelled as measured coefficient tables. This remains an acoustic-quality
gap, not proof that the numerical solver is invalid or commercial realism is
achieved. Measured bridge response and separate radiativity/directivity data
should precede a neural-surrogate effort.

The synthetic body profiles are explicitly labelled in `PhysicalModel.cpp`'s
provenance here. Their fixed-frequency/Q/mass choices test the architecture and
give distinct tonal responses; they must not be advertised as measured violins,
violas, cellos, or basses. Real instrument measurements and comparative musician
listening are still necessary before a professional-realism claim.

## Retrieved PDF fingerprints

These identify the reviewed copies, not files redistributed with this project.

| Paper | SHA-256 |
| --- | --- |
| Maestre | `597e5fa1d68be7ccd31ad89c7a714d3aff2d67f8ceab4110c15f51704831c7ec` |
| Desvages/Bilbao | `b88e32799370a99348a1a7c295b0a79beac42b3522694a84aac027767d283cfc` |
| Percival | `5c5f979e378b8e9b5437f3871362412edc31e588c92bf541e66f6f516feb4df9` |
| Luan/Scavone | `eb6f37d96c8a12d987ab582ebad90ec2547b8b08c27b5006f2966341da48a842` |
| Zheleznov et al. | `14fe6a9eac1d240775a23190fbe87a5fca327dad93ec2009dda0a7df7da43aa4` |
| Tablas de Paula et al. | `2595348685fab6fc9615245d3065df19328e7d5112b18242266e3c6350ce486a` |
| Chabassier et al. | `718aeddc094f2892fa6a61fe7f832fc9b72d0dd55c2974ef6e5a63b8d1f3ac18` |
