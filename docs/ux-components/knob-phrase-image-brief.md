# Knob-led performance concept

Status: prepared November 8, 2026. No new image API call has been made. The
built-in image tool is unavailable and explicit fallback permission is pending.
This is a prompt/design brief, not a generated mockup or implementation proof.

## Design plan and critique

- Color: ivory `#F6F5F0`, pine `#204C42`, ink `#203B35`, sage `#DCE2D5`,
  brass `#BA914F`, muted `#66766D`.
- Type: Cormorant Garamond instrument title; clear sans-serif control labels.
- Layout: instrument/preset/listening controls above three generous knobs;
  pitch/time phrase and bow-weight lane below. Left-aligned musical context;
  centered dial labels. Physical detail is progressively disclosed.
- Purpose: pitches remain familiar; continuous changes feel like performance,
  not parameter administration. The phrase is editable, not a decorative spark.

An initial large XY surface would repeat the earlier mouse-bowing assumption;
replace it with Energy, Tone and Vibrato. Avoid turning the replacement into
a conventional 16-knob bank. Restrict distinctive shading to the actual phrase
envelope, leaving note edges, values and playhead readable. No hardware hero,
mouse illustration, floating blob, embossed controls or textured wallpaper.

```text
Strings    Violin / Solo    Preset          Audition / Stop / Record

Play notes. Shape the feeling.                   Input / details

       Energy              Tone              Vibrato
       (dial)             (dial)              (dial)
    light—strong       round—edgy         still—expressive

Local phrase       bar 1          bar 2          bar 3          bar 4
Pitch/time         [notes]-----[notes]--------[notes]--------------
Bow weight         ____/--------\____/------------\______________
                   restrained fill beneath the real expression curve

Original / Take / Undo          Follow controls     Instrument details
```

## Image prompt

Use case: ui-mockup
Asset type: front-facing desktop VST instrument concept, not a photo or device.
Primary request: A refined, buildable expressive string plugin for a composer
who plays MIDI notes, listens, then shapes their physical-model performance.
Make three expressive dials the focus: Energy, Tone, Vibrato. Their arcs,
values and labels must be readable at normal plugin size. Give the lower
phrasing area an interesting but very subtle backdrop derived from the bow
weight curve, not arbitrary ornament. Preserve a useful piano-roll pitch/time
grid, clear note rectangles, playhead and a separate editable expression lane.
Style: restrained ivory/pine/sage/brass; flat NanoVG-achievable circles, arcs,
paths and text. A little bespoke character, never a photoreal hardware panel.
Text: "Strings", "Violin", "Solo", "Audition", "Stop", "Record expression",
"Energy", "Tone", "Vibrato", "Local phrase", "Bow weight", "Original",
"Take", "Undo", "Instrument details".
Constraints: No mouse illustration or giant gesture pad. No complicated texture,
organic blob, fake string-displacement plot, training/AI claims, or hardware
connection claim. Do not copy Ableton branding or its assets. No perspective,
monitor frame, photo background or tiny illegible control bank. The result is
a concept image, not a screenshot of shipped functionality.

References available locally: selected historical generated concept
`../ux-production-storyboard/images/06-organic-performance.png` for palette,
and `screenshots/performance-knobs-native.png` for current native geometry.
Neither reference makes an image-model result an executable interface.
