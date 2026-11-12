/* SPDX-License-Identifier: GPL-3.0-only */
/* Copyright (C) 2026 Andrew GG — Roland S-1 parity engine */
/* The step sequencer: two tracks that share 256 steps (A from the front, B from the back, so a length change
 * never moves the other track's steps). A step holds a chord of up to four notes, a velocity, a gate (or a tie
 * into the next step), a ratchet (1-4 hits) and a chance. Each track has its own length, note value, swing and
 * transpose. A plays sound A; B plays it too, or its own program (trk[1].pad[0] = program + 1, pad[1] = how much of
 * it passes the effects by, 0 = none), straight into the voices (not through the arpeggiator).
 * The player runs in the audio ISR per control block, as the arpeggiator does; the main loop edits `pat`. */
#define SEQ_STEPS 256u
#define SEQ_CHORD 4u
#define GATE_TIE 127u                           /* a gate of 127: the notes hold into the next step */
#define NPAT NBLOB_ALL                          /* patterns, one flash sector each (S1Vave-Banks: 8 per bank) */
#define NPAT_BANK 8u                            /* patterns per bank: SAVE held + white keys 1-8 */
#define NBANK (NPAT / NPAT_BANK)                /* banks A, B, C, D (SAVE held + white keys 9-12) */
#define NVAR 4u                                 /* variations per pattern: track A plays one start..end part */
#define VAR_FIN 255u                            /* a variation's end: "FIN", track A's last step */
#define VCHAIN_MAX 8u                           /* a variation chain: up to 8 (SAVE held + white 13-16, one after another) */
#define PAT_MAGIC 0x31535653u                   /* "SVS1" */

typedef struct { uint8_t note[SEQ_CHORD], vel, gate, ratch, chance; } step_t;   /* note 0 = none; ratch 0..3 = 1..4 hits */
#define STEP_OFF 0x80u                          /* in ratch: the step is switched off, its notes kept */
typedef struct { uint16_t len; uint8_t div, swing; int8_t transp; uint8_t mute, pad[2]; } seqtrk_t;   /* swing 50..75 %; pad: B's sound */
#define B_PROG pad[0]                           /* track B: its own program + 1, 0 = it plays sound A */
#define B_DRY pad[1]                            /* track B's own sound: 0 all into the effects .. 127 all dry */
typedef struct {
    uint32_t magic;
    seqtrk_t trk[2];
    step_t st[SEQ_STEPS];
    int16_t draw_table[16];   /* OSC DRAW custom 16-step wavetable */
    uint16_t chop_mask[4];    /* OSC CHOP masks: [0]=Sqr, [1]=Saw, [2]=Sub, [3]=Noise */
    int8_t master_prob;       /* Master probability (-100..+100) */
    uint8_t step_loop_start;  /* Step loop start index */
    uint8_t step_loop_end;    /* Step loop end index */
    uint8_t tempo;            /* Pattern tempo (BPM, 40..240) */
    uint8_t pad[4];
    /* S1Vave-Banks: up to here the 2116 bytes of a saved pattern from before; an old one loads into this prefix */
    uint8_t var_lo[2][NVAR];  /* each variation's first step, per track (A, B; 0..255) */
    uint8_t var_hi[2][NVAR];  /* its last step, VAR_FIN = the track's last */
    uint8_t var_cur;          /* the variation both tracks play (they change together) */
    uint8_t var_pad[3];
    /* S1Vave-Banks 2: up to here the 2136 bytes of a banks-1 pattern */
    uint8_t var_voice[2][NVAR];   /* each variation's voicing per track: 0 = the sound's own, else Mono..Chord + 1 */
    uint8_t vchain[VCHAIN_MAX];   /* the variation chain: one loop each, then again from the first */
    uint8_t vchain_n;             /* its length (0, 1: no chain) */
    uint8_t vchain_pad[3];
    /* S1Vave-Banks 5: up to here the 2156 bytes of a banks2-4 pattern */
    int8_t var_transp[2][NVAR];   /* each variation's transpose per track, -24..+24 (over the track's TRANSP) */
} pattern_t;
#define PAT_SIZE_V1 2116u                       /* a pattern saved by S1Vave 1.x (no variations) */
#define PAT_SIZE_B1 2136u                       /* a pattern saved by S1Vave-Banks 1 (no chain, no voicing) */
#define PAT_SIZE_B2 2156u                       /* a pattern saved by S1Vave-Banks 2-4 (no transpose per variation) */
_Static_assert(__builtin_offsetof(pattern_t, var_voice) == PAT_SIZE_B1, "S1Vave-Banks: the banks-1 pattern must be the prefix");
_Static_assert(__builtin_offsetof(pattern_t, var_transp) == PAT_SIZE_B2, "S1Vave-Banks: the banks-2 pattern must be the prefix");
_Static_assert(__builtin_offsetof(pattern_t, var_lo) == PAT_SIZE_V1, "S1Vave-Banks: the old pattern must be the prefix");
_Static_assert(sizeof(pattern_t) <= SETTINGS_MAX, "a pattern fits its sector");

