# S1Vave-Banks — Changelog

Fork of [S1Vave](https://github.com/AndrewGG88/S1Vave) 1.12.3 for the M-VAVE FM-1.
Full guide (Spanish): [docs/BANKS.md](docs/BANKS.md).

## Next (not released yet)

- **SEQ held + SELECT** = move between the sequencer views both ways (STEP+ back to STEPS in one turn). Tapping SEQ still goes forward.
- **PATTERNS view: K2 CLEAR** empties the slot chosen with K1 (turn twice: `AGAIN: CLEAR B3`). Playing, it clears at the stop.

## banks3 (1.12.3-banks3)

- **Silent writing**: with the track being written muted (GLO held + white 1 / 2), the keys in STEPS write without sounding.
- Screen: with the track muted, the variation is left off the top line so MUTE fits.

## banks2 (1.12.3-banks2)

- **Voicing per variation**: LOOP **K4 VOICE** (replaces PLAY) = SOUND / MONO / UNISON / POLY / CHORD per variation and track, laid over the sound (the program itself never changes).
- **Variation chain**: hold SAVE + white 13–16 one after another (up to 8), let go. Saved with the pattern.
- **Pattern chain**: hold SAVE + white 1–8 one after another (up to 16, banks can change in between). A pattern plays its whole variation chain before the next. Not saved (RAM only).
- **CHORD page** (SEL → ADV MIX → ADV FX → CHORD): K1 TYPE (MAJ, MIN, SUS4, 7, MIN7, POWER, OCT), K2–K4 each chord voice (OFF, −12..+12). Before, only reachable by MIDI CC 81–87.
- Going back to a pattern left with unsaved edits (by hand or by the chain) brings its edits back instead of the flash copy.
- Patterns saved by S1Vave 1.x and by banks1 load fine; new settings start at their defaults.
- Browser simulator rebuilt (docs/sim) with a Spanish quick guide.

## banks1 (1.12.3-banks1)

- **4 pattern banks (A–D)**: 32 patterns instead of 8. Bank A is the original 8 slots (existing patterns kept); banks B–D use free flash (0x99000).
- **4 variations per pattern, in pairs**: each keeps its own start/end for track A and track B; both switch together at the end of A's loop.
- Hold SAVE + white 9–12 = bank, white 13–16 = variation.
- New SEQ view **LOOP** (last in the SEQ cycle): ALGORITHM = track, K1/K2 start/end, K3 the variation edited, K4 PLAY it.
- Screen: patterns named `A1`…`D8`, the variation playing shown on top (`V2`, `V2>3` when queued).

## Not yet / ideas

- Tested in the simulator and the host tests only: **not yet on a real FM-1**.
- Song mode (repeat counts per pattern, SE-02 style).
- Arpeggiator per variation; a chord per variation (J-6 style).
- More than one bank on the SMK-37.
