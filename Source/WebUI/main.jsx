// main.jsx — Serpe React app. Recreates the suite redesign; reuses the
// framework-agnostic engine (engine/*.js) and the suite CSS (styles/*.css).
// Runs standalone in the browser and inside the JUCE WebView (bridge no-ops
// when JUCE is absent).

// CSS is imported as text (esbuild --loader:.css=text) and injected at runtime,
// so the build produces a single bundle.js (no separate bundle.css). This keeps
// the JUCE binary-data step to two artifacts and avoids an Xcode build cycle.
import tokensCss from './styles/tokens.css';
import componentsCss from './styles/components.css';
import serpeCss from './styles/serpe.css';
{
  const el = document.createElement('style');
  el.textContent = [tokensCss, componentsCss, serpeCss].join('\n');
  document.head.appendChild(el);
}

import { parseUPI, euclid, polygon, rotate, invert, complement,
         barlowTransform, indispensabilityWeights, onsetCount } from './engine/upi.js';
import { analyse } from './engine/analysis.js';
import { createCircleView, createStepView } from './engine/render.js';
import { initJuceBridge, sendParamActual, sendUPI, sendPlaying, sendBPM, juceAvailable } from './juce-bridge.js';

// Inline SVG mark — a data-URL <img> with unescaped '#' hex colours renders in
// Chrome but breaks in macOS WKWebView, so inject the markup directly.
const ICON_SVG = `<svg viewBox="0 0 1024 1024" xmlns="http://www.w3.org/2000/svg" aria-label="Serpe">
<rect x="16" y="16" width="992" height="992" rx="208" fill="#f5f2eb"/>
<rect x="16" y="16" width="992" height="992" rx="208" fill="none" stroke="#ddd6ca" stroke-width="12"/>
<circle cx="512" cy="512" r="300" fill="none" stroke="#ddd6ca" stroke-width="14"/>
<polygon points="512,212 724,512 618,724 406,724 300,512" fill="#2d9d8a" fill-opacity="0.14" stroke="#2d9d8a" stroke-width="26" stroke-linejoin="round"/>
<circle cx="512" cy="212" r="42" fill="#2d9d8a" stroke="#2d2b27" stroke-width="14"/>
<circle cx="724" cy="300" r="24" fill="#fcfbf7" stroke="#2d2b27" stroke-width="12"/>
<circle cx="724" cy="512" r="42" fill="#2d9d8a" stroke="#2d2b27" stroke-width="14"/>
<circle cx="618" cy="724" r="42" fill="#2d9d8a" stroke="#2d2b27" stroke-width="14"/>
<circle cx="406" cy="724" r="42" fill="#2d9d8a" stroke="#2d2b27" stroke-width="14"/>
<circle cx="300" cy="512" r="42" fill="#2d9d8a" stroke="#2d2b27" stroke-width="14"/>
<circle cx="300" cy="300" r="24" fill="#fcfbf7" stroke="#2d2b27" stroke-width="12"/>
<circle cx="512" cy="512" r="20" fill="#2d2b27"/></svg>`;

const { useState, useRef, useEffect, useMemo, createElement: h } = React;

const LS = {
  get: (k, d) => { try { const v = localStorage.getItem('serpe.' + k); return v == null ? d : v; } catch { return d; } },
  set: (k, v) => { try { localStorage.setItem('serpe.' + k, v); } catch {} },
};

// Runtime config — feature gating + density. Real deployment detects the
// runtime (JUCE present ⇒ plugin; C++ may refine to ipad-p/ipad-l).
const RT = {
  webapp:  { web: true,  host: false, dense: false },
  plugin:  { web: false, host: true,  dense: false },
  'ipad-p':{ web: false, host: true,  dense: true  },
  'ipad-l':{ web: false, host: true,  dense: true  },
};

// Subdivision param (how long each step is, relative to the host beat) — order
// matches the C++ AudioParameterChoice "subdivision".
const SUBDIV = ['64th Triplet', '64th', '32nd Triplet', '32nd', '16th Triplet', '16th',
  '8th Triplet', '8th', 'Quarter Triplet', 'Quarter', 'Half Triplet', 'Half', 'Whole'];

// Apply a raw accent pattern to a step array's onsets, offset by `off` onsets
// (the precession). Onset k is accented when pattern[(k + off) % len] is set.
function applyAccents(steps, pattern, off = 0) {
  const acc = new Array(steps.length).fill(0);
  if (pattern && pattern.length) {
    let onset = 0;
    for (let i = 0; i < steps.length; i++)
      if (steps[i]) { acc[i] = pattern[(onset + off) % pattern.length] ? 1 : 0; onset++; }
  }
  return acc;
}