static pattern_t pat;                           /* the pattern now: the main loop edits, the ISR plays */
static step_t *seq_step(uint32_t t, uint32_t i) { return &pat.st[t ? SEQ_STEPS - 1u - i : i]; }
static uint32_t seq_b_own(void) { return pat.trk[1].B_PROG != 0u; }
static uint32_t seq_b_dry(void) { return b_own ? (pat.trk[1].B_DRY > 127u ? 127u : pat.trk[1].B_DRY) : 0u; }
static inline const int16_t *seq_draw_table(void) { return pat.draw_table; }
static inline const uint16_t *seq_chop_masks(void) { return pat.chop_mask; }

/* main loop -> ISR */
static volatile uint32_t ctl_seq_run, ctl_seq_restart;
/* the next pattern, as zp12 switches loops: the main loop reads it into pat_q and sets ctl_pat_swap; the ISR swaps it
 * in when track A starts over (the one it leaves goes to pat_old, so its edits can be saved at the stop) */
static pattern_t pat_q, pat_old;
static volatile uint32_t ctl_pat_swap, ctl_pat_keep, ui_pat_swapped;   /* keep: the one left goes to pat_old */
static volatile uint32_t ctl_var_swap, ctl_var_next, ui_var_swapped;   /* the next variation, at track A's loop end */
/* the variation chain's place (0xFF: before its first; ctl_vchain_restart: from its first at A's next loop end), the
 * pattern's cycles (a loop of A, or its whole variation chain; the pattern chain moves on them), ctl_pat_cycle: the
 * pattern waiting in pat_q comes in only at the end of a cycle */
static volatile uint32_t vchain_pos = 0xFFu, ctl_vchain_restart, ui_cycles, ctl_pat_cycle;
/* ISR -> main loop: where each track is (step, and how far into it, Q8) */
static volatile uint32_t ui_seq_pos[2], ui_seq_frac[2], ui_seq_dur[2], ui_seq_ticks, ui_seq_beat;   /* dur: frames */

/* ---- banks 5: no transpose per variation */
static void pat_v5_init(pattern_t *p)
{
    uint32_t i;
    for (i = 0; i < NVAR; i++) p->var_transp[0][i] = p->var_transp[1][i] = 0;
}

/* ---- banks 2: the voicing per variation (all "the sound's own") and no variation chain */
static void pat_v2_init(pattern_t *p)
{
    uint32_t i;
    for (i = 0; i < NVAR; i++) p->var_voice[0][i] = p->var_voice[1][i] = 0;
    for (i = 0; i < VCHAIN_MAX; i++) p->vchain[i] = 0;
    p->vchain_n = 0;
    p->vchain_pad[0] = p->vchain_pad[1] = p->vchain_pad[2] = 0;
    pat_v5_init(p);
}

/* ---- the variations, in pairs: one number for both tracks, each track with its own start..end.
 * A: 1 = the whole track (as before), 2..4 = steps 17-32, 33-48, 49-64; B: the whole track in all four */
static void pat_var_init(pattern_t *p)
{
    uint32_t v;
    for (v = 0; v < NVAR; v++) {
        p->var_lo[0][v] = (uint8_t)(v * 16u); p->var_hi[0][v] = (uint8_t)(v * 16u + 15u);
        p->var_lo[1][v] = 0; p->var_hi[1][v] = VAR_FIN;
    }
    p->var_hi[0][0] = VAR_FIN;
    p->var_cur = 0;
    p->var_pad[0] = p->var_pad[1] = p->var_pad[2] = 0;
    pat_v2_init(p);
}
/* the steps track t plays in variation v (the track `len` long): [*lo, *hi]; one that starts past the end: all */
static void pat_var_span(const pattern_t *p, uint32_t t, uint32_t v, uint32_t len, uint32_t *lo, uint32_t *hi)
{
    uint32_t a = p->var_lo[t & 1u][v % NVAR], b = p->var_hi[t & 1u][v % NVAR];
    if (!len) { *lo = *hi = 0; return; }
    if (b == VAR_FIN || b >= len) b = len - 1u;
    if (a >= len) { a = 0; b = len - 1u; }
    if (a > b) a = b;
    *lo = a; *hi = b;
}

