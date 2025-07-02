/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RhythmPatternExplorerAudioProcessor::RhythmPatternExplorerAudioProcessor()
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
    // Initialize parameters
    addParameter(bpmParam = new juce::AudioParameterFloat("bpm", "BPM", 60.0f, 180.0f, 120.0f));
    addParameter(patternTypeParam = new juce::AudioParameterChoice("patternType", "Pattern Type", 
        juce::StringArray("Euclidean", "Polygon", "Random", "Binary"), 0));
    addParameter(onsetsParam = new juce::AudioParameterInt("onsets", "Onsets", 1, 16, 3));
    addParameter(stepsParam = new juce::AudioParameterInt("steps", "Steps", 4, 32, 8));
    addParameter(playingParam = new juce::AudioParameterBool("playing", "Playing", false));
    
    // Initialize ADSR envelope
    adsrParams.attack = 0.01f;
    adsrParams.decay = 0.1f;
    adsrParams.sustain = 0.0f;
    adsrParams.release = 0.1f;
    adsr.setParameters(adsrParams);
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    DBG("RhythmPatternExplorer: Plugin initialized");
}

RhythmPatternExplorerAudioProcessor::~RhythmPatternExplorerAudioProcessor()
{
}

//==============================================================================
const juce::String RhythmPatternExplorerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RhythmPatternExplorerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool RhythmPatternExplorerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double RhythmPatternExplorerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RhythmPatternExplorerAudioProcessor::getNumPrograms()
{
    return 1;
}

int RhythmPatternExplorerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RhythmPatternExplorerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String RhythmPatternExplorerAudioProcessor::getProgramName (int index)
{
    return "Default";
}

void RhythmPatternExplorerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // Setup audio processing chain
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    oscillator.initialise([](float x) { return std::sin(x); });
    oscillator.prepare(spec);
    oscillator.setFrequency(440.0f); // A4 for now
    
    gain.prepare(spec);
    gain.setGainLinear(0.1f);
    
    adsr.setSampleRate(sampleRate);
    
    updateTiming();
    
    DBG("RhythmPatternExplorer: Prepared to play at " << sampleRate << " Hz");
}

void RhythmPatternExplorerAudioProcessor::releaseResources()
{
    DBG("RhythmPatternExplorer: Released resources");
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RhythmPatternExplorerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void RhythmPatternExplorerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    bool isPlaying = playingParam->get();
    
    // Update pattern if parameters changed
    static int lastOnsets = 0;
    static int lastSteps = 0;
    static int lastPatternType = 0;
    
    int currentOnsets = onsetsParam->get();
    int currentSteps = stepsParam->get();
    int currentPatternType = patternTypeParam->getIndex();
    
    if (currentOnsets != lastOnsets || currentSteps != lastSteps || currentPatternType != lastPatternType)
    {
        switch (currentPatternType)
        {
            case 0: // Euclidean
                patternEngine.generateEuclideanPattern(currentOnsets, currentSteps);
                break;
            case 1: // Polygon
                patternEngine.generatePolygonPattern(currentOnsets, currentSteps, 0);
                break;
            case 2: // Random
                patternEngine.generateRandomPattern(currentOnsets, currentSteps);
                break;
            case 3: // Binary
                patternEngine.generateBinaryPattern(currentOnsets < currentSteps ? (1 << currentOnsets) - 1 : 0, currentSteps);
                break;
        }
        
        lastOnsets = currentOnsets;
        lastSteps = currentSteps;
        lastPatternType = currentPatternType;
        
        updateTiming();
    }
    
    // Update timing if BPM changed
    updateTiming();
    
    // Process each sample
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (isPlaying)
        {
            // Check if we need to trigger a step
            if (currentSample >= samplesPerStep)
            {
                processStep(midiMessages, sample);
                currentSample = 0;
                currentStep = (currentStep + 1) % patternEngine.getStepCount();
            }
            currentSample++;
        }
        else if (wasPlaying)
        {
            // Just stopped playing - reset position
            currentSample = 0;
            currentStep = 0;
        }
        
        // Generate audio if ADSR is active
        float adsrOutput = adsr.getNextSample();
        if (adsrOutput > 0.001f)
        {
            float oscillatorSample = oscillator.processSample(0.0f);
            float outputSample = oscillatorSample * adsrOutput * 0.1f;
            
            for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            {
                buffer.addSample(channel, sample, outputSample);
            }
        }
    }
    
    wasPlaying = isPlaying;
}

//==============================================================================
bool RhythmPatternExplorerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RhythmPatternExplorerAudioProcessor::createEditor()
{
    return new RhythmPatternExplorerAudioProcessorEditor (*this);
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create an XML representation of our state
    auto xml = std::make_unique<juce::XmlElement>("RhythmPatternExplorerState");
    
    xml->setAttribute("bpm", bpmParam->get());
    xml->setAttribute("patternType", patternTypeParam->getIndex());
    xml->setAttribute("onsets", onsetsParam->get());
    xml->setAttribute("steps", stepsParam->get());
    xml->setAttribute("playing", playingParam->get());
    
    // Save pattern data
    auto patternXml = xml->createNewChildElement("Pattern");
    auto pattern = patternEngine.getCurrentPattern();
    juce::String patternString;
    for (int i = 0; i < pattern.size(); ++i)
    {
        patternString += pattern[i] ? "1" : "0";
    }
    patternXml->setAttribute("data", patternString);
    
    copyXmlToBinary(*xml, destData);
}

void RhythmPatternExplorerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    
    if (xml != nullptr && xml->hasTagName("RhythmPatternExplorerState"))
    {
        *bpmParam = xml->getDoubleAttribute("bpm", 120.0);
        *patternTypeParam = xml->getIntAttribute("patternType", 0);
        *onsetsParam = xml->getIntAttribute("onsets", 3);
        *stepsParam = xml->getIntAttribute("steps", 8);
        *playingParam = xml->getBoolAttribute("playing", false);
        
        // Restore pattern data if available
        auto patternXml = xml->getChildByName("Pattern");
        if (patternXml != nullptr)
        {
            juce::String patternString = patternXml->getStringAttribute("data");
            std::vector<bool> pattern;
            for (int i = 0; i < patternString.length(); ++i)
            {
                pattern.push_back(patternString[i] == '1');
            }
            patternEngine.setPattern(pattern);
        }
        
        updateTiming();
    }
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::updateTiming()
{
    float bpm = bpmParam->get();
    int steps = stepsParam->get();
    
    // Calculate samples per step (16th notes at current BPM)
    double beatsPerSecond = bpm / 60.0;
    double stepsPerSecond = beatsPerSecond * 4.0; // 16th note subdivisions
    samplesPerStep = static_cast<int>(currentSampleRate / stepsPerSecond);
    
    DBG("RhythmPatternExplorer: Updated timing - BPM: " << bpm << ", Samples per step: " << samplesPerStep);
}

void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    if (currentStep < pattern.size() && pattern[currentStep])
    {
        triggerNote(midiBuffer, samplePosition);
    }
}

void RhythmPatternExplorerAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition)
{
    // Send MIDI note
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, 36, 0.8f); // C2, velocity 102
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, 36, 0.0f);
    
    midiBuffer.addEvent(noteOn, samplePosition);
    midiBuffer.addEvent(noteOff, samplePosition + 100); // 100 samples note duration
    
    // Trigger audio envelope
    adsr.noteOn();
    
    DBG("RhythmPatternExplorer: Note triggered at step " << currentStep);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}