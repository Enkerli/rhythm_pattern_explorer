/**
 * Serpe — visualisers. Two framework-agnostic SVG views over a pattern:
 *   createCircleView(el, opts)  → ring + onset polygon + playhead + CoG
 *   createStepView(el, opts)    → linear step lane, beat grouping, playhead
 * Both expose .update({ steps, accents, playhead, ... }) and theme via tokens.
 */
import { centerOfGravity } from "./analysis.js";

const NS = "http://www.w3.org/2000/svg";
const TAU = Math.PI * 2;
const el = (n, a = {}) => {
  const e = document.createElementNS(NS, n);
  for (const k in a) e.setAttribute(k, a[k]);
  return e;
};
// step 0 at top (12 o'clock), clockwise
const ang = (i, n) => (TAU * i) / n - Math.PI / 2;
const pol = (cx, cy, r, a) => [cx + r * Math.cos(a), cy + r * Math.sin(a)];

export function createCircleView(host, opts = {}) {
  const state = { steps: [], accents: [], playhead: -1, showCog: true, showLabels: false, lane: "ink", ...opts };
  const svg = el("svg", { viewBox: "0 0 320 320", role: "img" });
  svg.setAttribute("aria-label", "Rhythm circle");
  svg.style.width = "100%";
  svg.style.height = "auto";
  svg.style.display = "block";
  host.replaceChildren(svg);

  const laneColor = (lane) => ({
    ink: "var(--es-accent)", rose: "var(--es-dim-pressure)",
    moss: "var(--es-dim-expr)", plum: "var(--es-dim-slide)",
  }[lane] || "var(--es-accent)");

  function render() {
    const { steps, accents, playhead } = state;
    const n = steps.length || 1;
    const cx = 160, cy = 160, R = 118;
    const kids = [];
    const accent = laneColor(state.lane);

    // guide ring
    kids.push(el("circle", { cx, cy, r: R, fill: "none", stroke: "var(--es-border)", "stroke-width": 1.5 }));

    // spokes (subtle) + downbeat tick
    for (let i = 0; i < n; i++) {
      const [x, y] = pol(cx, cy, R, ang(i, n));
      const [xi] = pol(cx, cy, R - 6, ang(i, n));
      const isDown = i === 0;
      kids.push(el("line", {
        x1: cx, y1: cy, x2: x, y2: y,
        stroke: "var(--es-border-soft)", "stroke-width": isDown ? 1.5 : 0.75,
      }));
    }

    // onset polygon
    const onsets = [];
    for (let i = 0; i < n; i++) if (steps[i]) onsets.push(i);
    if (onsets.length >= 2) {
      const pts = onsets.map((i) => pol(cx, cy, R, ang(i, n)).map((v) => v.toFixed(1)).join(",")).join(" ");
      kids.push(el("polygon", {
        points: pts, fill: `color-mix(in oklab, ${accent} 14%, transparent)`,
        stroke: accent, "stroke-width": 2.5, "stroke-linejoin": "round",
      }));
    }

    // center of gravity vector
    if (state.showCog && onsets.length >= 1) {
      const cog = centerOfGravity(steps);
      const a = ang(cog.angleSteps, n);
      const [gx, gy] = pol(cx, cy, R * cog.magnitude, a);
      if (cog.magnitude > 0.012) {
        kids.push(el("line", { x1: cx, y1: cy, x2: gx, y2: gy, stroke: "var(--es-fg-muted)", "stroke-width": 1.5, "stroke-dasharray": "3 3" }));
        kids.push(el("circle", { cx: gx, cy: gy, r: 4, fill: "var(--es-fg-muted)" }));
      } else {
        // perfectly balanced — mark the center
        kids.push(el("circle", { cx, cy, r: 6, fill: "none", stroke: "var(--es-dim-expr)", "stroke-width": 2 }));
      }
    }

    // playhead wedge
    if (playhead >= 0 && playhead < n) {
      const a0 = ang(playhead - 0.5, n), a1 = ang(playhead + 0.5, n);
      const [x0, y0] = pol(cx, cy, R + 8, a0);
      const [x1, y1] = pol(cx, cy, R + 8, a1);
      const wedge = el("path", {
        d: `M ${cx} ${cy} L ${x0.toFixed(1)} ${y0.toFixed(1)} A ${R + 8} ${R + 8} 0 0 1 ${x1.toFixed(1)} ${y1.toFixed(1)} Z`,
        fill: `color-mix(in oklab, ${accent} 12%, transparent)`,
      });
      kids.push(wedge);
    }

    // step nodes — accented onsets in the suite amber, plain onsets in the accent
    const accentAmber = "var(--es-dim-pressure)";
    // thin labels on large cycles so they don't overlap / shrink illegibly
    const labelEvery = n <= 16 ? 1 : n <= 32 ? 2 : Math.ceil(n / 16);
    for (let i = 0; i < n; i++) {
      const [x, y] = pol(cx, cy, R, ang(i, n));
      const on = !!steps[i];
      const acc = !!accents[i];
      const here = i === playhead;
      const r = on ? (acc ? 12 : 9.5) : 5.5;
      const onColor = acc ? accentAmber : accent;
      if (acc && on) {
        kids.push(el("circle", { cx: x, cy: y, r: r + 3.5, fill: "none", stroke: onColor, "stroke-width": 1.5 }));
      }
      const node = el("circle", {
        cx: x, cy: y, r,
        fill: on ? onColor : "var(--es-bg-raised)",
        stroke: here ? "var(--es-fg)" : on ? "var(--es-fg)" : "var(--es-border-strong)",
        "stroke-width": here ? 3 : on ? 1.5 : 1.25,
      });
      if (state.onToggle) {
        // generous invisible hit target over each node for tapping
        const hit = el("circle", { cx: x, cy: y, r: Math.max(r + 6, 14), fill: "transparent", style: "cursor:pointer" });
        hit.addEventListener("click", ((idx) => () => state.onToggle(idx, steps[idx]))(i));
        kids.push(node, hit);
      } else {
        kids.push(node);
      }
      if (state.showLabels && (on || i % labelEvery === 0)) {
        const [lx, ly] = pol(cx, cy, R + 22, ang(i, n));
        const t = el("text", {
          x: lx, y: ly, "text-anchor": "middle", "dominant-baseline": "central",
          "font-size": 11, "font-family": "var(--es-font-mono)",
          fill: on ? "var(--es-fg)" : "var(--es-fg-muted)",
        });
        t.textContent = i;
        kids.push(t);
      }
    }
    svg.replaceChildren(...kids);
  }
  render();
  return { update(next) { Object.assign(state, next); render(); }, el: svg };
}