/* ---- a fresh pattern */
static void step_clear(step_t *s)
{
    uint32_t i;
    for (i = 0; i < SEQ_CHORD; i++) s->note[i] = 0;
    s->vel = 100;
    s->gate = 96;                               /* 76 % */
    s->ratch = 0;
    s->chance = 100;
}
static void pat_init(pattern_t *p)
{
    uint32_t i, t;
    p->magic = PAT_MAGIC;
    for (i = 0; i < SEQ_STEPS; i++) step_clear(&p->st[i]);
    for (t = 0; t < 2u; t++) {
        p->trk[t].len = t ? 0 : 16;             /* B off until it gets a length */
        p->trk[t].div = D_16;
        p->trk[t].swing = 50;
        p->trk[t].transp = 0;
        p->trk[t].mute = 0;
        p->trk[t].pad[0] = p->trk[t].pad[1] = 0;
    }
    for (i = 0; i < 16u; i++) p->draw_table[i] = (int16_t)(((int32_t)i - 7) * 4096);
    p->chop_mask[0] = 0xFFFFu;
    p->chop_mask[1] = 0xFFFFu;
    p->chop_mask[2] = 0xFFFFu;
    p->chop_mask[3] = 0xFFFFu;
    p->master_prob = 0;
    p->step_loop_start = 0;
    p->step_loop_end = 15;
    p->tempo = 120;
    pat_var_init(p);
}
static void pat_factory(uint32_t n, pattern_t *p)   /* a pattern never saved: empty, track A 16 sixteenths */
{
    pat_init(p);
    if (n < NFACT) {
        uint32_t i;
        for (i = 0; i < 16u; i++) p->draw_table[i] = FACT_WAVE[n].draw[i];
        for (i = 0; i < 4u; i++) p->chop_mask[i] = FACT_WAVE[n].chop[i];
        p->tempo = FACT_TEMPO[n];
    }
}

/* ---- the player (ISR, per CTL block) */
typedef struct { uint32_t cnt, dur, step, sub, rat_n, rat_i, gate_at, nsnd, play, tie, part; uint8_t snd[SEQ_CHORD]; } seqrun_t;
static seqrun_t sr[2];
static uint32_t seq_was, seq_beat_cnt, seq_beat_n, count_left;

static void seq_release(seqrun_t *r)
{
    uint32_t i;
    for (i = 0; i < r->nsnd; i++) voice_release(r->part, r->snd[i], 1);
    r->nsnd = 0;
    r->part = r == &sr[1] && b_own;             /* (what sounds lets go by the part it began on) */
}
static uint32_t seq_note(const step_t *s, uint32_t i, int32_t transp)
{
    return (uint32_t)clampi((int32_t)s->note[i] + transp, 1, 127);
}
static void seq_sound(seqrun_t *r, const step_t *s, int32_t transp)
{
    uint32_t i;
    for (i = 0; i < SEQ_CHORD && s->note[i] && r->nsnd < SEQ_CHORD; i++) {
        uint32_t n = seq_note(s, i, transp);
        voice_trigger(r->part, n, s->vel ? s->vel : 100u);
        r->snd[r->nsnd++] = (uint8_t)n;
    }
}
static uint32_t seq_has(const step_t *s, uint32_t m, int32_t transp)   /* the step plays note m */
{
    uint32_t i;
    for (i = 0; i < SEQ_CHORD && s->note[i]; i++)
        if (seq_note(s, i, transp) == m) return 1;
    return 0;
}

/* a tie, note by note: what the new step plays again keeps sounding, the rest lets go, the new ones start */
static void seq_tie_into(seqrun_t *r, const step_t *s, int32_t transp, uint32_t play)
{
    uint32_t i, j, keep = 0;
    for (i = j = 0; i < r->nsnd; i++) {
        if (play && seq_has(s, r->snd[i], transp)) { r->snd[j++] = r->snd[i]; keep++; }
        else voice_release(r->part, r->snd[i], 1);
    }
    r->nsnd = j;
    if (!play) return;
    for (i = 0; i < SEQ_CHORD && s->note[i] && r->nsnd < SEQ_CHORD; i++) {
        uint32_t m = seq_note(s, i, transp);
        for (j = 0; j < keep && r->snd[j] != m; j++) ;
        if (j < keep) continue;
        voice_trigger(r->part, m, s->vel ? s->vel : 100u);
        r->snd[r->nsnd++] = (uint8_t)m;
    }
}

