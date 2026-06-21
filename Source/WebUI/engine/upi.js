/**
 * Serpe — UPI engine. Parses the Universal Pattern Input notation and
 * provides the generators + transforms. Framework-agnostic, no DOM.
 *
 * A pattern is { steps:Uint8Array(n) of 0/1, accents:Uint8Array(n) of 0/1 }.
 * All numeric notation is STRICT MSB-FIRST (first step = leftmost = most
 * significant bit), per music-suite CONVENTIONS.md.
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

/* ── generators ────────────────────────────────────────────────────── */

/** Bjorklund's algorithm — k onsets distributed as evenly as possible in n. */
export function euclid(k, n) {
  k = Math.max(0, Math.min(k, n));
  if (n <= 0) return [];
  if (k === 0) return new Array(n).fill(0);
  let a = Array.from({ length: k }, () => [1]);
  let b = Array.from({ length: n - k }, () => [0]);
  while (b.length > 1) {
    const m = Math.min(a.length, b.length);
    const na = [], nb = [];
    for (let i = 0; i < m; i++) na.push(a[i].concat(b[i]));
    if (a.length > m) for (let i = m; i < a.length; i++) nb.push(a[i]);
    else for (let i = m; i < b.length; i++) nb.push(b[i]);
    a = na; b = nb;
    if (a.length <= 1) break;
  }
  return a.concat(b).flat();
}

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

/* ── numeric notation (MSB-first) ──────────────────────────────────── */
function bitsFromBigEndian(value, width) {
  const steps = new Array(width).fill(0);
  for (let i = 0; i < width; i++) {
    steps[i] = (value >> (width - 1 - i)) & 1;
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
 *   0x92 / b10010010 / 10010010 / o222 / d146   numeric (MSB-first)
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
      return out(rotate(euclid(k, n), rot), `E(${k},${n}${rot ? "," + rot : ""})`);
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
      const base = new Array(n).fill(0);
      if (tag === "D") return out(complement(euclid(n - k, n)), `D(${k},${n})`);
      // B / W: place k onsets by indispensability (B = strongest, W = weakest)
      const order = indispensabilityOrder(n);
      const pick = tag === "B" ? order : order.slice().reverse();
      for (let i = 0; i < Math.min(k, n); i++) base[pick[i]] = 1;
      return out(base, `${tag}(${k},${n})`);
    }
    if ((m = src.match(/^0x([0-9a-f]+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 16);
      const width = m[2] ? +m[2] : m[1].length * 4;
      return out(bitsFromBigEndian(v, width), `0x${m[1].toUpperCase()}`);
    }
    if ((m = src.match(/^o([0-7]+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 8);
      const width = m[2] ? +m[2] : m[1].length * 3;
      return out(bitsFromBigEndian(v, width), `o${m[1]}`);
    }
    if ((m = src.match(/^d(\d+)(?::(\d+))?$/i))) {
      const v = parseInt(m[1], 10);
      const width = m[2] ? +m[2] : Math.max(1, v.toString(2).length);
      return out(bitsFromBigEndian(v, width), `d${m[1]}`);
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

/* ── indispensability-based ordering (used by B/W + transforms) ─────── */
/** Shallowest metric subdivision depth per step (0 = downbeat, strongest). */
export function metricDepth(n) {
  const depth = new Array(n).fill(Infinity);
  const smallestFactor = (x) => { for (let d = 2; d <= x; d++) if (x % d === 0) return d; return x; };
  (function rec(start, size, d) {
    depth[start] = Math.min(depth[start], d);
    if (size === 1) return;
    const p = smallestFactor(size);
    const sub = size / p;
    for (let j = 0; j < p; j++) rec(start + j * sub, sub, d + 1);
  })(0, n, 0);
  return depth;
}

/** Step indices ordered strongest → weakest (Barlow indispensability). */
export function indispensabilityOrder(n) {
  const depth = metricDepth(n);
  return [...Array(n).keys()].sort((a, b) => depth[a] - depth[b] || a - b);
}

/** Normalised indispensability weight per step (0..1, 1 = downbeat). */
export function indispensabilityWeights(n) {
  const depth = metricDepth(n);
  const max = Math.max(...depth);
  return depth.map((d) => (max === 0 ? 1 : 1 - d / max));
}

/* ── Barlow-family transforms ──────────────────────────────────────── */
/** Add/remove onsets by indispensability. dir>0 concentrate, dir<0 dilute. */
export function barlowTransform(steps, targetK, anti = false) {
  const n = steps.length;
  const order = indispensabilityOrder(n); // strong→weak
  const ranked = anti ? order.slice().reverse() : order;
  const cur = steps.reduce((a, s) => a + s, 0);
  const next = steps.slice();
  if (targetK > cur) {
    // add: fill the strongest empty steps (weakest if anti)
    let need = targetK - cur;
    for (const i of ranked) { if (need <= 0) break; if (!next[i]) { next[i] = 1; need--; } }
  } else if (targetK < cur) {
    // remove: drop the weakest onsets (strongest if anti)
    let need = cur - targetK;
    for (const i of ranked.slice().reverse()) { if (need <= 0) break; if (next[i]) { next[i] = 0; need--; } }
  }
  return next;
}

export function onsetCount(steps) { return steps.reduce((a, s) => a + s, 0); }
