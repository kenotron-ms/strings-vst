# Third-party notices

Strings source is MIT licensed. Dependencies retain their original licenses.

- **DISTRHO Plugin Framework**, pinned git submodule in `third_party/dpf`:
  ISC. Its VST3 implementation uses the ISC-licensed travesty API definitions,
  not the official Steinberg SDK. See `third_party/dpf/LICENSE` and
  `third_party/dpf/LICENSING.md` (the main license is included in binary archives).
- **NanoVG / stb**, bundled by DPF: zlib-style license / public domain or MIT,
  as documented in `third_party/dpf/dgl/src/nanovg/`.
- **Pugl**, bundled by DPF: ISC, see
  `third_party/dpf/dgl/src/pugl-upstream/COPYING`.
- **Khronos OpenGL/EGL headers**, MIT: pinned sources and notices in
  `third_party/khronos/`, used only to compile the Windows OpenGL interface.
- **DejaVu Sans**, embedded by DPF: Bitstream Vera and public-domain DejaVu
  modifications. See `docs/DejaVu-LICENSE.txt`.
- **Cormorant Garamond**, Christian Thalmann / Cormorant project: SIL Open Font
  License 1.1. Font and full license in `assets/`; license included in archives.
- **RtAudio / RtMidi**, bundled by DPF for standalone audio/MIDI: permissive
  MIT-style licenses, see `docs/RtAudio-LICENSE.txt` and `docs/RtMidi-LICENSE.txt`.
- **The Synthesis ToolKit (STK)**, Perry R. Cook and Gary P. Scavone: permissive
  MIT-style license. The nonlinear bow-reflection equation and waveguide
  topology are informed by its Bowed/BowTable implementation. No STK library
  is linked. Full notice in `docs/STK-LICENSE.txt`.

pluginval and Pedalboard (GPLv3) are **test tools only**, downloaded by CI, and
are not linked into or shipped in Strings binary archives. NumPy is also a
test-only dependency. No proprietary samples, impulse responses, presets,
trademarks, or code from competing products are included.