/* S1Vave-Banks: the variation chain's length (0: none) */
static uint32_t seq_vchain_n(void)
{
    uint32_t n = pat.vchain_n;
    return n >= 2u ? (n > VCHAIN_MAX ? VCHAIN_MAX : n) : 0u;
}
/* the voicing of the variation playing, laid over each part's sound (B on sound A: A's). A change lets go of what
 * that part holds first, so nothing started in one voicing is let go in another */
static void seq_vmode_sync(void)
{
    uint32_t v = pat.var_cur % NVAR, t, k;
    for (t = 0; t < 2u; t++) {
        uint32_t want = t == 0u || b_own ? pat.var_voice[t][v] : 0u;
        if (want > NPOLY_MODE) want = 0;
        if (want == vmode_ovr[t]) continue;
        for (k = 0; k < 2u; k++) if (sr[k].part == t) { seq_release(&sr[k]); sr[k].tie = 0; }
        voices_release_part(t);
        vmode_ovr[t] = (uint8_t)want;
    }
}
/* both tracks to variation v (at A's loop end): B starts its new part with A when its range changes */
static void seq_var_to(uint32_t v)
{
    uint32_t lb = pat.trk[1].len, b0, b1, n0, n1;
    v %= NVAR;
    if (v == pat.var_cur % NVAR) return;
    pat_var_span(&pat, 1, pat.var_cur, lb, &b0, &b1);
    pat.var_cur = (uint8_t)v;
    ui_var_swapped++;
    pat_var_span(&pat, 1, v, lb, &n0, &n1);
    if (lb && (n0 != b0 || n1 != b1)) {         /* (its next step, this block: its start) */
        seq_release(&sr[1]);
        sr[1].tie = 0;
        sr[1].step = 0xFFFFFFFFu;
        sr[1].cnt = sr[1].dur;
    }
    seq_vmode_sync();
}

