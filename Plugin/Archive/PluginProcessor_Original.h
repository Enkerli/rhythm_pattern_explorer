/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatternEngine.h"

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - Audio processing (synthesis)
 * - Parameter management
 * - Pattern generation and analysis
 * - Host synchronization
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

private:
    //==============================================================================
    // Pattern Engine
    PatternEngine patternEngine;
    
    // Audio synthesis
    juce::dsp::Oscillator<float> oscillator;
    juce::dsp::Gain<float> gain;
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    
    // Timing and sequencing
    double currentSampleRate = 44100.0;
    int samplesPerStep = 0;
    int currentSample = 0;
    int currentStep = 0;
    bool wasPlaying = false;
    
    // Parameters
    juce::AudioParameterFloat* bpmParam;
    juce::AudioParameterChoice* patternTypeParam;
    juce::AudioParameterInt* onsetsParam;
    juce::AudioParameterInt* stepsParam;
    juce::AudioParameterBool* playingParam;
    
    // Helper methods
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};