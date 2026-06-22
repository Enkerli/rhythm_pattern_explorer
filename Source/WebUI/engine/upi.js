/**
 * Serpe — UPI engine. Parses the Universal Pattern Input notation and
 * provides the generators + transforms. Framework-agnostic, no DOM.
 *
 * A pattern is { steps:Uint8Array(n) of 0/1, accents:Uint8Array(n) of 0/1 }.
 * All numeric notation is LEFTMOST = LSB (first step = bit 0, so step k has
 * value 2^k; 0x1:4 = 1000, tresillo 10010010 = 0x49), per CONVENTIONS.md.
 */

/* ── number theory ─────────────────────────────────────────────────── */
export function primeFactors(n) {
  const f = [];
  let d = 2, m = n;
  while (m > 1) {
    while (m % d === 0) { f.push(d); m /= d; }
    d++;
    if (d * d > m && m > 1) { f.push(m); break; }
  }
  return f;
}

import { euclideanRhythm, barlowTransform as barlowCore, barlowIndispensabilityTable } from './rhythm.js';

/* ── generators ────────────────────────────────────────────────────── */

/** Euclidean rhythm (canonical Bjorklund, rotated so the first onset is at 0). */
export function euclid(k, n) { return euclideanRhythm(k, n); }

/** Regular k-gon mapped onto n steps, rotated by offset steps. */
export function polygon(k, offset, n) {
  const steps = new Array(n).fill(0);
  if (k <= 0) return steps;
  for (let i = 0; i < k; i++) {
    const pos = Math.round((i * n) / k) % n;
    steps[(pos + offset) % n] = 1;
  }
  return steps;
}

/** Random onsets with a bell-curve bias toward `density` (0..1). */
export function randomPattern(k, n) {
  const idx = [...Array(n).keys()];
  for (let i = idx.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [idx[i], idx[j]] = [idx[j], idx[i]];
  }
  const steps = new Array(n).fill(0);
  for (let i = 0; i < Math.min(k, n); i++) steps[idx[i]] = 1;
  return steps;
}

export function rotate(steps, by) {
  const n = steps.length;
  if (n === 0) return steps.slice();
  const r = ((by % n) + n) % n;
  return steps.map((_, i) => steps[(i - r + n) % n]);
}

export function invert(steps) {
  // reflect about step 0 (retrograde of the cycle)
  const n = steps.length;
  return steps.map((_, i) => steps[(n - i) % n]);
}

export function complement(steps) {
  return steps.map((s) => (s ? 0 : 1));
}

/* ── numeric notation (leftmost = LSB) ─────────────────────────────────
 * Strict left-to-right: the first step is bit 0, so step k has value 2^k.
 * 0x1:4 = 1000, 0x8:4 = 0001, tresillo 10010010 = 0x49. Matches the C++
 * engine and @enkerli/theory. */
function bitsFromValue(value, width) {
  const steps = new Array(width).fill(0);
  for (let i = 0; i < width; i++) {
    steps[i] = (value >> i) & 1;
  }
  return steps;
}

/* ── parser ────────────────────────────────────────────────────────── */
/**
 * Parse one UPI token into a pattern. Supports the headline grammar:
 *   E(k,n) / E(k,n,rot)      Euclidean
 *   P(k,off) / P(k,off,n)    polygon
 *   R(k,n)                   random
 *   B(k,n) / W(k,n)          Barlow / Wolrab (anti-Barlow) of length n
 *   D(k,n)                   Dilcue (anti-Euclidean)
 *   0x49 / b10010010 / 10010010 / o111 / d73   numeric (leftmost = LSB)
 *   [0,3,6]:8                onset array with optional :N step count
 *   {10010}<expr>            accent layer wrapping any of the above
 * Returns { steps, accents, label, ok, error }.
 */
