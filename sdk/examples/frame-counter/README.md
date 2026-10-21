# Frame counter

An example mod for authors, using each part of OpenRAC's mod interface: it
refuses a game it is not for (`fail`), reads its two settings, counts the
game's ticks (`on_frame`) and takes the `Pad_ProcessReport` hook point,
counting the reports and calling what it replaced, the way several mods on one
hook point chain. Every `every` ticks it writes both counts in the game's log.
It changes nothing in the game.

| Setting | | Default |
|---|---|---|
| `every` | a line every so many ticks | 600 (about 12 s on PAL) |
| `verbose` | the lines in the normal log, not only in the detailed one | off |
