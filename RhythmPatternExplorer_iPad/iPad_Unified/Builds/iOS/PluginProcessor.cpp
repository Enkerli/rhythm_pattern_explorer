/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - Minimal AUv3 MIDI Effect
    Audio Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RhythmPatternExplorerIPadAudioProcessor::RhythmPatternExplorerIPadAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

RhythmPatternExplorerIPadAudioProcessor::~RhythmPatternExplorerIPadAudioProcessor()
{
}

//==============================================================================
const juce::String RhythmPatternExplorerIPadAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RhythmPatternExplorerIPadAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerIPadAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerIPadAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RhythmPatternExplorerIPadAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RhythmPatternExplorerIPadAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RhythmPatternExplorerIPadAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RhythmPatternExplorerIPadAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String RhythmPatternExplorerIPadAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void RhythmPatternExplorerIPadAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void RhythmPatternExplorerIPadAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    midiCollector.reset(sampleRate);
    
    juce::ignoreUnused (samplesPerBlock);
}

void RhythmPatternExplorerIPadAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RhythmPatternExplorerIPadAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RhythmPatternExplorerIPadAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear any audio data since this is a MIDI effect
    buffer.clear();
    
    // Get any queued MIDI messages from the collector and add them to output
    juce::MidiBuffer collectedMidi;
    midiCollector.removeNextBlockOfMessages(collectedMidi, buffer.getNumSamples());
    
    // Add collected MIDI to the output
    midiMessages.addEvents(collectedMidi, 0, buffer.getNumSamples(), 0);
}

//==============================================================================
bool RhythmPatternExplorerIPadAudioProcessor::hasEditor() const
{
    return true; // Will create a simple UI
}

juce::AudioProcessorEditor* RhythmPatternExplorerIPadAudioProcessor::createEditor()
{
    return new RhythmPatternExplorerIPadAudioProcessorEditor (*this);
}

//==============================================================================
void RhythmPatternExplorerIPadAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void RhythmPatternExplorerIPadAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
void RhythmPatternExplorerIPadAudioProcessor::sendMidiNote(int noteNumber, int velocity, int channel)
{
    // Create note on message
    auto noteOnMessage = juce::MidiMessage::noteOn(channel, noteNumber, static_cast<juce::uint8>(velocity));
    noteOnMessage.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    
    // Create note off message (100ms later)
    auto noteOffMessage = juce::MidiMessage::noteOff(channel, noteNumber, static_cast<juce::uint8>(0));
    noteOffMessage.setTimeStamp((juce::Time::getMillisecondCounterHiRes() + 100) * 0.001);
    
    // Add to MIDI collector
    midiCollector.addMessageToQueue(noteOnMessage);
    midiCollector.addMessageToQueue(noteOffMessage);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerIPadAudioProcessor();
}