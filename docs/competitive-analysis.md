# Competitive landscape

Research date: **November 7, 2026**. Vendor documentation, not hands-on listening
tests. Prices are intentionally omitted: regional pricing and promotions vary.
These products are benchmarks, not libraries used in this repository.

| Product | Approach / scope | Strength to learn from | Strings 0.1.0 distinction |
|---|---|---|---|
| Audio Modeling SWAM Solo Strings | Sample-free physical modeling; violin, viola, cello, double bass | Deep bowing/fingering control; expressive monophonic playing | Open source and one shared instrument; far less acoustic calibration |
| Audio Modeling SWAM String Sections | Physically modeled ensembles | Section interaction, room placement and efficient player management | Explicit fixed-player divisi, but only simplified player/room interaction |
| Expressive E Soliste | Four physically modeled solo strings; MPE-oriented | Immediate gestural bow control and instrument-body customization | Ordinary MIDI keyboard usability first; does not claim full MPE support |
| Samplemodeling Solo, Chamber & Ensemble Strings | Sample-based/modeling hybrid in Kontakt | Continuous control across solo and ensemble configurations | No Kontakt or sample dependency; less recorded acoustic detail |
| VSL Synchron Strings Pro | Recorded ensemble samples with many articulations and microphone positions | Rich performance vocabulary and recorded room/body realism | Tiny generated instrument; no recorded legato/microphone library |

## Product decisions

1. **Performance before parameter count.** Expose pressure/speed/contact point,
   with safe defaults that produce sound from ordinary note-on messages.
2. **One player is not a section.** Independent waveguide/body state per player;
   section count stays fixed when chords divide it. Avoid chorus-only ensembles.
3. **Separate timbre from ambience.** Room can be turned off for exposed solo
   passages and dry evaluation. A lush reverb cannot establish acoustic fidelity.
4. **Show the mechanism.** The contact-point drawing and seating view correspond
   to real controls rather than decorative waveforms.
5. **Reproducibility as differentiation.** MIT/ISC dependencies, no activation,
   offline renders, tests, inspectable synthesis and reproducible CI packages.

## Honest gap to the benchmarks

This release implements a playable simplified model, not years of measured
instrument identification. It lacks full MPE configuration, realistic string
selection/fingering, double stops, measured bow transients, convincing bow
reversals, calibrated instrument/body responses and broad performer evaluation.
Neither unit tests nor pluginval establish perceptual indistinguishability.
Priorities after 0.1.0: recorded reference datasets with clear usage rights;
calibrated string/body measurements; ABX/listening panels; per-note expression;
energy-stable friction/dispersion; then richer articulations.

## Primary sources

- [SWAM Solo Strings manual](https://static.audiomodeling.com/manuals/strings/SWAM%20Solo%20Strings%20-%20User%20Manual%20-%20v3.0.3.pdf)
- [Audio Modeling: evolution of SWAM String Sections](https://kb.audiomodeling.com/support/solutions/articles/206000051098-the-creation-and-evolution-of-swam-string-sections)
- [Expressive E Soliste](https://www.expressivee.com/139-soliste)
- [Samplemodeling Strings manual](https://www.samplemodeling.com/wp-content/uploads/2025/03/USER-MANUAL-Solo-Chamber-and-Ensemble-Strings-v.-2.0.4.pdf)
- [VSL Synchron Strings Pro](https://www.vsl.co.at/products/synchron/strings-pro)
- [DPF licensing by format](https://github.com/DISTRHO/DPF/blob/main/LICENSING.md)
- [STK Bowed source](https://github.com/thestk/stk/blob/master/src/Bowed.cpp)
- [Julius O. Smith: Digital Waveguide Bowed-String](https://www.dsprelated.com/freebooks/pasp/Digital_Waveguide_Bowed_String.html)
