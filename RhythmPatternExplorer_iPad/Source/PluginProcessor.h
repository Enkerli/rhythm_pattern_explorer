/*
  ==============================================================================

    iPad AUv3 MIDI Effect Template
    Audio Processor Header

    A minimal working template for creating iPad AUv3 MIDI effect plugins
    using JUCE framework.
    
    This template is in the public domain (CC0 1.0).
    Feel free to use it as a starting point for your own plugins.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * @brief Template Audio Processor for iPad AUv3 MIDI Effects
 * 
 * This class provides a minimal but complete foundation for iPad AUv3 MIDI 
 * effect plugins. It demonstrates:
 * 
 * - Proper AUv3 configuration for iOS deployment
 * - Thread-safe MIDI input/output handling
 * - Basic parameter management
 * - State saving/loading foundation
 * - Touch-optimized UI integration
 * 
 * To customize this template:
 * 1. Rename this class to match your plugin
 * 2. Update bundle identifiers in Projucer
 * 3. Modify MIDI processing logic in processBlock()
 * 4. Add your custom UI in the Editor files
 * 5. Implement parameter controls as needed
 * 
 * The template is designed to work out-of-the-box while providing
 * clear extension points for your custom functionality.
 */
class TemplateAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TemplateAudioProcessor();
    ~TemplateAudioProcessor() override;

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
    /**
     * @brief Send a MIDI note message (example functionality)
     * 
     * This demonstrates how to generate MIDI output from your plugin.
     * Replace this with your own MIDI generation logic.
     * 
     * @param noteNumber MIDI note number (0-127)
     * @param velocity Note velocity (0-127)  
     * @param channel MIDI channel (1-16)
     */
    void sendMidiNote(int noteNumber = 60, int velocity = 100, int channel = 1);
    
private:
    //==============================================================================
    /// Thread-safe MIDI message collector for output
    juce::MidiMessageCollector midiCollector;
    
    /// Current sample rate for timing calculations
    double currentSampleRate { 44100.0 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemplateAudioProcessor)
};