static void seq_tick(void)
{
    uint32_t t, q;
    seq_vmode_sync();                           /* (the variation's voicing, playing or not) */
    if (!ctl_seq_run) {
        if (seq_was) { seq_release(&sr[0]); seq_release(&sr[1]); seq_was = 0; }
        return;
    }
    if (!seq_was || ctl_seq_restart) {          /* start: step 0 of both tracks in this block */
        ctl_seq_restart = 0;
        for (t = 0; t < 2u; t++) {
            seq_release(&sr[t]);
            sr[t].step = 0xFFFFFFFFu;
            sr[t].cnt = sr[t].dur = 0;
            sr[t].play = sr[t].tie = 0;
        }
        seq_beat_cnt = 0;
        seq_beat_n = 0;
        vchain_pos = 0xFFu;                     /* (the variation chain from its first) */
        count_left = ctl_count ? 4u : 0u;
        seq_was = 1;
    }
    q = ctl_ext_quarter ? ctl_ext_quarter : 60u * FS / clampi(ctl_tempo, 40, 240);
    if (seq_beat_cnt == 0u) {                   /* a beat: the PLAY light, the click, the count-in */
        if (ctl_count && !count_left) {         /* counted in: the tracks from the top, now */
            ctl_count = 0;
            seq_beat_n = 0;
            for (t = 0; t < 2u; t++) { sr[t].step = 0xFFFFFFFFu; sr[t].cnt = sr[t].dur = 0; sr[t].play = sr[t].tie = 0; }
            vchain_pos = 0xFFu;
        }
        ui_seq_beat++;
        if (ctl_click || ctl_count) click_start(seq_beat_n % 4u == 0u);
        seq_beat_n++;
        if (count_left) count_left--;
        ui_seq_count = ctl_count ? count_left + 1u : 0u;
    }
    seq_beat_cnt += CTL;
    if (seq_beat_cnt >= q) seq_beat_cnt = 0;
    if (ctl_count) return;                      /* counting in: no steps yet */
    for (t = 0; t < 2u; t++) {
        seqrun_t *r = &sr[t];
        const seqtrk_t *k = &pat.trk[t];
        const step_t *s;
        int32_t tr;
        uint32_t len = k->len;
        if (!len) { seq_release(r); r->tie = 0; continue; }
        if (r->cnt >= r->dur) {                 /* the next step */
            uint32_t base = q / DIV_PER_QUARTER[k->div % NDIV], sw;
            r->cnt -= r->dur;
            {                                   /* each track: its start..end in the variation playing */
                uint32_t lo, hi;
                pat_var_span(&pat, t, pat.var_cur, len, &lo, &hi);
                if (r->step >= hi || r->step < lo) {   /* from the top */
                    uint32_t first = r->step == 0xFFFFFFFFu;   /* (the start: not the end of a loop) */
                    r->step = lo;
                    if (t == 0u) {                      /* A from the top: the next pattern, variation, chain step */
                        uint32_t vn = seq_vchain_n(), cyc = !first && (!vn || vchain_pos + 1u >= vn);
                        if (cyc) ui_cycles++;           /* (a loop, or the whole variation chain: the pattern chain moves) */
                        if (ctl_pat_swap && (!ctl_pat_cycle || cyc || first)) {   /* the next pattern */
                            if (ctl_pat_keep) pat_old = pat;
                            pat = pat_q;
                            if (ctl_bq_swap) { live_b = live_bq; ctl_bq_swap = 0; }   /* B's sound comes along */
                            ctl_pat_swap = 0;
                            ctl_pat_cycle = 0;
                            parts_point();
                            ui_pat_swapped++;
                            vchain_pos = 0xFFu;         /* (its own chain, from its first) */
                            vn = seq_vchain_n();
                            len = k->len;
                            seq_vmode_sync();
                            if (!len) { seq_release(r); r->tie = 0; continue; }
                        }
                        if (ctl_vchain_restart) { ctl_vchain_restart = 0; vchain_pos = 0xFFu; }
                        if (ctl_var_swap) {             /* the next variation (SAVE held + a white 13-16), both tracks */
                            ctl_var_swap = 0;
                            seq_var_to(ctl_var_next);
                        } else if (vn) {                /* the chain's next (one loop each) */
                            vchain_pos = vchain_pos >= vn ? 0u : (vchain_pos + 1u) % vn;
                            seq_var_to(pat.vchain[vchain_pos]);
                        }
                        pat_var_span(&pat, 0, pat.var_cur, len, &lo, &hi);
                        r->step = lo;
                    }
                } else r->step++;
            }
            sw = base * (uint32_t)(clampi(k->swing, 50, 75) - 50) / 50u;
            r->dur = r->step & 1u ? base - sw : base + sw;   /* swing: the first of each pair longer */
            if (r->dur < CTL) r->dur = CTL;
            s = seq_step(t, r->step);
            tr = k->transp + pat.var_transp[t][pat.var_cur % NVAR];   /* S1Vave-Banks: + the variation's */
            uint32_t eff_chance = s->chance;
            if (eff_chance < 100u) {
                int32_t c = (int32_t)eff_chance + (int32_t)pat.master_prob;
                eff_chance = (uint32_t)clampi(c, 0, 99);
            }
            r->play = s->note[0] && !(s->ratch & STEP_OFF) && !k->mute && (s->chance >= 100u || xorshift(&rnd_state) % 100u < eff_chance);
            if (r->tie)                         /* a tie: the notes the step plays again hold */
                seq_tie_into(r, s, tr, r->play);
            else {
                seq_release(r);
                if (r->play) seq_sound(r, s, tr);
            }
            r->rat_n = (s->ratch & 3u) + 1u;
            r->sub = r->dur / r->rat_n;
            r->rat_i = 0;
            r->tie = r->play && s->gate >= GATE_TIE;
            r->gate_at = s->gate >= GATE_TIE ? 0xFFFFFFFFu : (uint32_t)clampi((int32_t)((r->sub * (s->gate + 1u)) >> 7), CTL, (int32_t)r->sub);
            ui_seq_ticks++;
        } else {
            s = seq_step(t, r->step < len ? r->step : 0u);
            if (r->play && r->rat_n > 1u && r->sub) {   /* a ratchet: the next hit */
                uint32_t i = r->cnt / r->sub;
                if (i > r->rat_i && i < r->rat_n) { r->rat_i = i; seq_release(r); seq_sound(r, s, k->transp + pat.var_transp[t][pat.var_cur % NVAR]); }
            }
        }
        if (r->nsnd && r->gate_at != 0xFFFFFFFFu && r->cnt - r->rat_i * r->sub >= r->gate_at) seq_release(r);
        r->cnt += CTL;
        ui_seq_dur[t] = r->dur;
        ui_seq_pos[t] = r->step;
        ui_seq_frac[t] = r->dur ? r->cnt * 256u / r->dur : 0u;
    }
}
