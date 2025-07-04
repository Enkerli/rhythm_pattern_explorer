/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Header (Fixed)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatternEngine.h"
#include "UPIParser.h"

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - MIDI effect processing (no audio synthesis)
 * - Parameter management
 * - Pattern generation and analysis
 * - DAW transport synchronization
 */
class RhythmPatternExplorerAudioProcessor  : public juce::AudioProcessor
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
    PatternEngine& getPatternEngine() { return patternEngine; }
    
    // Parameter access for editor
    juce::AudioParameterFloat* getBpmParameter() { return bpmParam; }
    juce::AudioParameterChoice* getPatternTypeParameter() { return patternTypeParam; }
    juce::AudioParameterInt* getOnsetsParameter() { return onsetsParam; }
    juce::AudioParameterInt* getStepsParameter() { return stepsParam; }
    juce::AudioParameterBool* getPlayingParameter() { return playingParam; }
    juce::AudioParameterBool* getUseHostTransportParameter() { return useHostTransportParam; }
    
    // Playback state
    int getCurrentStep() const { return currentStep.load(); }
    bool isCurrentlyPlaying() const { 
        // REFINED: Check if we're getting recent processBlock calls AND transport is playing
        double currentTime = juce::Time::getMillisecondCounter();
        bool recentProcessBlock = (currentTime - lastProcessBlockTime) < 100.0; // Within last 100ms
        
        // First try host transport, fall back to recent processBlock activity
        if (useHostTransportParam && useHostTransportParam->get()) {
            return hostIsPlaying && recentProcessBlock;
        } else {
            return playingParam->get() && recentProcessBlock;
        }
    }
    
    // UPI pattern input methods
    void setUPIInput(const juce::String& upiPattern);
    juce::String getUPIInput() const { return currentUPIInput; }
    void parseAndApplyUPI(const juce::String& upiPattern);
    

private:
    //==============================================================================
    // Pattern Engine
    PatternEngine patternEngine;
    
    // MIDI effect mode - no audio synthesis components needed
    
    // Timing and sequencing
    double currentSampleRate = 44100.0;
    int samplesPerStep = 0;
    int currentSample = 0;
    std::atomic<int> currentStep{0};
    bool wasPlaying = false;
    
    // DAW transport sync
    bool useHostTransport = true;
    double lastHostPosition = 0.0;
    bool hostIsPlaying = false;
    mutable double lastProcessBlockTime = 0.0;
    
    // Parameters - original working approach
    juce::AudioParameterFloat* bpmParam;
    juce::AudioParameterChoice* patternTypeParam;
    juce::AudioParameterInt* onsetsParam;
    juce::AudioParameterInt* stepsParam;
    juce::AudioParameterBool* playingParam;
    juce::AudioParameterBool* useHostTransportParam;
    
    // UPI pattern input
    juce::String currentUPIInput;
    juce::String lastParsedUPI;
    
    // Thread safety
    juce::CriticalSection processingLock;
    
    // Helper methods
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition);
    void syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void checkMidiInputForTriggers(juce::MidiBuffer& midiMessages);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};