#include "SerpeEditor.h"
#include <BinaryDataWebUI.h>

// ── Helpers ───────────────────────────────────────────────────────────────────

static juce::var makeObj (std::initializer_list<std::pair<juce::String, juce::var>> pairs)
{
    auto obj = std::make_unique<juce::DynamicObject>();
    for (auto& [k, v] : pairs) obj->setProperty (k, v);
    return juce::var (obj.release());
}

// ── Resource provider (serves the embedded React bundle) ──────────────────────

std::optional<juce::WebBrowserComponent::Resource>
SerpeEditor::provideResource (const juce::String& path)
{
    struct Entry { const char* data; int size; juce::String mime; };
    static const std::array<std::pair<const char*, Entry>, 3> table {{
        { "/",           { BinaryData::index_html, BinaryData::index_htmlSize, "text/html; charset=utf-8" } },
        { "/index.html", { BinaryData::index_html, BinaryData::index_htmlSize, "text/html; charset=utf-8" } },
        { "/bundle.js",  { BinaryData::bundle_js,  BinaryData::bundle_jsSize,  "application/javascript"   } },
    }};

    for (auto& [key, entry] : table)
        if (path == key)
        {
            std::vector<std::byte> bytes (static_cast<std::size_t> (entry.size));
            std::memcpy (bytes.data(), entry.data, static_cast<std::size_t> (entry.size));
            return juce::WebBrowserComponent::Resource { std::move (bytes), entry.mime };
        }
    return std::nullopt;
}

// ── Options builder ───────────────────────────────────────────────────────────

juce::WebBrowserComponent::Options SerpeEditor::buildOptions (SerpeEditor* owner)
{
    using juce::WebBrowserComponent;
    using juce::var;
    using juce::Array;

    return WebBrowserComponent::Options{}
        .withResourceProvider ([] (const juce::String& path) { return SerpeEditor::provideResource (path); },
            WebBrowserComponent::getResourceProviderRoot())
        .withNativeIntegrationEnabled()
       #if JUCE_MAC
        .withKeepPageLoadedWhenBrowserIsHidden()
       #endif

        .withEventListener ("log", [] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            std::fprintf (stderr, "[serpe-js:%s] %s\n",
                          args[0]["level"].toString().toRawUTF8(),
                          args[0]["msg"].toString().toRawUTF8());
        })

        .withEventListener ("uiReady", [owner] (const Array<var>&)
        {
            owner->pageReady = true;
            juce::Component::SafePointer<SerpeEditor> safe (owner);
            juce::MessageManager::callAsync ([safe] { if (safe) safe->sendStateSnapshot(); });
        })

        // UPI text from the UI — parsed by the authoritative C++ engine, which
        // then reports back the actual pattern + accents it produced.
        .withEventListener ("setUPI", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            owner->proc.setUPIInput (args[0]["text"].toString());
            juce::Component::SafePointer<SerpeEditor> safe (owner);
            juce::MessageManager::callAsync ([safe] { if (safe) safe->sendEngineState(); });
        })

        // Standalone play/stop (no host transport) — drives the internal sequencer.
        .withEventListener ("setPlaying", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            owner->proc.setInternalPlaying (static_cast<bool> (args[0]["playing"]));
        })

        // Manual tempo (standalone).
        .withEventListener ("setBPM", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            owner->proc.setCurrentBPM (static_cast<float> (static_cast<double> (args[0]["bpm"])));
        })

        // Toggle a step's accent in the engine (manual-accent mode). No snapshot
        // echo — the UI keeps its own explicit accent override; echoing the upi
        // would clear it. (Onset edits go through setUPI, not a toggle event.)
        .withEventListener ("toggleAccent", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            owner->proc.toggleAccentAtStep (static_cast<int> (args[0]["step"]));
            juce::Component::SafePointer<SerpeEditor> safe (owner);
            juce::MessageManager::callAsync ([safe] { if (safe) safe->sendEngineState(); });
        })

        // Parameter change in actual domain — C++ normalises and notifies host.
        .withEventListener ("setParamActual", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            const auto id  = args[0]["id"].toString();
            const float val = static_cast<float> (static_cast<double> (args[0]["value"]));
            if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (owner->proc.getAPVTS().getParameter (id)))
                p->setValueNotifyingHost (p->convertTo0to1 (val));
        });
}

// ── Construction ──────────────────────────────────────────────────────────────

