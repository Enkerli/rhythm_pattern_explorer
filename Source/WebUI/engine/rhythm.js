/**
 * Serpe — canonical rhythm algorithms, ported verbatim from @enkerli/theory
 * (music-suite packages/theory/src/rhythm.ts), which is the suite's reference
 * implementation and is conformance-locked against the original Serpe engine.
 *
 * Replaces the prototype's simplified metric-depth approximation so the WebUI
 * preview and the Dilute/Concentrate/Wolrab/Dilcue transforms match the C++
 * plugin exactly. Patterns are 0/1 number arrays (first step = leftmost).
 */

// ── Euclidean (Bjorklund, rotated so the first onset is at index 0) ──────────
export function euclideanRhythm(beats, steps, offset = 0) {
  if (beats > steps) beats = steps;
  if (beats <= 0) return new Array(steps).fill(0);

  let pattern = [];
  const counts = [];
  const remainders = [];
  let divisor = steps - beats;
  remainders[0] = beats;
  let level = 0;

  do {
    counts[level] = Math.floor(divisor / remainders[level]);
    remainders[level + 1] = divisor % remainders[level];
    divisor = remainders[level];
    level++;
  } while (remainders[level] > 1);

  counts[level] = divisor;

  function build(l) {
    if (l === -1) pattern.push(0);
    else if (l === -2) pattern.push(1);
    else {
      for (let i = 0; i < counts[l]; i++) build(l - 1);
      if (remainders[l] !== 0) build(l - 2);
    }
  }
  build(level);

  while (pattern.length < steps) pattern.push(0);

  const firstBeatIndex = pattern.findIndex((b) => b);
  if (firstBeatIndex > 0) pattern = pattern.slice(firstBeatIndex).concat(pattern.slice(0, firstBeatIndex));

  if (offset !== 0) {
    offset = ((offset % steps) + steps) % steps;
    const out = new Array(steps);
    for (let i = 0; i < steps; i++) out[i] = pattern[(i - offset + steps) % steps];
    pattern = out;
  }
  return pattern;
}

export function euclideanComplement(beats, steps, offset = 0) {
  return euclideanRhythm(steps - beats, steps, offset);
}

// ── Barlow indispensability (Clarence Barlow, stratified meter) ──────────────
function gcd(a, b) { while (b !== 0) { const t = b; b = a % b; a = t; } return a; }
function primeFactors(n) {
  const f = []; let d = 2;
  while (n > 1) { while (n % d === 0) { f.push(d); n /= d; } d++; }
  return f;
}
function stratifiedMeter(length) { return primeFactors(length).sort((a, b) => a - b); }

export function positionIndispensability(position, length) {
  if (position === 0) return 1;
  if (position === length - 1) return 0.75; // anacrustic pickup

  const strat = stratifiedMeter(length);
  let total = 0;
  for (let level = 0; level < strat.length; level++) {
    const prime = strat[level];
    const levelSize = length / Math.pow(prime, level + 1);
    if (levelSize > 0 && position % levelSize === 0) total += 1 / Math.pow(prime, level + 1);
  }
  if (total === 0) {
    const g = gcd(position, length);
    const level = length / g;
    total = 1;
    for (const p of primeFactors(level)) total *= 1 / p;
  }
  return Math.min(total, 1);
}

export function barlowIndispensabilityTable(length) {
  const table = new Array(length);
  for (let i = 0; i < length; i++) table[i] = positionIndispensability(i, length);
  return table;
}

function isWeakBeat(position, stepCount) {
  const quarter = stepCount / 4, eighth = stepCount / 8;
  return !(position % quarter === 0 || position % eighth === 0);
}

// ── Barlow transform: dilute / concentrate, with Wolrab (anti) mode ──────────
export function barlowTransform(pattern, targetOnsets, options = {}) {
  const stepCount = pattern.length;
  const current = pattern.filter((s) => s).length;
  if (targetOnsets === current) return pattern.slice();

  const table = barlowIndispensabilityTable(stepCount);
  return targetOnsets < current
    ? dilute(pattern, targetOnsets, table, options)
    : concentrate(pattern, targetOnsets, table, options);
}

function dilute(pattern, targetOnsets, table, options) {
  const { preserveDownbeat = true, minimumIndispensability = 0.0, wolrabMode = false } = options;
  const current = pattern.filter((s) => s).length;
  const toRemove = current - targetOnsets;

  const onsets = pattern
    .map((on, i) => ({ position: i, indispensability: table[i], isDownbeat: i === 0, on }))
    .filter((p) => p.on);
  onsets.sort((a, b) => {
    if (preserveDownbeat && !wolrabMode) {
      if (a.isDownbeat && !b.isDownbeat) return 1;
      if (!a.isDownbeat && b.isDownbeat) return -1;
    }
    return wolrabMode ? b.indispensability - a.indispensability : a.indispensability - b.indispensability;
  });

  const next = pattern.slice();
  for (let i = 0; i < Math.min(toRemove, onsets.length); i++) {
    const c = onsets[i];
    if (wolrabMode || c.indispensability >= minimumIndispensability || !preserveDownbeat || !c.isDownbeat)
      next[c.position] = 0;
  }
  return next;
}

function concentrate(pattern, targetOnsets, table, options) {
  const { avoidWeakBeats = false, minimumIndispensability = 0.1, wolrabMode = false } = options;
  const stepCount = pattern.length;
  const current = pattern.filter((s) => s).length;
  const toAdd = targetOnsets - current;

  const empty = pattern
    .map((on, i) => ({ position: i, indispensability: table[i], isWeakBeat: isWeakBeat(i, stepCount), on }))
    .filter((p) => !p.on);
  empty.sort((a, b) => {
    if (avoidWeakBeats) {
      if (a.isWeakBeat && !b.isWeakBeat) return 1;
      if (!a.isWeakBeat && b.isWeakBeat) return -1;
    }
    return wolrabMode ? a.indispensability - b.indispensability : b.indispensability - a.indispensability;
  });

  const next = pattern.slice();
  let added = 0;
  for (let i = 0; i < empty.length && added < toAdd; i++) {
    const c = empty[i];
    if (c.indispensability >= minimumIndispensability) { next[c.position] = 1; added++; }
  }
  if (added < toAdd) {
    for (let i = 0; i < empty.length && added < toAdd; i++) {
      const c = empty[i];
      if (next[c.position]) continue;
      next[c.position] = 1; added++;
    }
  }
  return next;
}
