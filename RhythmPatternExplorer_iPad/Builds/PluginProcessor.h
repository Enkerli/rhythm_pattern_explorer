/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - Minimal AUv3 MIDI Effect
    Audio Processor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * Minimal iPad AUv3 MIDI Effect Processor
 * 
 * This is a starting point for porting the full Rhythm Pattern Explorer
 * to iPad. Currently implements basic MIDI note sending functionality.
 */
class RhythmPatternExplorerIPadAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    RhythmPatternExplorerIPadAudioProcessor();
    ~RhythmPatternExplorerIPadAudioProcessor() override;

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
    // MIDI note sending functionality
    void sendMidiNote(int noteNumber = 60, int velocity = 100, int channel = 1);
    
private:
    //==============================================================================
    // MIDI collector for queuing outgoing MIDI messages
    juce::MidiMessageCollector midiCollector;
    
    // Basic timing
    double currentSampleRate = 44100.0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerIPadAudioProcessor)
};