SerpeEditor::SerpeEditor (SerpeAudioProcessor& p)
    : AudioProcessorEditor (p), proc (p), webView (buildOptions (this))
{
    addAndMakeVisible (webView);
    setResizable (true, true);
    setResizeLimits (360, 480, 1600, 1200);
    setSize (1080, 760);

    for (auto* param : proc.getParameters())
        if (auto* pwid = dynamic_cast<juce::AudioProcessorParameterWithID*> (param))
        {
            proc.getAPVTS().addParameterListener (pwid->paramID, this);
            listenedParams.add (pwid->paramID);
        }

    juce::Component::SafePointer<SerpeEditor> safe (this);
    juce::MessageManager::callAsync ([safe] { if (safe) safe->navigateIfNeeded(); });
    startTimer (33);   // ~30 Hz — host transport readout + playhead
}

SerpeEditor::~SerpeEditor()
{
    stopTimer();
    for (const auto& id : listenedParams)
        proc.getAPVTS().removeParameterListener (id, this);
}

// ── Navigation / layout ───────────────────────────────────────────────────────

void SerpeEditor::navigateIfNeeded()
{
    if (pageNavigated) return;
    pageNavigated = true;
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());
}

void SerpeEditor::parentHierarchyChanged()
{
    AudioProcessorEditor::parentHierarchyChanged();
    if (getParentComponent() != nullptr) navigateIfNeeded();
}

void SerpeEditor::resized() { webView.setBounds (getLocalBounds()); }

// ── Timer: push host transport (BPM + playing) for the sync chip ──────────────

void SerpeEditor::timerCallback()
{
    if (!pageReady) return;
    sendTransport();
    sendEngineState();   // re-pushes only when the pattern/accents actually change
    sendPolyState();     // re-pushes only when a lane's step actually changes
}

// The engine's actual rendered state: which steps are onsets, and which are
// accented THIS cycle. Pushed only on change so it doesn't spam / re-render
// the WebUI.
void SerpeEditor::sendEngineState()
{
    const juce::String pattern = proc.getCurrentPatternDisplay();

    // Mirror processStep() EXACTLY so the displayed accents match what's heard:
    //  - suspension mode (manual edits): per-step accents (shouldStepBeAccented);
    //  - normal mode (UPI patterns): the accent the audio plays for each onset is
    //    shouldOnsetBeAccented(globalOnsetCount), and that onset's global count is
    //    getCycleStartOnsetCount() + its index within this cycle. Using the live
    //    cycle-start count (not the boundary-latched uiAccentOffset) keeps the
    //    display in lockstep through precession AND through a progressive pattern
    //    change, which resets baseTickRhythm — the previous offset latched at the
    //    wrong time and drifted on a shift (e.g. {100}B(0,17)>17).
    const bool manual = proc.isPatternManuallyModified();
    const uint32_t cycleStart = proc.getCycleStartOnsetCount();
    juce::String accents;
    int onsetIdx = 0;
    for (int i = 0; i < pattern.length(); ++i)
    {
        const bool isOnset = (pattern[i] == '1');
        bool acc = false;
        if (manual)
            acc = proc.shouldStepBeAccented (i);
        else if (isOnset)
            acc = proc.shouldOnsetBeAccented (static_cast<int> (cycleStart) + onsetIdx);
        if (isOnset) ++onsetIdx;
        accents += acc ? '1' : '0';
    }

    if (pattern == lastPattern && accents == lastAccents) return;
    lastPattern = pattern; lastAccents = accents;

    webView.emitEventIfBrowserIsVisible ("engineState", makeObj ({
        { "pattern", pattern },
        { "accents", accents },
    }));
}

void SerpeEditor::sendTransport()
{
    const bool hostSync = proc.getAPVTS().getRawParameterValue ("useHostTransport") != nullptr
                       && proc.getAPVTS().getRawParameterValue ("useHostTransport")->load() > 0.5f;
    const int  bpm          = (int) juce::roundToInt (proc.getCurrentBPM());
    const bool playing      = proc.isCurrentlyPlaying();
    const int  step         = playing ? proc.getCurrentStep() : -1;
    const int  accentOffset = proc.getUIAccentOffset();

    // Only emit when something actually changed — pushing 30x/s would re-render
    // the WebUI continuously and make step cells un-clickable.
    if (transportSent && step == lastStep && playing == lastPlaying && bpm == lastBpm
        && accentOffset == lastAccentOffset && hostSync == lastHostSync)
        return;
    lastStep = step; lastPlaying = playing; lastBpm = bpm;
    lastAccentOffset = accentOffset; lastHostSync = hostSync; transportSent = true;

    webView.emitEventIfBrowserIsVisible ("transport", makeObj ({
        { "bpm",          bpm },
        { "playing",      playing },
        { "step",         step },           // drives the UI playhead (-1 when stopped)
        { "accentOffset", accentOffset },   // precesses the displayed accents
        { "hostSync",     hostSync },
    }));
}