export function parseUPI(input, ctx = { n: 16 }) {
  let src = String(input || "").trim();
  let accents = null;

  // accent prefix {…}
  const accM = src.match(/^\{([^}]*)\}\s*(.*)$/);
  if (accM) {
    src = accM[2].trim();
    accents = accM[1].replace(/[^01]/g, "").split("").map(Number);
  }

  const out = (steps, label) => {
    const n = steps.length;
    let acc = new Array(n).fill(0);
    // Accents cycle over ONSETS, not steps: the k-th onset gets accents[k % len].
    // (Across playback cycles the onset counter keeps going, so accents precess —
    // that persistence is handled by the C++ engine; this shows the first cycle.)
    if (accents && accents.length) {
      let onset = 0;
      for (let i = 0; i < n; i++)
        if (steps[i]) { acc[i] = accents[onset % accents.length] ? 1 : 0; onset++; }
    }
    // accentPattern is the raw {…} layer; the UI re-applies it with a live offset
    // so the displayed accents precess across cycles in step with playback.
    return { steps: steps.map(Number), accents: acc, accentPattern: accents, label, ok: true };
  };

  try {
    let m;
    if ((m = src.match(/^E\(\s*(\d+)\s*,\s*(\d+)\s*(?:,\s*(-?\d+)\s*)?\)$/i))) {
      const k = +m[1], n = +m[2], rot = m[3] ? +m[3] : 0;
      return out(euclideanRhythm(k, n, rot), `E(${k},${n}${rot ? "," + rot : ""})`);
    }
    if ((m = src.match(/^P\(\s*(\d+)\s*,\s*(-?\d+)\s*(?:,\s*(\d+)\s*)?\)$/i))) {
      const k = +m[1], off = +m[2], n = m[3] ? +m[3] : ctx.n;
      return out(polygon(k, ((off % n) + n) % n, n), `P(${k},${off}${m[3] ? "," + n : ""})`);
    }
    if ((m = src.match(/^R\(\s*(\d+)\s*,\s*(\d+)\s*\)$/i))) {
      const k = +m[1], n = +m[2];
      return out(randomPattern(k, n), `R(${k},${n})`);
    }
    if ((m = src.match(/^([BWD])\(\s*(\d+)\s*,\s*(\d+)\s*\)$/i))) {
      const tag = m[1].toUpperCase(), k = +m[2], n = +m[3];
      // D = Dilcue (anti-Euclidean): complement of E(n−k, n) — matches the C++.
      if (tag === "D") return out(complement(euclideanRhythm(n - k, n)), `D(${k},${n})`);
      // B / W = Barlow / Wolrab: start on the downbeat and concentrate to k onsets
      // by (anti-)indispensability — exactly the plugin's generateBarlowTransformation.
      const base = new Array(n).fill(0); base[0] = 1;
      return out(barlowCore(base, k, { wolrabMode: tag === "W" }), `${tag}(${k},${n})`);
    }
    if ((m = src.match(/^0x([0-9a-f]+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 16);
      const width = m[2] ? +m[2] : m[1].length * 4;
      return out(bitsFromValue(v, width), `0x${m[1].toUpperCase()}`);
    }
    if ((m = src.match(/^o([0-7]+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 8);
      const width = m[2] ? +m[2] : m[1].length * 3;
      return out(bitsFromValue(v, width), `o${m[1]}`);
    }
    if ((m = src.match(/^d(\d+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 10);
      const width = m[2] ? +m[2] : Math.max(1, v.toString(2).length);
      return out(bitsFromValue(v, width), `d${m[1]}`);
    }
    if ((m = src.match(/^\[([\d,\s]*)\](?::(\d+))?$/))) {
      const idx = m[1].split(",").map((s) => s.trim()).filter((s) => s !== "").map(Number);
      const n = m[2] ? +m[2] : (idx.length ? Math.max(...idx) + 1 : ctx.n);
      const steps = new Array(n).fill(0);
      idx.forEach((i) => { if (i >= 0 && i < n) steps[i] = 1; });
      return out(steps, `[${idx.join(",")}]:${n}`);
    }
    if ((m = src.match(/^b?([01]+)$/i)) && /[01]/.test(src)) {
      const bits = m[1].split("").map(Number);
      return out(bits, bits.join(""));
    }
  } catch (e) {
    return { steps: euclid(5, ctx.n), accents: new Array(ctx.n).fill(0), label: "E(5,16)", ok: false, error: String(e) };
  }
  return { steps: euclid(5, ctx.n), accents: new Array(ctx.n).fill(0), label: "", ok: false, error: "Unrecognised pattern" };
}

/* ── Barlow indispensability (canonical — for the analysis display) ─── */
/** Per-step indispensability (0..1, 1 = downbeat) — Clarence Barlow's method. */
export function indispensabilityWeights(n) { return barlowIndispensabilityTable(n); }

/* ── Barlow-family transforms (canonical) ───────────────────────────── */
/** Add/remove onsets to reach targetK by Barlow indispensability; anti = Wolrab. */
export function barlowTransform(steps, targetK, anti = false) {
  return barlowCore(steps, targetK, { wolrabMode: anti });
}

export function onsetCount(steps) { return steps.reduce((a, s) => a + s, 0); }
