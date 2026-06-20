#pragma once

// SerpeEditor — AudioProcessorEditor backed by a JUCE 8 WebBrowserComponent.
//
// The React UI is bundled in Source/WebUI/dist/ (index.html + bundle.js +
// bundle.css), embedded as binary data by CMake.
//   C++ → JS : webView.emitEventIfBrowserIsVisible("eventId", juce::var{…})
//   JS → C++ : Options::withEventListener("eventId", …)

#include <juce_audio_utils/juce_audio_utils.h>
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

    static std::optional<juce::WebBrowserComponent::Resource> provideResource (const juce::String& path);
    static juce::WebBrowserComponent::Options buildOptions (SerpeEditor* owner);

    SerpeAudioProcessor& proc;
    juce::WebBrowserComponent webView;

    juce::StringArray listenedParams;
    std::atomic<bool> pageReady     { false };
    bool              pageNavigated { false };
    juce::String      lastUPISent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpeEditor)
};
