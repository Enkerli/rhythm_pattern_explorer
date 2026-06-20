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

        // UPI text from the UI — parsed by the authoritative C++ engine.
        .withEventListener ("setUPI", [owner] (const Array<var>& args)
        {
            if (args.isEmpty()) return;
            owner->proc.setUPIInput (args[0]["text"].toString());
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
    startTimer (200);   // 5 Hz — host transport readout
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
}

void SerpeEditor::sendTransport()
{
    double bpm = 120.0; bool playing = false;
    if (auto* ph = proc.getPlayHead())
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            playing = pos->getIsPlaying();
        }
    webView.emitEventIfBrowserIsVisible ("transport",
        makeObj ({ { "bpm", (int) juce::roundToInt (bpm) }, { "playing", playing } }));
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
        { "bpm",                (int)  raw ("bpm")                 },
        { "accentVelocity",            raw ("accentVelocity")      },
        { "unaccentedVelocity",        raw ("unaccentedVelocity")  },
        { "accentPitchOffset",  (int)  raw ("accentPitchOffset")   },
        { "midiNote",           (int)  raw ("midiNote")            },
        { "useHostTransport",   raw ("useHostTransport") > 0.5f     },
        { "upi",                proc.getUPIInput()                  },
    });

    webView.emitEventIfBrowserIsVisible ("stateSnapshot", snap);
}
