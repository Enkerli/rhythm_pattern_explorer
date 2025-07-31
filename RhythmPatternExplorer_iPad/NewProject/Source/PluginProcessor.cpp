/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - AUv3 Plugin
    Main Audio Processor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DebugLogger.h"

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
    // Add parameters
    addParameter(midiNoteParam = new juce::AudioParameterInt("midiNote", "MIDI Note", 21, 108, 60));
    addParameter(tickParam = new juce::AudioParameterBool("tick", "Trigger", false));
    
    // Initialize pattern engine connection
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    // Initialize advanced management systems
    sceneManager = std::make_unique<SceneManager>();
    progressiveManager = std::make_unique<ProgressiveManager>();
    
    // Initialize pattern with basic Euclidean pattern
    currentUPIInput = "E(3,8)";
    auto parseResult = UPIParser::parse(currentUPIInput);
    if (parseResult.isValid())
    {
        currentPattern = parseResult.pattern;
    }
    else
    {
        // Fallback pattern if parsing fails
        currentPattern = {true, false, false, true, false, false, true, false};
    }
}

RhythmPatternExplorerAudioProcessor::~RhythmPatternExplorerAudioProcessor()
{
    // Clean up resources if needed
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
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int RhythmPatternExplorerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RhythmPatternExplorerAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String RhythmPatternExplorerAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void RhythmPatternExplorerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Store sample rate for timing calculations
    currentSampleRate = sampleRate;
    
    // Reset MIDI collector with new sample rate
    midiCollector.reset(sampleRate);
    
    // Initialize your processing here
    // Set up filters, allocate buffers, etc.
    
    juce::ignoreUnused (samplesPerBlock);
}

void RhythmPatternExplorerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool RhythmPatternExplorerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void RhythmPatternExplorerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    // Clear any audio data since this is a MIDI effect
    buffer.clear();
    
    // Check for parameter changes
    bool tickTriggered = tickParam->get();
    if (tickTriggered)
    {
        // Reset tick parameter
        tickParam->setValueNotifyingHost(0.0f);
        
        // Handle progressive transformations and scenes (desktop-compatible logic)
        if (!currentUPIInput.isEmpty())
        {
            // Use original UPI input if available (preserves progressive/scene syntax)
            juce::String upiToProcess = originalUPIInput.isEmpty() ? currentUPIInput : originalUPIInput;
            
            // Check for scenes and progressive patterns to handle them correctly
            bool hasProgressiveTransformation = upiToProcess.contains(">");
            bool hasScenes = upiToProcess.contains("|");
            
            bool triggerNeeded = false;
            
            if (hasScenes) {
                // CRITICAL FIX: If we have scenes, handle scene advancement first
                // This prevents double/triple advancement when scenes contain progressive transformations
                advanceScene();
                applyCurrentScenePattern(); 
                debugInfo = "TRIGGER advanced to Scene " + juce::String(sceneManager->getCurrentSceneIndex() + 1) + 
                           " - " + sceneManager->getCurrentScenePattern();
                triggerNeeded = true;
            }
            else if (hasProgressiveTransformation) {
                // Progressive transformations: advance without resetting accents
                // Only process this if we DON'T have scenes (to avoid double advancement)
                parseAndApplyUPI(upiToProcess, false); // false = preserve accents
                debugInfo = "TRIGGER advanced progressive transformation";
                triggerNeeded = true;
            }
            
            // For regular patterns without scenes or progressive transformations
            if (!triggerNeeded) {
                parseAndApplyUPI(upiToProcess, true); // true = reset accents for new patterns
                debugInfo = "TRIGGER regular pattern";
            }
        }
        
        // Reset pattern playback
        currentStep = 0;
        sampleCounter = 0.0;
    }
    
    // Get transport info
    auto position = getPlayHead()->getPosition();
    bool isPlaying = position.hasValue() && position->getIsPlaying();
    
    // Update timing with host BPM if available
    updateTiming(position);
    
    // Generate pattern-based MIDI only if transport is playing or manually triggered
    if (!currentPattern.empty() && (isPlaying || tickTriggered))
    {
        if (isPlaying && position.hasValue())
        {
            // Use desktop timing system
            double hostBPM = position->getBpm().orFallback(120.0);
            auto ppqPosition = position->getPpqPosition().orFallback(0.0);
            
            // Calculate pattern duration in beats (simplified auto mode)
            int patternSteps = static_cast<int>(currentPattern.size());
            double patternLengthInBeats = std::max(2.0, patternSteps / 4.0);
            
            // Calculate beats per step based on pattern length parameters
            double beatsPerStep = patternLengthInBeats / patternSteps;
            double currentBeat = ppqPosition;
            double stepsFromStart = currentBeat / beatsPerStep;
            
            // Calculate which step should be active within the current pattern cycle
            double stepsInCurrentCycle = fmod(stepsFromStart, patternSteps);
            int targetStep = static_cast<int>(stepsInCurrentCycle);
            
            // Log timing info occasionally and update debug display
            static int logCounter = 0;
            if (logCounter++ % 1000 == 0) // Log every ~20 seconds at 44kHz
            {
                DEBUG_LOG(juce::String::formatted("Pattern: size=%d, lengthBeats=%.2f, beatsPerStep=%.4f", 
                    patternSteps, patternLengthInBeats, beatsPerStep));
                DEBUG_TIMING("Transport", ppqPosition, hostBPM, targetStep, -1);
                
                // Update debug display for UI
                debugInfo = juce::String::formatted(
                    "PPQ: %.2f | BPM: %.1f | Step: %d/%d | Beat/Step: %.3f",
                    ppqPosition, hostBPM, targetStep + 1, patternSteps, beatsPerStep
                );
            }
            
            // Trigger notes at the exact moment they should occur
            int numSamples = buffer.getNumSamples();
            double samplesPerBeat = getSampleRate() * 60.0 / hostBPM;
            
            // Track the last processed step to detect boundaries
            static int lastProcessedStep = -1;
            
            for (int sample = 0; sample < numSamples; ++sample)
            {
                // Calculate the ppqPosition for this sample
                double sampleBeat = currentBeat + (sample / samplesPerBeat);
                double sampleStepsFromStart = sampleBeat / beatsPerStep;
                int sampleStep = static_cast<int>(sampleStepsFromStart) % patternSteps;
                
                // Check if we're crossing a step boundary
                if (sampleStep != lastProcessedStep)
                {
                    lastProcessedStep = sampleStep;
                    
                    // Log step triggers
                    DEBUG_TIMING("StepTrigger", sampleBeat, hostBPM, sampleStep, sample);
                    
                    // Trigger the step at this exact sample position
                    processPatternStep(midiMessages, sample, sampleStep);
                    
                    // Update current step
                    currentStep = sampleStep;
                }
            }
        }
        else if (tickTriggered)
        {
            // Manual trigger - use free-running counter
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                sampleCounter += 1.0;
                
                if (sampleCounter >= samplesPerStep)
                {
                    sampleCounter -= samplesPerStep;
                    processPatternStep(midiMessages, sample);
                    
                    currentStep = (currentStep + 1) % currentPattern.size();
                }
            }
        }
    }
    
    // Reset when transport stops
    static bool wasPlaying = false;
    if (wasPlaying && !isPlaying)
    {
        // Just stopped playing - reset position
        currentStep = 0;
        sampleCounter = 0.0;
    }
    wasPlaying = isPlaying;
    
    // Store incoming MIDI for scene advancement processing - throttled to prevent rapid triggering
    static juce::uint32 lastMidiTriggerTime = 0;
    juce::uint32 currentTime = juce::Time::getMillisecondCounter();
    
    // Process incoming MIDI for scene changes, but don't clear the buffer yet
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() && (currentTime - lastMidiTriggerTime > 500))  // 500ms throttle
        {
            // MIDI input triggers scene advancement (like desktop version)
            if (currentUPIInput.contains("|") && sceneManager && sceneManager->hasScenes())
            {
                advanceScene();
                applyCurrentScenePattern();
                debugInfo = "MIDI triggered Scene " + juce::String(sceneManager->getCurrentSceneIndex() + 1) + 
                           " - " + sceneManager->getCurrentScenePattern();
                
                // Reset position for new scene
                currentStep = 0;
                sampleCounter = 0.0;
                lastMidiTriggerTime = currentTime;
                break;  // Only process first note in buffer
            }
        }
    }
    
    // Clear input MIDI to prevent passthrough, then add our generated notes
    midiMessages.clear();
    
    // Get any queued MIDI messages from UI interactions and pattern generation
    juce::MidiBuffer collectedMidi;
    midiCollector.removeNextBlockOfMessages(collectedMidi, buffer.getNumSamples());
    
    // Add our generated MIDI to the output
    midiMessages.addEvents(collectedMidi, 0, buffer.getNumSamples(), 0);
}

//==============================================================================
bool RhythmPatternExplorerAudioProcessor::hasEditor() const
{
    return true; // Set to false if you don't want a UI
}

juce::AudioProcessorEditor* RhythmPatternExplorerAudioProcessor::createEditor()
{
    return new RhythmPatternExplorerAudioProcessorEditor (*this);
}

//==============================================================================
void RhythmPatternExplorerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Store your plugin's state here
    // You can save parameters, settings, presets, etc.
    
    // Example using XML:
    // auto xml = std::make_unique<juce::XmlElement>("PluginState");
    // xml->setAttribute("someParameter", someParameterValue);
    // copyXmlToBinary(*xml, destData);
    
    juce::ignoreUnused (destData);
}

void RhythmPatternExplorerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
// RPE Pattern Methods
void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiString)
{
    currentUPIInput = upiString;
    
    // Store original UPI input if it contains progressive/scene syntax (desktop-compatible)
    bool hasProgressiveTransformation = upiString.contains(">");
    bool hasScenes = upiString.contains("|");
    if (hasProgressiveTransformation || hasScenes) {
        originalUPIInput = upiString;
    } else {
        originalUPIInput.clear(); // Clear if no progressive syntax
    }
    
    // Check if this is a scene pattern
    if (hasScenes && sceneManager)
    {
        // Parse scene pattern into individual scenes
        juce::StringArray scenes;
        scenes.addTokens(upiString, "|", "");
        
        // Initialize scene manager with the parsed scenes (starts at scene 0)
        sceneManager->initializeScenes(scenes);
        
        // Apply the first scene pattern directly without calling applyCurrentScenePattern
        // to avoid any potential advancement
        auto firstScenePattern = sceneManager->getCurrentScenePattern();
        if (!firstScenePattern.isEmpty())
        {
            auto parseResult = UPIParser::parse(firstScenePattern);
            if (parseResult.isValid())
            {
                currentPattern = parseResult.pattern;
            }
        }
        
        debugInfo = "Initialized " + juce::String(scenes.size()) + " scenes. Starting at Scene 1 - " + 
                   sceneManager->getCurrentScenePattern();
    }
    else if (hasProgressiveTransformation && progressiveManager)
    {
        // Progressive transformation pattern - parse and initialize state
        auto parseResult = UPIParser::parse(upiString);
        if (parseResult.isValid())
        {
            currentPattern = parseResult.pattern;
            // Initialize progressive state
            progressiveManager->resetProgressiveOffset(upiString);
            debugInfo = "Initialized progressive transformation: " + upiString;
        }
    }
    else
    {
        // Regular pattern processing
        auto parseResult = UPIParser::parse(upiString);
        if (parseResult.isValid())
        {
            currentPattern = parseResult.pattern;
        }
        else
        {
            // Keep existing pattern if parsing fails
            // Could also set an error pattern or log the error
        }
    }
    
    currentStep = 0;
    sampleCounter = 0.0;
    updateTiming(); // Use default parameters when no position info available
}

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty()) return;
    
    // Parse the UPI pattern
    auto parseResult = UPIParser::parse(upiPattern);
    if (parseResult.isValid())
    {
        // Update current pattern
        currentPattern = parseResult.pattern;
        
        // Update timing for new pattern
        updateTiming();
        
        // Reset step counter if requested
        if (resetAccentPosition)
        {
            currentStep = 0;
            sampleCounter = 0.0;
        }
    }
}

//==============================================================================
// Scene Management Functions (simplified for iPad)

void RhythmPatternExplorerAudioProcessor::advanceScene()
{
    // Use SceneManager if available
    if (sceneManager && sceneManager->hasScenes()) 
    {
        sceneManager->advanceScene();
        // Don't update debugInfo here - let the caller do it with pattern info
    }
}

void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // Use SceneManager to get current scene pattern
    if (sceneManager && sceneManager->hasScenes())
    {
        auto scenePattern = sceneManager->getCurrentScenePattern();
        int sceneIndex = sceneManager->getCurrentSceneIndex();
        
        if (!scenePattern.isEmpty())
        {
            // Apply the current scene pattern
            auto parseResult = UPIParser::parse(scenePattern);
            if (parseResult.isValid())
            {
                currentPattern = parseResult.pattern;
                // Don't update debugInfo here - let the caller provide context
            }
        }
    }
}

void RhythmPatternExplorerAudioProcessor::updateTiming(juce::Optional<juce::AudioPlayHead::PositionInfo> position)
{
    if (currentPattern.empty() || currentSampleRate <= 0)
    {
        samplesPerStep = 0.0;
        return;
    }
    
    // Use host BPM if available, otherwise default to 120 BPM
    double bpm = 120.0;
    if (position.hasValue())
    {
        bpm = position->getBpm().orFallback(120.0);
    }
    
    // Calculate samples per 16th note step
    double beatsPerSecond = bpm / 60.0;
    double secondsPerBeat = 1.0 / beatsPerSecond;
    double secondsPerSixteenthNote = secondsPerBeat / 4.0;
    
    samplesPerStep = secondsPerSixteenthNote * currentSampleRate;
}

void RhythmPatternExplorerAudioProcessor::processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber, int stepToProcess)
{
    if (stepToProcess >= currentPattern.size()) return;
    
    // Send MIDI note if this step is active
    if (currentPattern[stepToProcess])
    {
        int noteNumber = midiNoteParam->get();
        float velocity = 0.8f;
        int channel = 1;
        
        // Log MIDI events
        DEBUG_MIDI("NoteOn", noteNumber, velocity, sampleNumber);
        
        // Create note on message and add to collector (not directly to midiMessages)
        juce::MidiMessage noteOn = juce::MidiMessage::noteOn(channel, noteNumber, velocity);
        juce::MidiMessage noteOff = juce::MidiMessage::noteOff(channel, noteNumber, 0.0f);
        
        midiCollector.addMessageToQueue(noteOn);
        // Note off after short duration
        juce::Timer::callAfterDelay(50, [this, noteOff]() {
            midiCollector.addMessageToQueue(noteOff);
        });
    }
}

void RhythmPatternExplorerAudioProcessor::processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber)
{
    // Legacy method for manual triggers
    processPatternStep(midiMessages, sampleNumber, currentStep);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}