// ── Parameter listener: relay host/automation changes to the UI ───────────────

void SerpeEditor::parameterChanged (const juce::String& paramID, float newValue)
{
    if (!pageReady) return;
    juce::Component::SafePointer<SerpeEditor> safe (this);
    juce::MessageManager::callAsync ([safe, paramID, newValue]
    {
        if (safe == nullptr || !safe->pageReady) return;
        safe->webView.emitEventIfBrowserIsVisible ("paramChange",
            makeObj ({ { "id", paramID }, { "value", newValue } }));
    });
}

// ── State snapshot (actual-domain values + current UPI) ───────────────────────

void SerpeEditor::sendStateSnapshot()
{
    auto& apvts = proc.getAPVTS();
    const auto raw = [&] (const char* id) -> float
    {
        if (auto* v = apvts.getRawParameterValue (id)) return v->load();
        return 0.0f;
    };

    const auto snap = makeObj ({
        // BPM is a processor member (currentBPM), NOT an APVTS param — querying
        // a "bpm" param returned 0 (the cause of the runaway default tempo).
        { "bpm",                (int) juce::roundToInt (proc.getCurrentBPM())  },
        { "accentVelocity",            raw ("accentVelocity")      },
        { "unaccentedVelocity",        raw ("unaccentedVelocity")  },
        { "accentPitchOffset",  (int)  raw ("accentPitchOffset")   },
        { "midiNote",           (int)  raw ("midiNote")            },
        { "useHostTransport",   raw ("useHostTransport") > 0.5f     },
        { "subdivision",        (int)  raw ("subdivision")          },
        { "patternLengthUnit",  (int)  raw ("patternLengthUnit")    },
        { "patternLengthValue", (int)  raw ("patternLengthValue")   },
        { "internalPlaying",    proc.getInternalPlaying()           },
        { "upi",                proc.getUPIInput()                  },
        // Poly lanes (music-suite docs/SERPE_POLY.md §8 milestone 3): lane
        // note/channel/mute + the base lag, so the WebView's lane controls
        // reflect real automatable parameters on load, not just defaults.
        { "polyLagMs",          raw ("polyLagMs")                   },
        { "polyLock",    (int) raw ("polyLock")                     },
        { "laneNote0",   (int) raw ("laneNote0")   }, { "laneChannel0", (int) raw ("laneChannel0") }, { "laneMute0", raw ("laneMute0") > 0.5f },
        { "laneNote1",   (int) raw ("laneNote1")   }, { "laneChannel1", (int) raw ("laneChannel1") }, { "laneMute1", raw ("laneMute1") > 0.5f },
        { "laneNote2",   (int) raw ("laneNote2")   }, { "laneChannel2", (int) raw ("laneChannel2") }, { "laneMute2", raw ("laneMute2") > 0.5f },
        { "laneNote3",   (int) raw ("laneNote3")   }, { "laneChannel3", (int) raw ("laneChannel3") }, { "laneMute3", raw ("laneMute3") > 0.5f },
        { "laneNote4",   (int) raw ("laneNote4")   }, { "laneChannel4", (int) raw ("laneChannel4") }, { "laneMute4", raw ("laneMute4") > 0.5f },
        { "laneNote5",   (int) raw ("laneNote5")   }, { "laneChannel5", (int) raw ("laneChannel5") }, { "laneMute5", raw ("laneMute5") > 0.5f },
    });

    webView.emitEventIfBrowserIsVisible ("stateSnapshot", snap);
}

// Per-lane live step, for the poly lanes panel's playhead — the mono
// equivalent of sendTransport's `step`, but one per active lane. Only runs
// (and only costs anything) while a poly pattern is actually active.
void SerpeEditor::sendPolyState()
{
    if (!proc.getIsPolyPattern())
    {
        if (lastPolyActive)
        {
            lastPolyActive = false;
            webView.emitEventIfBrowserIsVisible ("polyState", makeObj ({ { "active", false } }));
        }
        return;
    }

    juce::Array<juce::var> steps;
    bool changed = !lastPolyActive;
    for (int i = 0; i < SerpeAudioProcessor::kMaxPolyLanes; ++i)
    {
        const int s = proc.getPolyLaneStep (i);
        steps.add (s);
        if (s != lastPolySteps[static_cast<size_t> (i)]) changed = true;
        lastPolySteps[static_cast<size_t> (i)] = s;
    }
    lastPolyActive = true;
    if (!changed) return;

    webView.emitEventIfBrowserIsVisible ("polyState", makeObj ({
        { "active", true },
        { "steps",  juce::var (steps) },
    }));
}
