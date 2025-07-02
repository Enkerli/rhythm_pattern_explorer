/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Implementation (Fixed)

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
    // Initialize parameters using original working approach
    addParameter(bpmParam = new juce::AudioParameterFloat("bpm", "BPM", 60.0f, 180.0f, 120.0f));
    addParameter(patternTypeParam = new juce::AudioParameterChoice("patternType", "Pattern Type", 
        juce::StringArray("Euclidean", "Polygon", "Random", "Binary", "UPI"), 0));
    addParameter(onsetsParam = new juce::AudioParameterInt("onsets", "Onsets", 1, 16, 3));
    addParameter(stepsParam = new juce::AudioParameterInt("steps", "Steps", 4, 32, 8));
    addParameter(playingParam = new juce::AudioParameterBool("playing", "Playing", false));
    addParameter(useHostTransportParam = new juce::AudioParameterBool("useHostTransport", "Use Host Transport", true));
    
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
    
    // MIDI effect mode - minimal audio setup
    // Reset sequencer state
    currentSample = 0;
    currentStep = 0;
    wasPlaying = false;
    
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
    juce::ScopedLock lock(processingLock);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Check for incoming MIDI that could trigger pattern regeneration
    checkMidiInputForTriggers(midiMessages);

    // Get transport info from host
    juce::AudioPlayHead* playHead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    bool hasValidPosition = false;
    
    if (playHead != nullptr)
    {
        hasValidPosition = playHead->getCurrentPosition(posInfo);
    }

    // Determine if we should be playing
    bool isPlaying;
    if (useHostTransportParam && useHostTransportParam->get() && hasValidPosition)
    {
        isPlaying = posInfo.isPlaying;
        syncWithHost(posInfo);
    }
    else
    {
        isPlaying = playingParam->get();
    }

    // Update pattern if parameters changed
    static int lastOnsets = 0;
    static int lastSteps = 0;
    static int lastPatternType = 0;
    
    int currentOnsets = onsetsParam->get();
    int currentSteps = stepsParam->get();
    int currentPatternType = patternTypeParam ? patternTypeParam->getIndex() : 0;
    
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
            case 4: // UPI
                // UPI patterns are set directly via setUPIInput(), no need to regenerate here
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
    xml->setAttribute("useHostTransport", useHostTransportParam->get());
    
    // Save pattern data
    auto patternXml = xml->createNewChildElement("Pattern");
    auto pattern = patternEngine.getCurrentPattern();
    juce::String patternString;
    for (int i = 0; i < pattern.size(); ++i)
    {
        patternString += pattern[i] ? "1" : "0";
    }
    patternXml->setAttribute("data", patternString);
    
    // Save UPI input
    auto upiXml = xml->createNewChildElement("UPI");
    upiXml->setAttribute("input", currentUPIInput);
    
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
        *useHostTransportParam = xml->getBoolAttribute("useHostTransport", true);
        
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
        
        // Restore UPI input
        auto upiXml = xml->getChildByName("UPI");
        if (upiXml != nullptr)
        {
            currentUPIInput = upiXml->getStringAttribute("input");
        }
        
        updateTiming();
    }
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::updateTiming()
{
    float bpm = bpmParam->get();
    
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
    
    // MIDI effect mode - no audio synthesis
    
    DBG("RhythmPatternExplorer: Note triggered at step " << currentStep);
}

void RhythmPatternExplorerAudioProcessor::syncWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Use host BPM if available
    if (posInfo.bpm > 0)
    {
        // Update our BPM parameter to match host
        if (std::abs(bpmParam->get() - posInfo.bpm) > 0.1f)
        {
            bpmParam->setValueNotifyingHost(bpmParam->convertTo0to1(static_cast<float>(posInfo.bpm)));
            updateTiming();
        }
    }
    
    // Calculate pattern position based on host timeline
    if (posInfo.ppqPosition >= 0.0)
    {
        // Calculate which step we should be on based on timeline position
        double beatsPerStep = 0.25; // 16th note = quarter beat
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // If we're significantly out of sync, jump to correct position
        if (std::abs(targetStep - currentStep) > 1)
        {
            currentStep = targetStep;
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
            DBG("RhythmPatternExplorer: Synced to host position - Step: " << currentStep << ", Sample: " << currentSample);
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // If the pattern changed, reset all progressive states
    if (currentUPIInput != upiPattern)
    {
        UPIParser::resetAllProgressiveStates();
    }
    
    currentUPIInput = upiPattern;
    parseAndApplyUPI(upiPattern);
}

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern)
{
    if (upiPattern.isEmpty())
        return;
    
    DBG("parseAndApplyUPI called with: '" + upiPattern + "'");
    
    // For progressive patterns, always re-parse to advance state
    bool isProgressive = upiPattern.contains(">");
    DBG("   Is progressive: " + juce::String(isProgressive ? "YES" : "NO"));
    
    auto parseResult = UPIParser::parse(upiPattern);
    
    if (parseResult.isValid())
    {
        DBG("Parse successful in processor");
        DBG("   Pattern: " + UPIParser::patternToBinary(parseResult.pattern));
        DBG("   Name: " + parseResult.patternName);
        
        // Apply the parsed pattern to the engine
        patternEngine.setPattern(parseResult.pattern);
        
        // Update parameters to reflect the new pattern
        int onsets = UPIParser::countOnsets(parseResult.pattern);
        int steps = static_cast<int>(parseResult.pattern.size());
        
        DBG("   Onsets: " + juce::String(onsets) + ", Steps: " + juce::String(steps));
        
        // Update parameters without triggering host notifications during parsing
        if (onsetsParam && stepsParam)
        {
            onsetsParam->setValueNotifyingHost(onsetsParam->convertTo0to1(static_cast<float>(onsets)));
            stepsParam->setValueNotifyingHost(stepsParam->convertTo0to1(static_cast<float>(steps)));
        }
        
        // Set pattern type to a custom indicator (use index 4 for UPI patterns)
        if (patternTypeParam)
        {
            patternTypeParam->setValueNotifyingHost(patternTypeParam->convertTo0to1(4.0f));
        }
        
        updateTiming();
        
        // Only cache non-progressive patterns to prevent re-parsing
        if (!isProgressive)
        {
            lastParsedUPI = upiPattern;
        }
        
        DBG("RhythmPatternExplorer: Parsed UPI pattern '" << upiPattern << "' -> " 
            << onsets << " onsets in " << steps << " steps");
        DBG("Binary: " << UPIParser::patternToBinary(parseResult.pattern));
    }
    else
    {
        DBG("RhythmPatternExplorer: Failed to parse UPI pattern '" << upiPattern 
            << "' - " << parseResult.errorMessage);
    }
}

void RhythmPatternExplorerAudioProcessor::checkMidiInputForTriggers(juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isNoteOn())
        {
            // Any MIDI note input triggers pattern regeneration for progressive/random patterns
            if (!currentUPIInput.isEmpty())
            {
                // Check if it's a progressive pattern (contains ">")
                if (currentUPIInput.contains(">"))
                {
                    // Force re-parsing to advance progressive transformation
                    parseAndApplyUPI(currentUPIInput);
                }
                else
                {
                    // Re-parse other UPI patterns to trigger regeneration
                    parseAndApplyUPI(currentUPIInput);
                }
            }
            else if (patternTypeParam && patternTypeParam->getIndex() == 2) // Random pattern type
            {
                // Trigger random pattern regeneration
                int currentOnsets = onsetsParam->get();
                int currentSteps = stepsParam->get();
                patternEngine.generateRandomPattern(currentOnsets, currentSteps);
                updateTiming();
            }
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}