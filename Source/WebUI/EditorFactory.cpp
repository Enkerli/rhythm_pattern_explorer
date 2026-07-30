/*
    The processor's one link to the editor, isolated in a file of its own.

    SerpeAudioProcessor::createEditor() is the ONLY thing tying the engine to the
    WebView UI. While it lived in PluginProcessor.cpp, anything that wanted the
    processor also got juce_gui_extra — which is why the headless dataflow probe
    dragged in a WebView it never opens, and failed on Linux at gtk/gtk.h with no
    GTK dev headers present (2026-07-30).

    Splitting the definition out means the plugin compiles this file and gets a
    real editor, while a headless target compiles the engine, links
    juce_audio_processors_headless, and supplies its own createEditor returning
    nullptr. Same processor, same behaviour in the DAW, no UI in the probe.

    hasEditor() deliberately stays in PluginProcessor.cpp: it returns true and
    names no editor type, so it costs the engine nothing.
*/
#include "../Platform/PluginProcessor.h"
#include "SerpeEditor.h"

juce::AudioProcessorEditor* SerpeAudioProcessor::createEditor()
{
    return new SerpeEditor (*this);
}
