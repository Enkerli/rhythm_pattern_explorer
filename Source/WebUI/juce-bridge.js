// juce-bridge.js — wires the Serpe React UI to JUCE's WebBrowserComponent.
//
// JUCE 8 bridge:
//   C++ → JS:  backend.addEventListener("eventId", cb)
//   JS → C++:  backend.emitEvent("eventId", data)
//
// Outside JUCE (browser / web app) every juceEmit() is a no-op, so the same UI
// runs standalone. Parameters are relayed in their ACTUAL domain; the C++ side
// normalises (convertTo0to1), so this file needs no parameter-range knowledge.

export function juceAvailable() {
  return typeof window !== 'undefined' &&
         typeof window.__JUCE__ !== 'undefined' && !!window.__JUCE__.backend;
}

function juceEmit(eventId, data) {
  if (juceAvailable()) window.__JUCE__.backend.emitEvent(eventId, data);
}

function juceOn(eventId, cb) {
  if (juceAvailable()) window.__JUCE__.backend.addEventListener(eventId, cb);
}

// Mirror console to C++ stderr (the only way to see JS logs in WKWebView).
if (typeof window !== 'undefined' && !window.__serpeLogPatched && juceAvailable()) {
  window.__serpeLogPatched = true;
  for (const level of ['log', 'warn', 'error']) {
    const orig = console[level].bind(console);
    console[level] = (...args) => {
      orig(...args);
      try {
        const msg = args.map(a => typeof a === 'object' ? JSON.stringify(a) : String(a)).join(' ');
        juceEmit('log', { level, msg });
      } catch {}
    };
  }
}

// ── Serpe APVTS params the UI drives (actual-domain) ──────────────────────────
// reactField → paramId. Values flow both ways in actual units; the snapshot from
// C++ carries the same fields. Params the plugin doesn't have yet (swing, MIDI
// channel) are UI-local until the C++ side adds them (feature-pairing pass).
export const PARAM_MAP = [
  ['tempo',        'bpm'],
  ['accentVel',    'accentVelocity'],
  ['unaccentVel',  'unaccentedVelocity'],
  ['accentPitch',  'accentPitchOffset'],
  ['midiNote',     'midiNote'],
  ['subdivision',  'subdivision'],
  ['hostTransport','useHostTransport'],
];

// ── Initialisers / senders ────────────────────────────────────────────────────

export function initJuceBridge(onEvent) {
  juceOn('stateSnapshot', snap => onEvent({ type: 'stateSnapshot', snap }));
  juceOn('paramChange',   ({ id, value }) => onEvent({ type: 'paramChange', id, value }));
  juceOn('transport',     t => onEvent({ type: 'transport', ...t }));
  juceEmit('uiReady', {});
}

/** Send a parameter change in its actual domain; C++ normalises and notifies host. */
export function sendParamActual(field, value) {
  const entry = PARAM_MAP.find(([f]) => f === field);
  const id = entry ? entry[1] : field;
  juceEmit('setParamActual', { id, value });
}

/** Send the current pattern as UPI text — C++ parses it with the authoritative
 *  UPIParser (the engine of record for playback/MIDI). Accept the full string
 *  including any {accent} prefix. Raw transforms pass their binary string, which
 *  UPIParser also parses. */
export function sendUPI(text) {
  juceEmit('setUPI', { text: text || '' });
}
