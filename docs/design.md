# Strings UI design

## Direction: a modern instrument workbench

The instrument, not a rack of oscillators, owns the center of the screen.
An illuminated four-string drawing shows the bow contact location. A small
seating diagram communicates that an ensemble consists of individual players.
No photorealistic knobs, fake wood, or decorative oscilloscopes.

Palette: ink blue `#202c39`, deep slate `#17212c`, parchment `#eee8db`,
brass `#cbb27c`, cool silver `#9bb8c6`, muted rose `#bf8378`.
Typography: Cormorant Garamond for the instrument name; embedded DejaVu Sans
for controls. Left-aligned control labels; centered instrument and stage.

```
 Strings                         Solo / Ensemble       output
 Violin     Viola     Cello     Double bass
 ┌─────────────────────────────────┬──────────────────────┐
 │                                 │ The players          │
 │    Four-string / bow diagram    │ seating / section    │
 │    Violin                       │ Arco Pizz Tremolo    │
 └─────────────────────────────────┴──────────────────────┘
 Bow pressure    Bow speed    Contact point    Brightness
 Vibrato         Attack       Release          Room
 [              playable piano keyboard                  ]
```

Review: the first concept resembled a generic dark synthesizer. Revised to
put a string/contact-point illustration at its center, a serif instrument
name, and a seating view in place of oscillator tabs. Only the string drawing
is visually expressive; all other elements remain restrained.

The native plugin UI is the implemented mockup. `ui-mockup.png` is captured
from that executable, rather than a separate design that cannot be built.
