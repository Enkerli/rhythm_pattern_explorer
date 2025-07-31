/*
  ==============================================================================

    iPad AUv3 MIDI Effect Template
    Audio Processor Implementation

    A minimal working template for creating iPad AUv3 MIDI effect plugins
    using JUCE framework.
    
    This template is in the public domain (CC0 1.0).
    Feel free to use it as a starting point for your own plugins.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TemplateAudioProcessor::TemplateAudioProcessor()
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
    // Initialize your plugin here
    // Add parameters, set up initial state, etc.
}

TemplateAudioProcessor::~TemplateAudioProcessor()
{
    // Clean up resources if needed
}

//==============================================================================
const juce::String TemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TemplateAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String TemplateAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void TemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void TemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Store sample rate for timing calculations
    currentSampleRate = sampleRate;
    
    // Reset MIDI collector with new sample rate
    midiCollector.reset(sampleRate);
    
    // Initialize your processing here
    // Set up filters, allocate buffers, etc.
    
    juce::ignoreUnused (samplesPerBlock);
}

void TemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    // MIDI effects don't process audio, so any layout is fine
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

void TemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear any audio data since this is a MIDI effect
    buffer.clear();
    
    // ========================================================================
    // YOUR MIDI PROCESSING LOGIC GOES HERE
    // ========================================================================
    
    // Example: Process incoming MIDI messages
    for (const auto message : midiMessages)
    {
        const auto midiMsg = message.getMessage();
        
        // Example: Log incoming MIDI (remove in production)
        if (midiMsg.isNoteOn())
        {
            // Process note on messages
            // Add your custom logic here
        }
        else if (midiMsg.isNoteOff())
        {
            // Process note off messages  
            // Add your custom logic here
        }
        else if (midiMsg.isController())
        {
            // Process CC messages
            // Add your custom logic here
        }
        
        // Forward the message (or modify it first)
        // midiMessages remains unchanged by default
    }
    
    // ========================================================================
    // ADD GENERATED MIDI MESSAGES
    // ========================================================================
    
    // Get any queued MIDI messages from UI interactions
    juce::MidiBuffer collectedMidi;
    midiCollector.removeNextBlockOfMessages(collectedMidi, buffer.getNumSamples());
    
    // Add generated MIDI to the output
    midiMessages.addEvents(collectedMidi, 0, buffer.getNumSamples(), 0);
}

//==============================================================================
bool TemplateAudioProcessor::hasEditor() const
{
    return true; // Set to false if you don't want a UI
}

juce::AudioProcessorEditor* TemplateAudioProcessor::createEditor()
{
    return new TemplateAudioProcessorEditor (*this);
}

//==============================================================================
void TemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store your plugin's state here
    // You can save parameters, settings, presets, etc.
    
    // Example using XML:
    // auto xml = std::make_unique<juce::XmlElement>("PluginState");
    // xml->setAttribute("someParameter", someParameterValue);
    // copyXmlToBinary(*xml, destData);
    
    juce::ignoreUnused (destData);
}

void TemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore your plugin's state here
    // This is called when loading projects, presets, etc.
    
    // Example using XML:
    // std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary(data, sizeInBytes));
    // if (xml && xml->hasTagName("PluginState"))
    // {
    //     someParameterValue = xml->getDoubleAttribute("someParameter", defaultValue);
    // }
    
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
void TemplateAudioProcessor::sendMidiNote(int noteNumber, int velocity, int channel)
{
    // Validate parameters
    noteNumber = juce::jlimit(0, 127, noteNumber);
    velocity = juce::jlimit(0, 127, velocity);
    channel = juce::jlimit(1, 16, channel);
    
    // Create note on message
    auto noteOnMessage = juce::MidiMessage::noteOn(channel, noteNumber, static_cast<juce::uint8>(velocity));
    noteOnMessage.setTimeStamp(juce::Time::getMillisecondCounterHiRes() * 0.001);
    
    // Create note off message (100ms later)
    auto noteOffMessage = juce::MidiMessage::noteOff(channel, noteNumber, static_cast<juce::uint8>(0));
    noteOffMessage.setTimeStamp((juce::Time::getMillisecondCounterHiRes() + 100) * 0.001);
    
    // Add to MIDI collector for thread-safe output
    midiCollector.addMessageToQueue(noteOnMessage);
    midiCollector.addMessageToQueue(noteOffMessage);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TemplateAudioProcessor();
}