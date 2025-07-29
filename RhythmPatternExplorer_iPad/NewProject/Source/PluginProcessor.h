/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - AUv3 Plugin
    Main Audio Processor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatternEngine.h"
#include "UPIParser.h"

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer iPad AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - MIDI effect processing (no audio synthesis)
 * - Pattern generation and analysis
 * - iPad-optimized UI integration
 */
class RhythmPatternExplorerAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    RhythmPatternExplorerAudioProcessor();
    ~RhythmPatternExplorerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Pattern Engine Access
    const PatternEngine& getPatternEngine() const { return patternEngine; }
    PatternEngine& getPatternEngine() { return patternEngine; }
    
    // UPI Input for pattern creation
    void setUPIInput(const juce::String& upiString);
    juce::String getCurrentUPIInput() const { return currentUPIInput; }
    
    // Current pattern access
    std::vector<bool> getCurrentPattern() const { return currentPattern; }
    
    // Basic parameters
    juce::AudioParameterInt* getMidiNoteParameter() const { return midiNoteParam; }
    juce::AudioParameterBool* getTickParameter() const { return tickParam; }
    
    // Debug info for UI
    juce::String getDebugInfo() const { return debugInfo; }
    
private:
    //==============================================================================
    // Core pattern engine
    PatternEngine patternEngine;
    
    // Current state
    juce::String currentUPIInput;
    std::vector<bool> currentPattern;
    
    // MIDI processing
    juce::MidiMessageCollector midiCollector;
    double currentSampleRate { 44100.0 };
    
    // Timing and playback
    int currentStep { 0 };
    double samplesPerStep { 0.0 };
    double sampleCounter { 0.0 };
    
    // Parameters
    juce::AudioParameterInt* midiNoteParam;
    juce::AudioParameterBool* tickParam;
    
    // Helper methods
    void updateTiming(juce::Optional<juce::AudioPlayHead::PositionInfo> position = {});
    void processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber);
    void processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber, int stepToProcess);
    
private:
    juce::String debugInfo = "Ready";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};