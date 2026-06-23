/**
 * Serpe — pattern analysis. Pure functions over a steps array (0/1).
 * Perfect balance, center of gravity, Euclidean evenness, interval
 * structure, and the readouts the panel shows.
 */

const TAU = Math.PI * 2;

export function onsetIndices(steps) {
  const out = [];
  for (let i = 0; i < steps.length; i++) if (steps[i]) out.push(i);
  return out;
}

/** Resultant vector of unit vectors at each onset angle. */
export function resultant(steps) {
  const n = steps.length;
  let x = 0, y = 0;
  const on = onsetIndices(steps);
  for (const i of on) {
    const a = (TAU * i) / n;
    x += Math.cos(a);
    y += Math.sin(a);
  }
  const mag = Math.hypot(x, y);
  return { x, y, mag, angle: Math.atan2(y, x), k: on.length };
}

/**
 * Center of gravity: normalised resultant magnitude (0 = perfectly
 * balanced, 1 = all onsets stacked) and its angle in steps.
 */
export function centerOfGravity(steps) {
  const r = resultant(steps);
  const norm = r.k ? r.mag / r.k : 0;
  const angleSteps = ((r.angle / TAU) * steps.length + steps.length) % steps.length;
  return { magnitude: norm, angleSteps, x: r.x, y: r.y };
}

/** Perfect balance: resultant ≈ 0 (within tolerance) with ≥2 onsets. */
export function perfectBalance(steps, tol = 1e-6) {
  const r = resultant(steps);
  return r.k >= 2 && r.mag < tol * Math.max(1, r.k) + 1e-9 ? true : r.mag < 1e-6;
}

/** Inter-onset intervals around the cycle (wraps). */
export function intervals(steps) {
  const on = onsetIndices(steps);
  const n = steps.length;
  if (on.length < 2) return on.length === 1 ? [n] : [];
  const iv = [];
  for (let i = 0; i < on.length; i++) {
    const a = on[i];
    const b = on[(i + 1) % on.length];
    iv.push(((b - a + n) % n) || n);
  }
  return iv;
}

/**
 * Euclidean evenness: 1 when intervals are as equal as possible (the
 * Euclidean rhythm), lower as they clump. Based on interval spread.
 */
export function evenness(steps) {
  const iv = intervals(steps);
  if (iv.length < 2) return 1;
  const mean = iv.reduce((a, b) => a + b, 0) / iv.length;
  if (mean === 0) return 1;
  // mean absolute deviation, normalised; Euclidean minimises this
  const mad = iv.reduce((a, b) => a + Math.abs(b - mean), 0) / iv.length;
  return Math.max(0, 1 - mad / mean);
}

/** A compact readout bundle for the analysis panel. */
export function analyse(steps) {
  const n = steps.length;
  const on = onsetIndices(steps);
  const cog = centerOfGravity(steps);
  return {
    n,
    k: on.length,
    density: n ? on.length / n : 0,
    onsets: on,
    intervals: intervals(steps),
    evenness: evenness(steps),
    balanced: perfectBalance(steps),
    cog,
    /** binary + hex + decimal renderings. Leftmost = LSB (step k = bit k): the
     *  value equals the ordinary numeral of the reversed step string. Decimal is
     *  that value; hex digits are then written little-endian (first step's nibble
     *  leftmost), so tresillo 10010010 = 0x94, not the value's 0x49. */
    binary: steps.join(""),
    hex: "0x" + [...BigInt("0b" + ((steps.slice().reverse().join("")) || "0")).toString(16).toUpperCase()].reverse().join(""),
    decimal: Number(BigInt("0b" + ((steps.slice().reverse().join("")) || "0"))),
  };
}