export function createStepView(host, opts = {}) {
  const state = { steps: [], accents: [], playhead: -1, group: 4, lane: "ink", ...opts };
  const wrap = document.createElement("div");
  wrap.className = "serpe-steplane";
  host.replaceChildren(wrap);

  function render() {
    const { steps, accents, playhead, group, onToggle } = state;
    const n = steps.length;
    // wrap large cycles into rows of a sensible width instead of squeezing one row
    const cols = n <= 16 ? n : (group && n % group === 0 && n / group <= 16 ? n / group : 16);
    wrap.style.setProperty("--cols", cols);
    const showNums = n <= 16 ? 1 : n <= 32 ? 2 : Math.ceil(n / 16);
    const cells = [];
    for (let i = 0; i < n; i++) {
      const c = document.createElement("div");
      c.className = "serpe-step";
      if (steps[i]) c.classList.add("on");
      if (accents[i]) c.classList.add("acc");
      if (i === playhead) c.classList.add("here");
      if (group && i % group === 0) c.classList.add("beat");
      if (onToggle) c.addEventListener("click", () => onToggle(i, steps[i]));
      if (steps[i] || i % showNums === 0) {
        const lab = document.createElement("span");
        lab.className = "serpe-step-n";
        lab.textContent = i;
        c.appendChild(lab);
      }
      cells.push(c);
    }
    wrap.replaceChildren(...cells);
  }
  render();
  return { update(next) { Object.assign(state, next); render(); }, el: wrap };
}
