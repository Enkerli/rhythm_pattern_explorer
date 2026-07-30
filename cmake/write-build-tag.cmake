# Writes an esbuild --inject file carrying this bundle's build time, and a very
# small badge that shows it.
#
# WHY. On 2026-07-29 Serpe shipped a two-day-old WebUI bundle: its UI rejected a
# string its own C++ engine parsed, and nothing on screen said which bundle was
# running. Three rounds of investigation went into the producer and the consumer
# before anyone checked whether the JS was even current. MIDIcurator already
# displayed a build tag; the other four plugins did not.
#
# WRITTEN AT BUILD TIME, not configure time — a tag baked in at configure never
# changes when only the bundle is rebuilt, which is exactly the case it exists to
# detect. Identical content is left untouched so it doesn't force a rebundle
# every build.
#
# ONLY IN PLUGIN BUNDLES: the webapp dev server serves pages live, so staleness
# isn't a question there and this file is never part of that build.
#
# The badge is a diagnostic, not a feature — quiet, non-interactive, and
# deliberately uniform across the four plugins rather than fitted into each
# header. A design pass may prefer it inside each app's title area, the way
# MIDIcurator does it; moving it means deleting the block below.
#
# Invoked as: cmake -DOUT=<path> -P cmake/write-build-tag.cmake
string(TIMESTAMP _tag "%Y-%m-%d %H:%M" UTC)
set(_body "globalThis.__BUILD_TAG__ = \"${_tag} UTC\";
// Quiet build badge. Bottom-right, non-interactive, sits above app chrome.
(() => {
  const show = () => {
    if (!document.body || document.getElementById('es-build-tag')) return;
    const el = document.createElement('div');
    el.id = 'es-build-tag';
    el.textContent = 'build ' + globalThis.__BUILD_TAG__;
    el.title = 'Build tag \\u2014 confirms which bundle is running';
    el.style.cssText = 'position:fixed;right:6px;bottom:4px;z-index:2147483000;'
      + 'font:10px/1.4 ui-monospace,SFMono-Regular,Menlo,monospace;'
      + 'color:var(--es-fg-muted,#8a8a8a);opacity:.55;pointer-events:none;'
      + 'user-select:none;font-variant-numeric:tabular-nums;';
    document.body.appendChild(el);
  };
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', show);
  else show();
})();
")
if(EXISTS "${OUT}")
    file(READ "${OUT}" _old)
    if(_old STREQUAL "${_body}")
        return()   # same minute — don't touch it and force a needless rebundle
    endif()
endif()
file(WRITE "${OUT}" "${_body}")
