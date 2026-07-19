#pragma once

// SerpeEditor — AudioProcessorEditor backed by a JUCE 8 WebBrowserComponent.
//
// The React UI is bundled in Source/WebUI/dist/ (index.html + bundle.js +
// bundle.css), embedded as binary data by CMake.
//   C++ → JS : webView.emitEventIfBrowserIsVisible("eventId", juce::var{…})
//   JS → C++ : Options::withEventListener("eventId", …)

#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include "../Platform/PluginProcessor.h"

class SerpeEditor : public juce::AudioProcessorEditor,
                    private juce::AudioProcessorValueTreeState::Listener,
                    private juce::Timer
{
public:
    explicit SerpeEditor (SerpeAudioProcessor& p);
    ~SerpeEditor() override;

    void paint   (juce::Graphics&) override {}
    void resized () override;
    void parentHierarchyChanged() override;

private:
    void parameterChanged (const juce::String& paramID, float newValue) override;
    void timerCallback() override;

    void navigateIfNeeded();
    void sendStateSnapshot();
    void sendTransport();
    void sendEngineState();   // the engine's actual pattern + accents (display = audio)
    void sendPolyState();     // per-lane live step, for the poly panel's playhead

    static std::optional<juce::WebBrowserComponent::Resource> provideResource (const juce::String& path);
    static juce::WebBrowserComponent::Options buildOptions (SerpeEditor* owner);

    SerpeAudioProcessor& proc;
    juce::WebBrowserComponent webView;

    juce::StringArray listenedParams;
    std::atomic<bool> pageReady     { false };
    bool              pageNavigated { false };
    juce::String      lastUPISent;

    // Last transport values sent — only emit on change so the WebUI isn't
    // re-rendered 30x/s (which would destroy click targets while editing).
    int  lastStep        { -2 };
    int  lastBpm         { -1 };
    int  lastAccentOffset{ -1 };
    bool lastPlaying     { false };
    bool lastHostSync    { false };
    bool transportSent   { false };

    juce::String lastPattern, lastAccents;   // for change-detecting engineState pushes

    bool lastPolyActive { false };
    std::array<int, SerpeAudioProcessor::kMaxPolyLanes> lastPolySteps {
        { -2, -2, -2, -2, -2, -2 } };   // for change-detecting polyState pushes

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpeEditor)
};