// An imperative SVG view (render.js) wrapped as a React component.
function EngineView({ create, opts, data }) {
  const host = useRef(null), view = useRef(null);
  useEffect(() => { view.current = create(host.current, opts || {}); }, []);
  useEffect(() => { if (view.current) view.current.update(data); });
  return h('div', { ref: host });
}

function SerpeApp() {
  const [steps, setSteps]     = useState(() => euclid(5, 8));
  // Accents are derived: the raw {…} pattern re-applied to the onsets with a
  // live offset, so they precess across playback cycles (offset from the C++
  // engine in the plugin, tracked locally in the webapp).
  const [accentPattern, setAccentPattern] = useState(null);
  const [accentOffset, setAccentOffset] = useState(0);
  const [label, setLabel]     = useState('E(5,8)');
  const [upiText, setUpiText] = useState(LS.get('upi', 'E(5,8)'));
  const [accText, setAccText] = useState('');
  const [parseErr, setParseErr] = useState(null);

  const [playing, setPlaying] = useState(false);
  const [playhead, setPlayhead] = useState(-1);
  const [tempo, setTempo]     = useState(+LS.get('tempo', 120));
  const [group, setGroup]     = useState(4);
  const [swing, setSwing]     = useState(0);
  const [subdiv, setSubdiv]   = useState(5);   // subdivision param index (5 = 16th)
  const [view, setView]       = useState('both');
  const [showLabels, setShowLabels] = useState(false);

  const [theme, setTheme]     = useState(LS.get('theme', 'light'));
  const [runtime, setRuntime] = useState(juceAvailable() ? 'plugin' : 'webapp');
  const cfg = RT[runtime];
  const [dense, setDense]     = useState(cfg.dense);

  const [genType, setGenType] = useState('E');
  const [genK, setGenK] = useState(5), [genN, setGenN] = useState(8), [genRot, setGenRot] = useState(0);

  const [accVel, setAccVel] = useState(112);
  const [unaccVel, setUnaccVel] = useState(72);
  const [accPitch, setAccPitch] = useState(0);
  const [midiNote, setMidiNote] = useState(38);
  const [midiChan, setMidiChan] = useState(1);

  const [progOff, setProgOff] = useState(1);
  const [progLeng, setProgLeng] = useState(false);
  const [cycle, setCycle] = useState(0);
  const baseRef = useRef(null);

  const [scenes, setScenes] = useState(() => new Array(8).fill(null));
  const [activeScene, setActiveScene] = useState(-1);

  const [dbQuery, setDbQuery] = useState('');
  const [waOn, setWaOn] = useState(true), [waVol, setWaVol] = useState(0.7);
  const [hostSync, setHostSync] = useState(false);
  const [hostInfo, setHostInfo] = useState(null);  // { bpm, playing } from C++

  const a = useMemo(() => analyse(steps), [steps]);
  const accents = useMemo(() => applyAccents(steps, accentPattern, accentOffset), [steps, accentPattern, accentOffset]);

  // live mirror for the audio loop (avoids stale closures)
  const live = useRef({});
  live.current = { steps, accents, accentPattern, tempo, group, swing, waOn, waVol };

  // ── apply theme / runtime / density to the document ──
  useEffect(() => { document.documentElement.setAttribute('data-theme', theme); LS.set('theme', theme); }, [theme]);
  useEffect(() => { document.documentElement.setAttribute('data-runtime', runtime); }, [runtime]);

  // ── core: set a pattern from parsed UPI or generator ──
  function applyPattern(p, { syncField = true } = {}) {
    setSteps(p.steps); setAccentPattern(p.accentPattern); setAccentOffset(0); setLabel(p.label);
    baseRef.current = null; setCycle(0);
    if (syncField && p.label) setUpiText(p.label);
  }

  function parseField(text = upiText, acc = accText) {
    // The accent layer lives in the Accents field (acc); prepend it unless the
    // pattern text already carries its own {…} prefix (don't double up).
    const hasInline = /^\s*\{/.test(text);
    const full = (!hasInline && acc.trim()) ? `{${acc.trim()}}${text}` : text;
    const p = parseUPI(full, { n: steps.length || 16 });
    if (p.ok) { setParseErr(null); applyPattern(p, { syncField: false }); LS.set('upi', text);
      if (juceAvailable()) sendUPI(full); }
    else setParseErr(p.error || 'unrecognised');
  }

  // parse whenever the text/accents change (debounced-ish via React batching)
  useEffect(() => { parseField(); /* eslint-disable-next-line */ }, [upiText, accText]);

  // The accent layer to carry onto a new pattern. If it lives in the Accents
  // field, parseField prepends it (return ''); if it was typed inline in the UPI
  // field, re-attach it as a {…} prefix so it survives the change either way.
  function accentPrefix() {
    if (accText.trim()) return '';
    return (accentPattern && accentPattern.length) ? `{${accentPattern.join('')}}` : '';
  }

  // ── generators ──
  function generate() {
    let lbl;
    if (genType === 'E') lbl = `E(${genK},${genN}${genRot ? ',' + genRot : ''})`;
    else if (genType === 'P') lbl = `P(${genK},${genRot},${genN})`;
    else if (genType === 'R') lbl = `R(${genK},${genN})`;
    else lbl = `${genType}(${genK},${genN})`;
    setUpiText(accentPrefix() + lbl);   // keep the accent layer across pattern changes
  }

  // ── transforms ──
  function applyTransform(fn) {
    // Re-attach the accent layer to the new pattern; parseField applies it and
    // sends the UPI — so accents survive transforms (inline or from the field).
    setUpiText(accentPrefix() + analyse(fn(steps.slice())).binary);
  }
  const TX = {
    dilute:  s => barlowTransform(s, Math.max(1, onsetCount(s) - 1)),
    conc:    s => barlowTransform(s, Math.min(s.length, onsetCount(s) + 1)),
    wolrab:  s => barlowTransform(s, Math.max(1, onsetCount(s) - 1), true),
    dilcue:  s => complement(euclid(onsetCount(s), s.length)),
    rotl:    s => rotate(s, -1),
    rotr:    s => rotate(s, 1),
    invert,
    comp:    complement,
  };

  // ── progressive ──
  function progAdvance() {
    if (!baseRef.current) baseRef.current = { steps: steps.slice(), accentPattern };
    const c = cycle + 1; setCycle(c);
    let next = rotate(baseRef.current.steps, progOff * c);
    if (progLeng) next = euclid(onsetCount(baseRef.current.steps), baseRef.current.steps.length + c);
    setSteps(next); setAccentPattern(null); setAccentOffset(0);
    setUpiText(analyse(next).binary);
  }
  function progReset() {
    setCycle(0);
    if (baseRef.current) { setSteps(baseRef.current.steps); setAccentPattern(baseRef.current.accentPattern); setAccentOffset(0); }
    baseRef.current = null;
    setUpiText(analyse(baseRef.current ? baseRef.current.steps : steps).binary);
  }

  // ── scenes ──
  function sceneClick(i) {
    setScenes(prev => {
      const next = prev.slice();
      if (next[i]) {
        const sc = next[i];
        setSteps(sc.steps.slice()); setAccentPattern(sc.accentPattern); setAccentOffset(0); setLabel(sc.label); setUpiText(sc.label);
        setActiveScene(i);
      } else {
        next[i] = { steps: steps.slice(), accentPattern, label: label || analyse(steps).binary };
        setActiveScene(i);
      }
      return next;
    });
  }
  function sceneClear(i) { setScenes(prev => { const n = prev.slice(); n[i] = null; return n; }); if (activeScene === i) setActiveScene(-1); }

  // ── transport (Web Audio in standalone; host drives the plugin) ──
  const audioCtx = useRef(null), timer = useRef(null);
  function click(accent) {
    const L = live.current;
    if (!cfg.web || !L.waOn || !audioCtx.current) return;  // Web Audio is webapp-only
    const t = audioCtx.current.currentTime;
    const o = audioCtx.current.createOscillator(), g = audioCtx.current.createGain();
    o.frequency.value = accent ? 1320 : 880; g.gain.value = 0.0001;
    o.connect(g); g.connect(audioCtx.current.destination);
    const v = L.waVol * (accent ? 1 : 0.55);
    g.gain.setValueAtTime(0.0001, t);
    g.gain.exponentialRampToValueAtTime(Math.max(0.0002, v), t + 0.005);
    g.gain.exponentialRampToValueAtTime(0.0001, t + 0.09);
    o.start(t); o.stop(t + 0.1);
  }
  function stepDur(idx) {
    const L = live.current; const grp = L.group || 4;
    const base = (60 / L.tempo) / grp; const s = L.swing / 100 * 0.5;
    if (s <= 0) return base * 1000;
    return base * (idx % 2 === 0 ? 1 + s : 1 - s) * 1000;
  }
  function tick() {
    setPlayhead(ph => {
      const L = live.current; const n = L.steps.length || 1;
      const next = (ph + 1) % n;
      // at the cycle boundary, advance the accent phase by this cycle's onset
      // count so the displayed accents precess like the engine's onset counter
      if (next === 0 && L.accentPattern && L.accentPattern.length) {
        const k = L.steps.reduce((acc, s) => acc + (s ? 1 : 0), 0);
        setAccentOffset(o => (o + k) % L.accentPattern.length);
      }
      if (L.steps[next]) click(!!L.accents[next]);
      timer.current = setTimeout(tick, stepDur(next));
      return next;
    });
  }
  function play() {
    if (cfg.host) {                       // plugin: drive the C++ internal sequencer
      const next = !playing; setPlaying(next); sendPlaying(next);
      return;
    }
    if (playing) { pause(); return; }     // webapp: Web Audio transport
    if (!audioCtx.current) { try { audioCtx.current = new (window.AudioContext || window.webkitAudioContext)(); } catch {} }
    if (audioCtx.current && audioCtx.current.state === 'suspended') audioCtx.current.resume();
    setPlaying(true); timer.current = setTimeout(tick, stepDur(0));
  }
  function pause() { setPlaying(false); clearTimeout(timer.current); }
  function stop() {
    if (cfg.host) { setPlaying(false); sendPlaying(false); return; }  // plugin
    pause(); setPlayhead(-1);
  }
  useEffect(() => () => clearTimeout(timer.current), []);

  // ── JUCE bridge ──
  useEffect(() => {
    initJuceBridge(ev => {
      if (ev.type === 'stateSnapshot') {
        const s = ev.snap || {};
        if (s.runtime) setRuntime(s.runtime);
        if (s.bpm != null && s.bpm >= 20) setTempo(Math.round(s.bpm));  // never 0
        if (s.accentVelocity != null) setAccVel(Math.round(s.accentVelocity));
        if (s.unaccentedVelocity != null) setUnaccVel(Math.round(s.unaccentedVelocity));
        if (s.accentPitchOffset != null) setAccPitch(s.accentPitchOffset);
        if (s.midiNote != null) setMidiNote(s.midiNote);
        if (s.useHostTransport != null) setHostSync(!!s.useHostTransport);
        if (s.subdivision != null) setSubdiv(s.subdivision);
        if (s.internalPlaying != null) setPlaying(!!s.internalPlaying);
        if (typeof s.upi === 'string' && s.upi) setUpiText(s.upi);
      } else if (ev.type === 'transport') {
        // In the plugin the C++ sequencer owns the playhead and effective tempo.
        setHostInfo({ bpm: ev.bpm, playing: ev.playing, hostSync: ev.hostSync });
        if (typeof ev.step === 'number') setPlayhead(ev.step);
        if (typeof ev.accentOffset === 'number') setAccentOffset(ev.accentOffset);
        if (ev.bpm >= 20) setTempo(ev.bpm);
        setHostSync(!!ev.hostSync);
        setPlaying(!!ev.playing);
      }
    });
  }, []);

  // ── derived UI bits ──
  const weights = useMemo(() => indispensabilityWeights(steps.length || 1), [steps.length]);
  const intervalsStr = a.intervals.join(' ');

  const chips = [
    ['E(5,8)', 'tresillo+'], ['E(3,8)', 'tresillo'], ['E(7,16)', '16-step'],
    ['P(3,0)', 'triangle'], ['0x92', 'hex'], ['[0,3,6,9]:12', 'array'], ['{10010}E(5,8)', 'accented'],
  ];
  function applyChip(v) {
    const m = v.match(/^\{([^}]*)\}(.*)$/);
    if (m) { setAccText(m[1]); setUpiText(m[2]); } else { setAccText(''); setUpiText(v); }
  }

  const dbLib = [
    ['E(5,8)', 'tresillo +1', 5, 8], ['E(3,8)', 'tresillo', 3, 8], ['E(7,16)', 'samba-ish', 7, 16],
    ['E(4,9)', 'balanced', 4, 9], ['P(3,0)', 'triangle', 3, 12], ['E(5,12)', 'even five', 5, 12],
  ].filter(([l]) => l.toLowerCase().includes(dbQuery.toLowerCase()));

  const synced = cfg.host && hostSync;

  // ── render ──
  return h('div', { className: 'serpe' + (dense ? ' es-dense' : ''), id: 'serpe' },
    // top bar
    h('div', { className: 'serpe-top' },
      h('div', { className: 'title' }, h('span', { className: 'title-mark', dangerouslySetInnerHTML: { __html: ICON_SVG } }), 'Serpe'),
      h('div', { className: 'transport' },
        h('button', { className: 'tbtn play' + (playing ? ' on' : ''), onClick: play, title: 'Play / pause', 'aria-label': 'Play' },
          playing
            ? h('svg', { viewBox: '0 0 24 24', fill: 'currentColor' }, h('rect', { x: 6, y: 5, width: 4, height: 14 }), h('rect', { x: 14, y: 5, width: 4, height: 14 }))
            : h('svg', { viewBox: '0 0 24 24', fill: 'currentColor' }, h('path', { d: 'M8 5v14l11-7z' }))),
        h('button', { className: 'tbtn', onClick: stop, title: 'Stop', 'aria-label': 'Stop' },
          h('svg', { viewBox: '0 0 24 24', fill: 'currentColor' }, h('rect', { x: 6, y: 6, width: 12, height: 12, rx: 2 })))),
      h('div', { className: 'tempo', style: { opacity: synced ? 0.45 : 1 } },
        h('input', { className: 'es-control', type: 'number', min: 40, max: 240, value: tempo, disabled: synced,
          onChange: e => { const t = Math.max(40, Math.min(240, +e.target.value || 120)); setTempo(t); LS.set('tempo', t); if (juceAvailable()) sendBPM(t); }, 'aria-label': 'Tempo' }),
        h('span', { className: 'unit' }, 'BPM')),
      cfg.host && h('div', { className: 'hostchip' + (synced ? ' synced' : '') },
        h('span', { className: 'led' }), h('span', null, synced ? `Host: ${hostInfo?.bpm ?? 124} BPM` : 'Host sync off')),
      h('div', { className: 'spacer' }),
      h('button', { className: 'iconbtn', onClick: () => setDense(d => !d), title: 'Density' },
        h('span', { 'aria-hidden': true }, dense ? '▤' : '≡'), h('span', { className: 'lbl' }, dense ? 'Dense' : 'Comfortable')),
      h('button', { className: 'iconbtn', onClick: () => setTheme(t => t === 'dark' ? 'light' : 'dark'), title: 'Theme' },
        h('span', { 'aria-hidden': true }, theme === 'dark' ? '☀' : '☾'), h('span', { className: 'lbl' }, theme === 'dark' ? 'Light' : 'Dark'))),

    // body
    h('div', { className: 'serpe-body' },
      // stage column
      h('div', { className: 'serpe-stage-col' },
        h('div', { className: 'upi' },
          h('div', { className: 'upi-row' },
            h('span', { className: 'prompt' }, '›'),
            h('input', { className: 'upi-field' + (parseErr ? ' bad' : ''), type: 'text', spellCheck: false, autoComplete: 'off',
              value: upiText, onChange: e => setUpiText(e.target.value), 'aria-label': 'Universal Pattern Input' })),
          h('div', { className: 'upi-status' }, parseErr
            ? [h('span', { key: 'e', className: 'err' }, '✗ ' + parseErr), h('span', { key: 'd', className: 'dot' }), h('span', { key: 't' }, 'try E(5,8), 0x92, [0,3,6]:8, P(3,0)')]
            : [h('span', { key: 'o', className: 'ok' }, '✓ parsed'), h('span', { key: 'd1', className: 'dot' }),
               h('span', { key: 'k' }, `${a.k} onsets in ${a.n} steps`), h('span', { key: 'd2', className: 'dot' }),
               h('span', { key: 'b', className: 'es-num' }, a.binary), h('span', { key: 'd3', className: 'dot' }),
               h('span', { key: 'h', className: 'es-num' }, a.hex), ' · ', h('span', { key: 'dec', className: 'es-num' }, 'd' + a.decimal)]),
          h('div', { className: 'upi-chips' }, chips.map(([v, t]) =>
            h('button', { key: v, className: 'upi-chip', onClick: () => applyChip(v) }, h('b', null, v), ' ' + t)))),

        h('div', { className: 'viz' + (view === 'circle' ? ' solo-circle' : '') },
          h('div', { className: 'viz-head' },
            h('span', { className: 'es-eyebrow' }, 'Pattern'),
            h('label', { className: 'iconbtn', style: { height: 30, fontSize: 12, gap: 6 } },
              h('input', { type: 'checkbox', checked: showLabels, onChange: e => setShowLabels(e.target.checked) }), ' step numbers'),
            h('div', { className: 'seg', role: 'group', 'aria-label': 'View' },
              ['both', 'circle', 'step'].map(v =>
                h('button', { key: v, 'aria-pressed': view === v, onClick: () => setView(v) }, v[0].toUpperCase() + v.slice(1))))),
          h('div', { className: 'viz-body' },
            view !== 'step' && h('div', { className: 'viz-circle' },
              h(EngineView, { create: createCircleView, opts: { showCog: true }, data: { steps, accents, playhead, showLabels } })),
            h('div', { className: 'viz-side' },
              view !== 'circle' && h(EngineView, { create: createStepView, opts: { group }, data: { steps, accents, playhead, group } }),
              h('div', { className: 'readstrip' },
                h('span', null, h('span', { className: 'k' }, 'pattern '), h('b', { className: 'es-num' }, label || a.binary)),
                h('span', null, h('span', { className: 'k' }, 'onsets '), h('b', null, a.k), '/', h('b', null, a.n)),
                h('span', null, h('span', { className: 'k' }, 'intervals '), h('b', { className: 'es-num' }, intervalsStr || '—'))))))),

      // control rail
      h('div', { className: 'serpe-rail' },
        // Generators
        h(Section, { title: 'Generators', open: true },
          h(Field, { label: 'Type' },
            h('select', { className: 'es-control', value: genType, onChange: e => setGenType(e.target.value) },
              [['E', 'Euclidean — E(k,n)'], ['P', 'Polygon — P(k,off)'], ['R', 'Random — R(k,n)'],
               ['B', 'Barlow — B(k,n)'], ['W', 'Wolrab — W(k,n)'], ['D', 'Dilcue — D(k,n)']].map(([v, t]) =>
                h('option', { key: v, value: v }, t)))),
          h(Slider, { label: 'Onsets', value: genK, min: 1, max: 16, set: setGenK }),
          h(Slider, { label: 'Steps', value: genN, min: 2, max: 32, set: setGenN }),
          genType !== 'R' && h(Slider, { label: genType === 'P' ? 'Offset' : 'Rotation', value: genRot, min: 0, max: Math.max(1, genN - 1), set: setGenRot }),
          h('button', { className: 'es-btn es-primary', style: { width: '100%' }, onClick: generate }, 'Generate')),

        // Transform
        h(Section, { title: 'Transform', open: true },
          h('div', { className: 'btn-grid', style: { marginBottom: 6 } },
            [['dilute', 'Dilute −'], ['conc', 'Concentrate +'], ['wolrab', 'Wolrab'], ['dilcue', 'Dilcue'],
             ['rotl', 'Rotate ←'], ['rotr', 'Rotate →'], ['invert', 'Invert'], ['comp', 'Complement']].map(([k, t]) =>
              h('button', { key: k, className: 'es-btn es-small', onClick: () => applyTransform(TX[k]) }, t))),
          h('p', { className: 'note', style: { fontSize: 11, color: 'var(--es-fg-muted)', margin: '2px 0 0' } },
            'Barlow add/remove by metric indispensability; Wolrab inverts that weighting; Dilcue is the anti-Euclidean complement.')),

        // Progressive
        h(Section, { title: 'Progressive' },
          h(Slider, { label: 'Offset / cycle', value: progOff, min: -4, max: 4, set: setProgOff, fmt: v => (v >= 0 ? '+' : '') + v }),
          h('label', { className: 'iconbtn', style: { height: 34, width: '100%', justifyContent: 'flex-start', marginBottom: 8 } },
            h('input', { type: 'checkbox', checked: progLeng, onChange: e => setProgLeng(e.target.checked) }), ' Progressive lengthening'),
          h('div', { className: 'field-row' },
            h('button', { className: 'es-btn es-small', style: { flex: 1 }, onClick: progAdvance }, 'Advance cycle'),
            h('button', { className: 'es-btn es-small', onClick: progReset }, 'Reset')),
          h('p', { className: 'note', style: { fontSize: 11, color: 'var(--es-fg-muted)', margin: '8px 0 0' } },
            'Cycle ', h('b', { className: 'es-num' }, cycle), ' — the pattern evolves step-by-step as it loops.')),

        // Accents
        h(Section, { title: 'Accents' },
          h(Field, { label: 'Accent pattern { }' },
            h('input', { className: 'es-control', type: 'text', spellCheck: false, placeholder: '10010', value: accText, onChange: e => setAccText(e.target.value) })),
          h(Slider, { label: 'Accent velocity', value: accVel, min: 1, max: 127, set: v => { setAccVel(v); if (juceAvailable()) sendParamActual('accentVel', v); } }),
          h(Slider, { label: 'Unaccented velocity', value: unaccVel, min: 1, max: 127, set: v => { setUnaccVel(v); if (juceAvailable()) sendParamActual('unaccentVel', v); } }),
          h(Slider, { label: 'Accent pitch offset', value: accPitch, min: -12, max: 12, set: v => { setAccPitch(v); if (juceAvailable()) sendParamActual('accentPitch', v); }, fmt: v => (v >= 0 ? '+' : '') + v })),

        // Analysis
        h(Section, { title: 'Analysis', open: true },
          h('div', { style: { marginBottom: 12 } },
            h('span', { className: 'balance-flag' + (a.balanced && a.k >= 2 ? ' yes' : '') },
              h('span', { className: 'led' }), h('span', null, a.k < 2 ? 'needs ≥2 onsets' : a.balanced ? 'Perfectly balanced' : 'Not balanced'))),
          h(Meter, { label: 'Evenness', frac: a.evenness, text: Math.round(a.evenness * 100) + '%' }),
          h(Meter, { label: 'Density', frac: a.density, text: Math.round(a.density * 100) + '%' }),
          h(Meter, { label: 'CoG radius', frac: a.cog.magnitude, text: a.cog.magnitude.toFixed(2) }),
          h('p', { className: 'es-eyebrow', style: { margin: '14px 0 6px' } }, 'Barlow indispensability'),
          h('div', { className: 'indisp' }, steps.map((on, i) =>
            h('div', { key: i, className: 'b' + (on ? ' on' : ''), style: { height: (10 + weights[i] * 46) + 'px' }, title: `step ${i} · weight ${weights[i].toFixed(2)}` }))),
          steps.length <= 16 && h('div', { className: 'indisp-x' }, steps.map((_, i) => h('span', { key: i }, i)))),

        // Scenes
        h(Section, { title: 'Scenes' },
          h('div', { className: 'scenes' }, scenes.map((sc, i) => {
            let timer;
            return h('button', { key: i, className: 'scene' + (sc ? ' filled' : '') + (i === activeScene ? ' active' : ''),
              onPointerDown: () => { timer = setTimeout(() => sceneClear(i), 600); },
              onPointerUp: () => clearTimeout(timer), onPointerLeave: () => clearTimeout(timer),
              onClick: () => sceneClick(i) }, i + 1);
          })),
          h('p', { className: 'note', style: { fontSize: 11, color: 'var(--es-fg-muted)', margin: '9px 0 0' } },
            'Tap to recall · long-press a filled slot clears it. Step between scenes while playing.')),

        // Timing & output
        h(Section, { title: 'Timing & output' },
          h(Field, { label: 'Step length' },
            h('select', { className: 'es-control', value: subdiv,
              onChange: e => { setSubdiv(+e.target.value);
                if (juceAvailable()) {
                  sendParamActual('subdivision', +e.target.value);
                  sendParamActual('patternLengthUnit', 0);  // Steps mode — each step = the subdivision
                } } },
              SUBDIV.map((t, i) => h('option', { key: i, value: i }, 'each step = ' + t)))),
          h('p', { className: 'note', style: { fontSize: 11, color: 'var(--es-fg-muted)', margin: '-4px 0 10px' } },
            'How long each step lasts against the host beat. Selecting this puts the pattern in "step = subdivision" timing.'),
          h(Field, { label: 'Beat grouping (visual)' },
            h('select', { className: 'es-control', value: group, onChange: e => setGroup(+e.target.value) },
              [['2', '2'], ['3', '3'], ['4', '4'], ['0', 'none']].map(([v, t]) => h('option', { key: v, value: +v }, t)))),
          h(Slider, { label: 'Swing', value: swing, min: 0, max: 60, set: setSwing, fmt: v => v + '%' }),
          h(Field, { label: 'MIDI note' },
            h('select', { className: 'es-control', value: midiNote, onChange: e => { setMidiNote(+e.target.value); if (juceAvailable()) sendParamActual('midiNote', +e.target.value); } },
              [[36, 'C1 — kick'], [38, 'D1 — snare'], [42, 'F♯1 — hat'], [60, 'C3']].map(([v, t]) => h('option', { key: v, value: v }, t)))),
          h(Field, { label: 'MIDI channel' },
            h('select', { className: 'es-control', value: midiChan, onChange: e => setMidiChan(+e.target.value) },
              [1, 2, 3, 10].map(v => h('option', { key: v, value: v }, v))))),

        // Pattern database (web)
        cfg.web && h(Section, { title: 'Pattern database', badge: 'web' },
          h(Field, null, h('input', { className: 'es-control', type: 'text', placeholder: 'Search patterns…', value: dbQuery, onChange: e => setDbQuery(e.target.value) })),
          h('div', null, dbLib.map(([l, d, k, n]) =>
            h('div', { key: l, className: 'meter-row', style: { gridTemplateColumns: '1fr auto auto', cursor: 'pointer', gap: 8 }, onClick: () => { setAccText(''); setUpiText(l); } },
              h('span', { className: 'es-num', style: { color: 'var(--es-fg)' } }, l),
              h('span', { className: 'lab', style: { textAlign: 'right' } }, d),
              h('span', { className: 'es-badge es-num' }, `${k}/${n}`)))),
          h('p', { className: 'note', style: { fontSize: 11, color: 'var(--es-fg-muted)', margin: '9px 0 0' } },
            h('b', { className: 'es-num' }, '2,773'), ' patterns · filter by onsets, steps, balance.')),

        // Web Audio (web)
        cfg.web && h(Section, { title: 'Web Audio', badge: 'web' },
          h('label', { className: 'iconbtn', style: { height: 34, width: '100%', justifyContent: 'flex-start', marginBottom: 9 } },
            h('input', { type: 'checkbox', checked: waOn, onChange: e => setWaOn(e.target.checked) }), ' Audible click'),
          h(Slider, { label: 'Volume', value: Math.round(waVol * 100), min: 0, max: 100, set: v => setWaVol(v / 100), fmt: v => v + '%' })),

        // Host & automation (plugin)
        cfg.host && h(Section, { title: 'Host & automation', badge: 'host' },
          h('label', { className: 'iconbtn', style: { height: 38, width: '100%', justifyContent: 'flex-start', marginBottom: 10 } },
            h('input', { type: 'checkbox', checked: hostSync, onChange: e => { setHostSync(e.target.checked); if (juceAvailable()) sendParamActual('hostTransport', e.target.checked ? 1 : 0); } }), ' Follow host transport'),
          h('p', { className: 'es-eyebrow', style: { margin: '4px 0 8px' } }, 'Automatable parameters'),
          h('div', null, [['Pattern length', a.n], ['Subdivision', group || '—'], ['Accent velocity', accVel],
            ['MIDI note', midiNote], ['BPM', tempo], ['Host transport', hostSync ? 'on' : 'off']].map(([k, v]) =>
            h('div', { key: k, className: 'meter-row', style: { gridTemplateColumns: '1fr auto' } },
              h('span', { className: 'lab' }, k), h('span', { className: 'val', style: { textAlign: 'right' } }, String(v)))))))));
}

// ── small presentational helpers ──
function Section({ title, badge, open, children }) {
  return h('details', { className: 'es-section', open: !!open },
    h('summary', null, title, badge && h('span', { className: 'feat-badge ' + badge }, badge === 'web' ? 'web' : 'plugin')),
    h('div', { className: 'es-section-body' }, children));
}
function Field({ label, children }) {
  return h('div', { className: 'field' }, label && h('label', null, label), children);
}
function Slider({ label, value, min, max, set, fmt }) {
  const shown = fmt ? fmt(value) : value;
  return h('div', { className: 'field' },
    h('label', null, label, ' ', h('span', { className: 'v' }, shown)),
    h('input', { type: 'range', min, max, value, onChange: e => set(+e.target.value) }));
}
function Meter({ label, frac, text }) {
  return h('div', { className: 'meter-row' },
    h('span', { className: 'lab' }, label),
    h('div', { className: 'es-bar' }, h('div', { style: { width: Math.round(Math.max(0, Math.min(1, frac)) * 100) + '%' } })),
    h('span', { className: 'val' }, text));
}

ReactDOM.createRoot(document.getElementById('root')).render(h(SerpeApp));
