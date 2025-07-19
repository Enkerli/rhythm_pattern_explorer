/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Implementation (Fixed)

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UPIParser.h"
#include <ctime>
#include <fstream>
#include <chrono>
#include <iomanip>

// Debug output disabled for production performance
#ifdef DEBUG
    // Keep debug output in debug builds
#else
    #undef DBG
#endif

//==============================================================================
/**
 * Constructor for the main audio processor.
 * 
 * Initializes all parameters, sets up the pattern engine with a default Euclidean
 * pattern, and configures the UPI parser with the pattern engine reference.
 * Only exposes essential parameters to the host DAW interface.
 */
RhythmPatternExplorerAudioProcessor::RhythmPatternExplorerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), randomGenerator(std::random_device{}())
#endif
{
    // Initialize parameters - only expose essential ones to host
    addParameter(useHostTransportParam = new juce::AudioParameterBool("useHostTransport", "Use Host Transport", true));
    addParameter(midiNoteParam = new juce::AudioParameterInt("midiNote", "MIDI Note", 0, 127, 36));
    addParameter(tickParam = new juce::AudioParameterBool("tick", "Tick", false));
    addParameter(accentPitchOffsetParam = new juce::AudioParameterInt("accentPitchOffset", "Accent Pitch Offset", -12, 12, 0));
    addParameter(accentVelocityParam = new juce::AudioParameterFloat("accentVelocity", "Accent Velocity", 0.0f, 1.0f, 1.0f));
    addParameter(unaccentedVelocityParam = new juce::AudioParameterFloat("unaccentedVelocity", "Unaccented Velocity", 0.0f, 1.0f, 0.69f));
    
    // Pattern Length parameters for Phase 2 temporal control
    addParameter(patternLengthUnitParam = new juce::AudioParameterChoice("patternLengthUnit", "Pattern Length Unit", 
        juce::StringArray{"Steps", "Beats", "Bars"}, 1)); // Default to "Beats"
    addParameter(patternLengthValueParam = new juce::AudioParameterChoice("patternLengthValue", "Pattern Length Value", 
        juce::StringArray{"0.125", "0.25", "0.5", "0.75", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32"}, 7)); // Default to "8"
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    // Set up progressive offset engine for UPI parser
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    // Plugin initialized successfully
}


/**
 * Destructor for the audio processor.
 * 
 * Cleanup is handled automatically by JUCE and member destructors.
 */
RhythmPatternExplorerAudioProcessor::~RhythmPatternExplorerAudioProcessor()
{
}

//==============================================================================
/**
 * Returns the plugin name.
 * 
 * @return The plugin name as defined in JucePlugin_Name
 */
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
/**
 * Prepares the audio processor for playback.
 * 
 * Called by the host before audio processing begins. Sets up sample rate,
 * resets sequencer state, and calculates initial timing parameters.
 * 
 * @param sampleRate The sample rate for audio processing
 * @param samplesPerBlock The number of samples per audio buffer
 */
void RhythmPatternExplorerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    // MIDI effect mode - minimal audio setup
    // Reset sequencer state
    currentSample = 0;
    currentStep = 0;
    globalAccentPosition = 0;
    wasPlaying = false;
    
    // Force early initialization to work around Logic Pro loading order issues
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    updateTiming();
    
    // Audio system prepared successfully
}

/**
 * Releases audio processing resources.
 * 
 * Called by the host when audio processing is stopped. In this MIDI effect
 * plugin, minimal cleanup is required as we don't use audio synthesis.
 */
void RhythmPatternExplorerAudioProcessor::releaseResources()
{
    // Audio resources released
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

/**
 * Main audio processing function.
 * 
 * This is the core of the plugin, called by the host for each audio buffer.
 * Handles:
 * - MIDI input processing and pattern triggering
 * - Host transport synchronization
 * - Step-based pattern playback with precise timing
 * - Accent pattern application
 * - Parameter updates and timing calculations
 * 
 * @param buffer Audio buffer (cleared for MIDI effect mode)
 * @param midiMessages MIDI input/output buffer
 */
void RhythmPatternExplorerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Handle tick parameter (equivalent to pressing Parse)
    bool currentTickState = tickParam ? tickParam->get() : false;
    if (currentTickState && !lastTickState) {
        // Tick edge detected - reparse current UPI pattern
        // Tick parameter activated - trigger pattern update
        if (!currentUPIInput.isEmpty()) {
            setUPIInput(currentUPIInput);
        }
        tickResetCounter = 1; // Start reset counter
    }
    
    // Reset tick parameter after a brief delay to allow for multiple ticks
    if (tickResetCounter > 0) {
        tickResetCounter++;
        if (tickResetCounter >= 20) { // Reset after ~20 process blocks (~1-2ms)
            tickParam->setValueNotifyingHost(0.0f);
            tickResetCounter = 0;
        }
    }
    lastTickState = currentTickState;
    
    // FIRST: Always log that processBlock is being called
    static int processBlockCallCount = 0;
    processBlockCallCount++;
    
    // Update last process time to indicate we're receiving audio callbacks
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
    // Validate buffer size
    int currentBufferSize = buffer.getNumSamples();
    if (currentBufferSize <= 0) {
        return;
    }
    
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
        
        // Transport sync with host
    }
    else
    {
        // No playhead available
    }

    // Determine if we should be playing
    bool isPlaying;
    if (useHostTransportParam && useHostTransportParam->get() && hasValidPosition)
    {
        // FIXED: Manual play button should work alongside host transport (OR logic)
        isPlaying = posInfo.isPlaying || internalPlaying;
        
        // BITWIG FIX: Always sync BPM, but conditionally sync position
        syncBPMWithHost(posInfo);
        syncPositionWithHost(posInfo);
    }
    else
    {
        isPlaying = internalPlaying;
    }

    // Pattern updates are now handled via UPI input only
    
    // Update timing if pattern length parameters changed
    static int lastPatternLengthUnit = 1; // Default to Beats
    static float lastPatternLengthValue = 8.0f;
    bool patternLengthChanged = false;
    
    int currentPatternLengthUnit = patternLengthUnitParam->getIndex();
    float currentPatternLengthValue = getPatternLengthValue();
    
    if (currentPatternLengthUnit != lastPatternLengthUnit || 
        std::abs(currentPatternLengthValue - lastPatternLengthValue) > 0.001f) {
        patternLengthChanged = true;
        lastPatternLengthUnit = currentPatternLengthUnit;
        lastPatternLengthValue = currentPatternLengthValue;
    }
    
    // Update timing if BPM or pattern length changed - but preserve currentSample ratio
    static float lastBPM = 120.0f;
    bool bpmChanged = std::abs(currentBPM - lastBPM) > 0.1f;
    
    if (bpmChanged || patternLengthChanged) {
        // BITWIG FIX: Preserve timing position when BPM changes
        double sampleRatio = (samplesPerStep > 0) ? (double)currentSample / samplesPerStep : 0.0;
        updateTiming();
        currentSample = static_cast<int>(sampleRatio * samplesPerStep);
        
        // BPM change processed
    }
    
    // Always update lastBPM outside the if block
    if (bpmChanged) {
        lastBPM = currentBPM;
    }
    
    // Playing state determined

    // FIRST STEP FIX: Track if we've processed step 0 in this buffer
    static bool processedFirstStepThisBuffer = false;
    if (!isPlaying) processedFirstStepThisBuffer = false; // Reset when not playing

    // TRANSPORT-SYNCED TIMING: Use DAW's ppqPosition for perfect alignment
    if (isPlaying && hasValidPosition)
    {
        // Calculate pattern timing based on DAW transport and pattern length parameters
        int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars
        float lengthValue = getPatternLengthValue();
        
        // Calculate pattern length in beats using pattern length parameters
        double patternLengthInBeats;
        switch (lengthUnit) {
            case 0: // Steps mode - use 16th note subdivisions
                patternLengthInBeats = lengthValue / 4.0; // Steps are 16th notes
                break;
            case 1: // Beats mode 
                patternLengthInBeats = lengthValue; 
                break;
            case 2: // Bars mode
                patternLengthInBeats = lengthValue * 4.0; // Assume 4/4 time
                break;
            default:
                patternLengthInBeats = lengthValue; // Default to beats
                break;
        }
        
        // Calculate beats per step based on pattern length parameters
        double beatsPerStep = patternLengthInBeats / patternEngine.getStepCount();
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        // Calculate which step should be active
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // Calculate the exact fractional position within the current step
        double stepFraction = stepsFromStart - std::floor(stepsFromStart);
        
        // Trigger notes at the exact moment they should occur
        int numSamples = buffer.getNumSamples();
        double samplesPerBeat = getSampleRate() * 60.0 / posInfo.bpm;
        double samplesPerStep = samplesPerBeat * beatsPerStep;
        
        // Track the last processed step to detect boundaries
        static int lastProcessedStep = -1;
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Calculate step using transport position for precision, but override if manually reset
            int targetStep;
            int totalSamples = currentSample + sample;
            if (totalSamples < samplesPerStep) {
                // We're still in the first step after a reset - force step 0
                targetStep = 0;
            } else {
                // Use transport timing for precision
                double sampleStepsFromStart = stepsFromStart + (sample / static_cast<double>(samplesPerStep));
                targetStep = static_cast<int>(sampleStepsFromStart) % patternEngine.getStepCount();
            }
            
            // Check if we're crossing a step boundary (only trigger once per step)
            if (targetStep != lastProcessedStep)
            {
                lastProcessedStep = targetStep;
                
                // Step boundary detected
                
                // CYCLE BOUNDARY UPDATE: Sync UI accent offset with MIDI at rhythm cycle boundaries  
                // CRITICAL PROTECTION: This prevents the second most common bug - accent markers 
                // sticking on first cycle for polymetric patterns like {10}E(3,8).
                // 
                // CRITICAL TIMING: Must happen BEFORE processStep() so UI uses the accent position
                // that matches the cycle we're about to play, not the position after advancement.
                if (hasAccentPattern && !currentAccentPattern.empty() && targetStep == 0) {
                    // At cycle boundary (step 0), capture both offset and reference position for current cycle
                    int oldUiOffset = uiAccentOffset;
                    uiAccentOffset = globalAccentPosition % currentAccentPattern.size();
                    uiAccentStartPosition = globalAccentPosition; // Capture global reference point
                    patternChanged.store(true); // Trigger UI update with current cycle offset
                    
                    // UI now uses same reference point as MIDI for this cycle
                }
                
                // Trigger the step at this exact sample position
                processStep(midiMessages, sample, targetStep);
                
                // Update current step
                currentStep.store(targetStep);
                
                // Transport-synced step progression completed
            }
        }
    }
    
    // TRANSPORT-AWARE RESET LOGIC: Handle transport start/stop for accent synchronization
    if (wasPlaying && !isPlaying)
    {
        // Just stopped playing - reset position and accent tracking
        currentSample = 0;
        currentStep.store(0);
        globalAccentPosition = 0;
        uiAccentOffset = 0;
        uiAccentStartPosition = 0;  // Reset UI accent offset on stop
        patternChanged.store(true);  // Notify UI to update
        // Stopped playing - resetting position and accent tracking
    }
    else if (!wasPlaying && isPlaying)
    {
        // Just started playing - minimal handling to avoid double initialization
        // The accent positions should already be correct from pattern loading
        // Started playing - letting existing positions remain
    }
    
    wasPlaying = isPlaying;
    
    // CRITICAL: Increment currentSample for pattern advancement
    currentSample += buffer.getNumSamples();
}

//==============================================================================
/**
 * Indicates whether this plugin has a graphical editor.
 * 
 * @return true as this plugin provides a GUI
 */
bool RhythmPatternExplorerAudioProcessor::hasEditor() const
{
    return true;
}

/**
 * Creates the plugin editor interface.
 * 
 * @return New instance of the plugin editor
 */
juce::AudioProcessorEditor* RhythmPatternExplorerAudioProcessor::createEditor()
{
    return new RhythmPatternExplorerAudioProcessorEditor (*this);
}

//==============================================================================
/**
 * Saves the plugin state to a memory block.
 * 
 * Serializes the current plugin state including BPM, transport settings,
 * MIDI note, pattern data, and UPI input for host session persistence.
 * 
 * @param destData Memory block to store the serialized state
 */
void RhythmPatternExplorerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Create an XML representation of our state
    auto xml = std::make_unique<juce::XmlElement>("RhythmPatternExplorerState");
    
    xml->setAttribute("bpm", currentBPM);
    xml->setAttribute("playing", internalPlaying);
    xml->setAttribute("useHostTransport", useHostTransportParam->get());
    xml->setAttribute("midiNote", midiNoteParam->get());
    
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

/**
 * Restores the plugin state from a memory block.
 * 
 * Deserializes the plugin state from host session data and restores
 * all settings including BPM, transport settings, pattern data, and UPI input.
 * 
 * @param data Pointer to the serialized state data
 * @param sizeInBytes Size of the state data in bytes
 */
void RhythmPatternExplorerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    
    if (xml != nullptr && xml->hasTagName("RhythmPatternExplorerState"))
    {
        currentBPM = xml->getDoubleAttribute("bpm", 120.0);
        internalPlaying = xml->getBoolAttribute("playing", false);
        *useHostTransportParam = xml->getBoolAttribute("useHostTransport", true);
        *midiNoteParam = xml->getIntAttribute("midiNote", 36);
        
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
/**
 * Updates timing calculations based on current BPM and pattern length parameters.
 * 
 * Calculates samples per step based on:
 * - Current BPM (from host or internal)
 * - Pattern length unit (Steps, Beats, or Bars)
 * - Pattern length value
 * - Current sample rate
 * 
 * Includes validation for high BPM scenarios to prevent timing issues.
 */
void RhythmPatternExplorerAudioProcessor::updateTiming()
{
    float bpm = currentBPM;
    
    // Get pattern length parameters for Phase 2 temporal control
    int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars
    float lengthValue = getPatternLengthValue();
    
    // Calculate samples per step using pattern length parameters
    double beatsPerSecond = bpm / 60.0;
    double patternLengthInBeats;
    
    switch (lengthUnit) {
        case 0: // Steps mode - use 16th note subdivisions (default behavior)
            patternLengthInBeats = lengthValue / 4.0; // Steps are 16th notes
            break;
        case 1: // Beats mode 
            patternLengthInBeats = lengthValue; 
            break;
        case 2: // Bars mode
            patternLengthInBeats = lengthValue * 4.0; // Assume 4/4 time
            break;
        default:
            patternLengthInBeats = lengthValue; // Default to beats
            break;
    }
    
    // Calculate timing for entire pattern, then divide by number of steps
    auto pattern = patternEngine.getCurrentPattern();
    int patternSteps = static_cast<int>(pattern.size());
    if (patternSteps <= 0) patternSteps = 8; // Fallback
    
    double patternDurationInSeconds = patternLengthInBeats / beatsPerSecond;
    double stepDurationInSeconds = patternDurationInSeconds / patternSteps;
    samplesPerStep = static_cast<int>(currentSampleRate * stepDurationInSeconds);
    
    // HIGH BPM FIX: Enhanced timing validation for Bitwig testing
    if (samplesPerStep <= 0) {
        samplesPerStep = static_cast<int>(currentSampleRate / 60.0); // Default to 1Hz fallback
    }
        
    // Timing updated based on current BPM
    
    // Timing analysis completed
}

/**
 * Processes a single step in the pattern sequence.
 * 
 * Checks if the current step should trigger a note based on the pattern,
 * calculates accent status using the global accent position, and triggers
 * the appropriate MIDI note if an onset is present.
 * 
 * @param midiBuffer MIDI buffer to add note events to
 * @param samplePosition Sample position within the current buffer
 * @param stepToProcess The step index to process (0-based)
 */
void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    // Pattern state validated for processing
    
    // Pattern check completed
    
    if (stepToProcess < pattern.size() && pattern[stepToProcess])
    {
        // Check if this step should be accented
        bool shouldAccent = false;
        if (hasAccentPattern && !currentAccentPattern.empty())
        {
            // SIMPLE SEQUENTIAL APPROACH: Use globalAccentPosition for reliable accent cycling
            // This approach works correctly and produces accurate MIDI accent patterns
            int accentStep = globalAccentPosition % currentAccentPattern.size();
            shouldAccent = currentAccentPattern[accentStep];
            
            // DEBUG LOGGING: Track MIDI accent output
            {
                std::ofstream logFile("/tmp/accent_debug.log", std::ios::app);
                if (logFile.is_open()) {
                    auto now = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(now);
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                    
                    logFile << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "." 
                           << std::setfill('0') << std::setw(3) << ms.count() << "] "
                           << "MIDI: step=" << stepToProcess 
                           << " globalPos=" << globalAccentPosition 
                           << " accentStep=" << accentStep 
                           << " shouldAccent=" << (shouldAccent ? "YES" : "NO")
                           << " uiOffset=" << uiAccentOffset
                           << " currentStep=" << currentStep.load()
                           << std::endl;
                    logFile.close();
                }
            }
            
            // Increment global accent position for next onset
            globalAccentPosition++;
            
            // Accent calculated based on sequential position
        }
        else
        {
            // No accent pattern active
        }
        
        // Note triggered with accent calculation
        
        triggerNote(midiBuffer, samplePosition, shouldAccent);
    }
}

/**
 * Triggers a MIDI note with appropriate velocity and pitch based on accent status.
 * 
 * Applies accent modulation using the accent parameters:
 * - Accented notes: use accent velocity and pitch offset
 * - Unaccented notes: use standard velocity
 * 
 * @param midiBuffer MIDI buffer to add note events to
 * @param samplePosition Sample position for note timing
 * @param isAccented Whether this note should be accented
 */
void RhythmPatternExplorerAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented)
{
    // Get base MIDI note number from parameter (set by incoming MIDI)
    int baseNoteNumber = midiNoteParam ? midiNoteParam->get() : 36; // Default kick drum note
    
    // Apply accent modulation based on parameters
    int noteNumber = baseNoteNumber;
    float velocity;
    
    if (isAccented)
    {
        // Apply accent pitch offset from parameter (default 0)
        int pitchOffset = accentPitchOffsetParam ? accentPitchOffsetParam->get() : 0;
        noteNumber = juce::jlimit(0, 127, baseNoteNumber + pitchOffset); // Clamped to MIDI range
        
        // Apply accent velocity from parameter (default 1.0)
        velocity = accentVelocityParam ? accentVelocityParam->get() : 1.0f;
    }
    else
    {
        // Apply unaccented velocity from parameter (default 0.69)
        velocity = unaccentedVelocityParam ? unaccentedVelocityParam->get() : 0.69f;
    }
    
    // Send MIDI note
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, velocity);
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
    
    midiBuffer.addEvent(noteOn, samplePosition);
    midiBuffer.addEvent(noteOff, samplePosition + 100); // 100 samples note duration
    
    // MIDI effect mode - no audio synthesis
    
    // MIDI note triggered with calculated velocity and accent status
}

/**
 * Synchronizes internal BPM with the host DAW's tempo.
 * 
 * Always updates the internal BPM to match the host when valid tempo
 * information is available. This ensures the pattern timing stays in sync
 * with the DAW's tempo changes.
 * 
 * @param posInfo Current position information from the host
 */
void RhythmPatternExplorerAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // ALWAYS sync BPM - this should never be disabled
    if (posInfo.bpm > 0)
    {
        float hostBPM = static_cast<float>(posInfo.bpm);
        float currentBPMValue = currentBPM;
        
        // BPM sync validation
        
        // Update our internal BPM to match host
        if (std::abs(currentBPMValue - hostBPM) > 0.1f)
        {
            currentBPM = hostBPM;
            updateTiming();
            
            // BPM updated successfully
        }
    }
}

/**
 * Synchronizes pattern position with the host DAW's transport position.
 * 
 * Calculates the appropriate step position based on the host's ppqPosition
 * and pattern length parameters. Only syncs when significantly out of sync
 * to avoid constant position adjustments.
 * 
 * @param posInfo Current position information from the host
 */
void RhythmPatternExplorerAudioProcessor::syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Calculate pattern position based on host timeline
    if (posInfo.ppqPosition >= 0.0)
    {
        // Calculate which step we should be on based on timeline position
        // For tresillo: 8 steps over 4 beats = 0.5 beats per step
        double beatsPerStep = 4.0 / patternEngine.getStepCount(); // Dynamic based on pattern length
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // DISABLE TRANSPORT SYNC: Use simple sample-based timing to respect manual resets
        bool allowPositionSync = false;  // DISABLED: Transport sync interferes with manual triggers
        
        // CRITICAL FIX: Only sync position when significantly out of sync (not every block)
        int stepDifference = std::abs(targetStep - currentStep.load());
        if (allowPositionSync && stepDifference > 2)  // Increased threshold from 1 to 2
        {
            currentStep.store(targetStep);
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
            // Position synchronized with host transport
        }
        else if (!allowPositionSync && currentBPM >= 180.0f)
        {
            // Position sync disabled at high BPM for stability
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

/**
 * Sets the UPI (Universal Pattern Interface) input and processes it.
 * 
 * This is the main entry point for pattern input. Handles:
 * - Scene cycling (patterns separated by |)
 * - Progressive offset patterns (pattern+N)
 * - Progressive lengthening patterns (pattern*N)
 * - Regular patterns
 * 
 * Resets accent position and step indicator when processing new patterns.
 * 
 * @param upiPattern The UPI pattern string to process
 */
void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Reset global accent position when setting new UPI input
    globalAccentPosition = 0;
    uiAccentOffset = 0;
    
    // UPI pattern input received for processing
    
    // Check for progressive syntax: scenes first (pattern|pattern|pattern), then pattern+N (offset), pattern*N (lengthening)
    juce::String pattern = upiPattern.trim();
    bool isScenes = pattern.contains("|");
    
    // Check if + is followed by a number (progressive offset) vs pattern (combination)
    bool isProgressiveOffset = false;
    if (!isScenes && pattern.contains("+") && pattern.lastIndexOf("+") > 0) {
        int lastPlusIndex = pattern.lastIndexOf("+");
        juce::String afterPlus = pattern.substring(lastPlusIndex + 1).trim();
        // Progressive offset if what follows + is purely numeric
        isProgressiveOffset = afterPlus.containsOnly("0123456789");
    }
    
    bool isProgressiveLengthening = !isScenes && pattern.contains("*") && pattern.lastIndexOf("*") > 0;
    
    if (isScenes)
    {
        // Handle scene cycling: pattern1|pattern2|pattern3
        // CRITICAL: Split scenes BEFORE any UPI parsing to preserve individual accent patterns
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // Log scene cycling detection
        juce::String sceneList;
        for (int i = 0; i < scenes.size(); ++i) {
            sceneList += juce::String::formatted("  Scene %d: %s\n", i, scenes[i].trim().toRawUTF8());
        }
        
        // Check if this is the same scene sequence or a new one
        bool isSameSequence = (scenes.size() == scenePatterns.size());
        if (isSameSequence) {
            for (int i = 0; i < scenes.size(); ++i) {
                if (scenes[i].trim() != scenePatterns[i]) {
                    isSameSequence = false;
                    break;
                }
            }
        }
        
        if (isSameSequence && !scenePatterns.isEmpty())
        {
            // Same sequence - advance to next scene
            advanceScene();
            
            // Advanced to next scene
        }
        else
        {
            // New scene sequence - reset and initialize per-scene progressive state
            scenePatterns.clear();
            sceneProgressiveOffsets.clear();
            sceneProgressiveSteps.clear();
            sceneBasePatterns.clear();
            sceneProgressiveLengthening.clear();
            sceneBaseLengthPatterns.clear();
            
            // Parse each scene and initialize its progressive state  
            // CRITICAL: Store original scene patterns with accent syntax intact
            for (const auto& scene : scenes) {
                juce::String scenePattern = scene.trim();
                scenePatterns.add(scenePattern);
                
                // Check if this scene has progressive syntax
                bool hasProgressiveOffset = false;
                if (scenePattern.contains("+") && scenePattern.lastIndexOf("+") > 0) {
                    int lastPlusIndex = scenePattern.lastIndexOf("+");
                    juce::String afterPlus = scenePattern.substring(lastPlusIndex + 1).trim();
                    hasProgressiveOffset = afterPlus.containsOnly("0123456789");
                }
                bool hasProgressiveLengthening = scenePattern.contains("*") && scenePattern.lastIndexOf("*") > 0;
                
                if (hasProgressiveOffset) {
                    // Parse offset syntax: pattern+N
                    int plusIndex = scenePattern.lastIndexOf("+");
                    juce::String basePattern = scenePattern.substring(0, plusIndex).trim();
                    juce::String offsetStr = scenePattern.substring(plusIndex + 1).trim();
                    int step = offsetStr.getIntValue();
                    
                    sceneBasePatterns.push_back(basePattern);
                    sceneProgressiveSteps.push_back(step);
                    sceneProgressiveOffsets.push_back(step); // Start with first offset
                    sceneProgressiveLengthening.push_back(0);
                    sceneBaseLengthPatterns.push_back(std::vector<bool>());
                } else if (hasProgressiveLengthening) {
                    // Parse lengthening syntax: pattern*N
                    int starIndex = scenePattern.lastIndexOf("*");
                    juce::String basePattern = scenePattern.substring(0, starIndex).trim();
                    juce::String lengthStr = scenePattern.substring(starIndex + 1).trim();
                    int lengthStep = lengthStr.getIntValue();
                    
                    sceneBasePatterns.push_back(basePattern);
                    sceneProgressiveSteps.push_back(lengthStep);
                    sceneProgressiveOffsets.push_back(0);
                    sceneProgressiveLengthening.push_back(lengthStep); // Start with first lengthening
                    sceneBaseLengthPatterns.push_back(std::vector<bool>()); // Will be filled when pattern is generated
                } else {
                    // Simple pattern without progressive syntax
                    sceneBasePatterns.push_back(scenePattern);
                    sceneProgressiveSteps.push_back(0);
                    sceneProgressiveOffsets.push_back(0);
                    sceneProgressiveLengthening.push_back(0);
                    sceneBaseLengthPatterns.push_back(std::vector<bool>());
                }
            }
            
            currentSceneIndex = 0;
            
            // Debug log reset
            juce::String scenesInfo = "New scene sequence - parsed " + juce::String(scenePatterns.size()) + " scenes, starting with scene 0: " + scenePatterns[0];
            for (int i = 0; i < scenePatterns.size(); ++i) {
                scenesInfo += "\n  Scene " + juce::String(i) + ": " + scenePatterns[i] + " (base: " + sceneBasePatterns[i] + ", step: " + juce::String(sceneProgressiveSteps[i]) + ")";
            }
        }
        
        // Parse and apply the current scene pattern using per-scene progressive state
        if (!scenePatterns.isEmpty() && currentSceneIndex < scenePatterns.size()) {
            applyCurrentScenePattern();
        }
    }
    else if (isProgressiveOffset)
    {
        // Handle progressive offset: pattern+N
        int plusIndex = pattern.lastIndexOf("+");
        juce::String newBasePattern = pattern.substring(0, plusIndex).trim();
        juce::String stepStr = pattern.substring(plusIndex + 1).trim();
        int newStep = stepStr.getIntValue();
        
        // Progressive offset processing
        
        // If same base pattern, advance offset; if different, reset
        if (basePattern == newBasePattern && progressiveStep == newStep)
        {
            advanceProgressiveOffset();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
            // Progressive offset advanced
        }
        else
        {
            // New progressive pattern - reset and start
            basePattern = newBasePattern;
            progressiveStep = newStep;
            progressiveOffset = newStep; // Start with first offset
            
            // Progressive pattern reset
        }
        
        // Parse the base pattern first, then apply progressive offset via rotation
        parseAndApplyUPI(basePattern);
        
        // Apply progressive offset by rotating the generated pattern
        if (progressiveOffset != 0)
        {
            auto currentPattern = patternEngine.getCurrentPattern();
            auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
            
            // Pattern rotation applied
        }
    }
    else if (isProgressiveLengthening)
    {
        // Handle progressive lengthening: pattern*N
        int starIndex = pattern.lastIndexOf("*");
        juce::String newBasePattern = pattern.substring(0, starIndex).trim();
        juce::String lengthStr = pattern.substring(starIndex + 1).trim();
        int newLengthening = lengthStr.getIntValue();
        
        // Progressive lengthening processing
        
        // If same base pattern, advance lengthening; if different, reset
        if (basePattern == newBasePattern && progressiveLengthening == newLengthening)
        {
            advanceProgressiveLengthening();
            
            // Notify UI that pattern has changed for accent map updates
            patternChanged.store(true);
            
            // Progressive lengthening advanced
        }
        else
        {
            // New progressive lengthening pattern - reset and start
            basePattern = newBasePattern;
            progressiveLengthening = newLengthening;
            
            // Parse base pattern and store for lengthening
            parseAndApplyUPI(basePattern);
            baseLengthPattern = patternEngine.getCurrentPattern();
            
            // Progressive lengthening reset
        }
        
        // Apply current lengthened pattern
        patternEngine.setPattern(baseLengthPattern);
    }
    else
    {
        // Non-progressive pattern - reset progressive states
        progressiveOffset = 0;
        progressiveStep = 0;
        progressiveLengthening = 0;
        basePattern = "";
        baseLengthPattern.clear();
        scenePatterns.clear();
        currentSceneIndex = 0;
        parseAndApplyUPI(pattern, true); // This now resets currentStep internally
    }
    
    currentUPIInput = upiPattern;
}

/**
 * Parses and applies a UPI pattern to the pattern engine.
 * 
 * Lower-level function that handles the actual pattern parsing and application.
 * Supports progressive transformations, accent patterns, and various pattern types.
 * 
 * @param upiPattern The UPI pattern string to parse
 * @param resetAccentPosition Whether to reset the global accent position
 */
void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty())
        return;
    
    // FULL RESTART: Reset all playback state to step 0
    if (resetAccentPosition) {
        globalAccentPosition = 0;
        uiAccentOffset = 0;
        uiAccentStartPosition = 0;
        currentStep.store(0);
        currentSample = 0; // CRITICAL: Reset sample position for actual playback restart
        
    }
    
    // UI UPDATE: Always trigger UI update on pattern changes
    patternChanged.store(true);
    
    
    // For progressive patterns, always re-parse to advance state
    bool isProgressive = upiPattern.contains("#");
    
    // Check if this is a progressive transformation with ">" syntax
    bool isProgressiveTransformation = upiPattern.contains(">");
    if (isProgressiveTransformation) {
        // Store the pattern key for step tracking
        currentProgressivePatternKey = upiPattern;
    }
    
    auto parseResult = UPIParser::parse(upiPattern);
    
    if (parseResult.isValid())
    {
        
        // Apply the parsed pattern to the engine
        patternEngine.setPattern(parseResult.pattern);
        
        // Set up progressive offset if present
        if (parseResult.hasProgressiveOffset)
        {
            patternEngine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
                
            // Store progressive pattern key for step tracking
            if (!parseResult.progressivePatternKey.isEmpty())
            {
                currentProgressivePatternKey = parseResult.progressivePatternKey;
            }
        }
        else
        {
            patternEngine.setProgressiveOffset(false);
            currentProgressivePatternKey.clear(); // Clear for non-progressive patterns
        }
        
        // Set up accent pattern if present
        if (parseResult.hasAccentPattern)
        {
            hasAccentPattern = true;
            currentAccentPattern = parseResult.accentPattern;
            currentAccentPatternName = parseResult.accentPatternName;
        }
        else
        {
            hasAccentPattern = false;
            currentAccentPattern.clear();
            currentAccentPatternName.clear();
        }
        
        // Update parameters to reflect the new pattern
        int onsets = UPIParser::countOnsets(parseResult.pattern);
        int steps = static_cast<int>(parseResult.pattern.size());
        
        
        // Pattern applied successfully via UPI
        
        updateTiming();
        
        // Only cache non-progressive patterns to prevent re-parsing
        if (!isProgressive)
        {
            lastParsedUPI = upiPattern;
        }
        
        // UPI pattern successfully parsed and applied
    }
    else
    {
        // Failed to parse UPI pattern
    }
}

/**
 * Processes incoming MIDI messages for pattern triggers.
 * 
 * Handles:
 * - Note messages: captures note number for output and triggers pattern advancement
 * - CC messages: can trigger pattern regeneration for certain pattern types
 * - Progressive pattern advancement (manual triggering)
 * - Scene cycling advancement
 * 
 * Clears input MIDI to prevent passthrough (MIDI effect mode).
 * 
 * @param midiMessages MIDI input buffer to process
 */
void RhythmPatternExplorerAudioProcessor::checkMidiInputForTriggers(juce::MidiBuffer& midiMessages)
{
    // Store MIDI messages to process, then clear the buffer to prevent passthrough
    std::vector<juce::MidiMessage> messagesToProcess;
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        messagesToProcess.push_back(message);
    }
    
    // Clear the input buffer to prevent MIDI passthrough
    midiMessages.clear();
    
    // Process the stored messages
    for (const auto& message : messagesToProcess)
    {
        if (message.isNoteOn())
        {
            // Capture the MIDI note number and set the parameter
            int noteNumber = message.getNoteNumber();
            if (midiNoteParam)
            {
                midiNoteParam->setValueNotifyingHost(midiNoteParam->convertTo0to1((float)noteNumber));
            }
            
            // Any MIDI note input triggers pattern regeneration for progressive/random patterns
            if (!currentUPIInput.isEmpty())
            {
                // Check for progressive patterns and scenes
                bool hasProgressiveOffset = currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0;
                bool hasProgressiveLengthening = currentUPIInput.contains("*") && currentUPIInput.lastIndexOf("*") > 0;
                bool hasOldProgressiveOffset = currentUPIInput.contains("#");
                bool hasProgressiveTransformation = currentUPIInput.contains(">"); // New progressive syntax
                bool hasScenes = currentUPIInput.contains("|");
                
                // MIDI triggers manual advancement - handle progressive transformations and scenes together when both present
                bool triggerNeeded = false;
                
                if (hasProgressiveTransformation && !hasScenes)
                {
                    // Trigger progressive transformation manually with accent reset
                    parseAndApplyUPI(currentUPIInput, true);
                    triggerNeeded = true;
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset)
                {
                    if (hasProgressiveOffset || hasOldProgressiveOffset) {
                        if (patternEngine.hasProgressiveOffsetEnabled()) {
                            patternEngine.triggerProgressiveOffset();
                        }
                    }
                    if (hasProgressiveLengthening) {
                        advanceProgressiveLengthening();
                        patternEngine.setPattern(baseLengthPattern);
                    }
                    // FULL RESTART: All triggers reset everything to step 0
                    parseAndApplyUPI(currentUPIInput, true);
                    triggerNeeded = true;
                }
                
                // Handle scenes - can occur together with progressive transformations
                if (hasScenes)
                {
                    advanceScene();
                    applyCurrentScenePattern();
                    triggerNeeded = true;
                }
                
                if (!triggerNeeded)
                {
                    // Re-parse other UPI patterns to trigger regeneration with accent reset
                    parseAndApplyUPI(currentUPIInput, true);
                }
                
                // Reset step indicator and notify UI for any trigger
                currentStep.store(0);
                patternChanged.store(true);
            }
            // Pattern updates are handled via UPI only
        }
        else if (message.isController())
        {
            // CC messages trigger pattern updates but don't affect the MIDI note parameter
            if (!currentUPIInput.isEmpty())
            {
                // Check for progressive patterns and scenes
                bool hasProgressiveOffset = currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0;
                bool hasProgressiveLengthening = currentUPIInput.contains("*") && currentUPIInput.lastIndexOf("*") > 0;
                bool hasOldProgressiveOffset = currentUPIInput.contains("#");
                bool hasProgressiveTransformation = currentUPIInput.contains(">"); // New progressive syntax
                bool hasScenes = currentUPIInput.contains("|");
                
                // CYCLE BOUNDARY ADVANCEMENT: Progressive transformations and scenes now only advance at step 0
                // CC input no longer triggers mid-cycle advancement for better accent synchronization
                if (hasProgressiveTransformation)
                {
                    // Don't re-parse - progressive transformations advance at cycle boundaries
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
                {
                    // Progressive transformations and scenes now advance only at cycle boundaries (step 0)
                    // This ensures accent maps are properly synchronized from pattern start
                }
                else
                {
                    // CONSISTENT RESET: All pattern changes reset to step 0 for clean synchronization
                    parseAndApplyUPI(currentUPIInput, true);
                }
            }
            // Pattern updates are handled via UPI only
        }
    }
}

/**
 * Rotates a pattern by a specified number of steps.
 * 
 * Used for progressive offset patterns. Positive steps rotate forward,
 * negative steps rotate backward. Steps are normalized to pattern length.
 * 
 * @param pattern The pattern to rotate
 * @param steps Number of steps to rotate (can be negative)
 * @return The rotated pattern
 */
std::vector<bool> RhythmPatternExplorerAudioProcessor::rotatePatternBySteps(const std::vector<bool>& pattern, int steps)
{
    if (pattern.empty()) 
        return pattern;
    
    std::vector<bool> rotated(pattern.size());
    int size = static_cast<int>(pattern.size());
    
    // Normalize steps to be within pattern size
    steps = steps % size;
    if (steps < 0) steps += size;
    
    for (int i = 0; i < size; ++i)
    {
        int newIndex = (i + steps) % size;
        rotated[newIndex] = pattern[i];
    }
    
    return rotated;
}

/**
 * Advances progressive lengthening patterns by adding random steps.
 * 
 * Generates additional steps using a bell curve distribution to create
 * musically interesting patterns. Used for pattern*N syntax.
 */
void RhythmPatternExplorerAudioProcessor::advanceProgressiveLengthening()
{
    if (progressiveLengthening > 0 && !baseLengthPattern.empty())
    {
        // Generate random steps using bell curve distribution
        auto randomSteps = generateBellCurveRandomSteps(progressiveLengthening);
        
        // Append the random steps to the pattern
        baseLengthPattern.insert(baseLengthPattern.end(), randomSteps.begin(), randomSteps.end());
        
        // Random steps added to lengthened pattern
    }
}

/**
 * Generates random steps using a bell curve distribution.
 * 
 * Creates a musically pleasing distribution of onsets that avoids extremes
 * (all onsets or no onsets). Used for progressive lengthening.
 * 
 * @param numSteps Number of steps to generate
 * @return Vector of boolean values representing the random pattern
 */
std::vector<bool> RhythmPatternExplorerAudioProcessor::generateBellCurveRandomSteps(int numSteps)
{
    std::vector<bool> randomSteps(numSteps, false);
    
    if (numSteps <= 0) return randomSteps;
    
    // Use bell curve distribution to determine number of onsets (avoid extremes)
    std::normal_distribution<double> distribution(numSteps / 2.0, (numSteps - 1) / 6.0);
    int onsets = static_cast<int>(std::round(distribution(randomGenerator)));
    
    // Clamp to valid range [1, numSteps-1] to avoid empty or full patterns
    onsets = juce::jmax(1, juce::jmin(numSteps - 1, onsets));
    
    // Randomly distribute the onsets
    std::vector<int> positions;
    for (int i = 0; i < numSteps; ++i)
    {
        positions.push_back(i);
    }
    
    std::shuffle(positions.begin(), positions.end(), randomGenerator);
    
    for (int i = 0; i < onsets && i < positions.size(); ++i)
    {
        randomSteps[positions[i]] = true;
    }
    
    return randomSteps;
}

/**
 * Lengthens a pattern by adding random steps.
 * 
 * Appends randomly generated steps to an existing pattern.
 * 
 * @param pattern The base pattern to extend
 * @param additionalSteps Number of additional steps to add
 * @return The lengthened pattern
 */
std::vector<bool> RhythmPatternExplorerAudioProcessor::lengthenPattern(const std::vector<bool>& pattern, int additionalSteps)
{
    auto lengthened = pattern;
    auto randomSteps = generateBellCurveRandomSteps(additionalSteps);
    lengthened.insert(lengthened.end(), randomSteps.begin(), randomSteps.end());
    return lengthened;
}

/**
 * Advances to the next scene in scene cycling mode.
 * 
 * Advances the progressive state for the current scene (if applicable),
 * then moves to the next scene in the sequence, cycling back to the
 * beginning when reaching the end.
 */
void RhythmPatternExplorerAudioProcessor::advanceScene()
{
    if (!scenePatterns.isEmpty() && currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()))
    {
        int oldSceneIndex = currentSceneIndex;
        
        // First, advance the progressive state for the current scene if it has progressive syntax
        if (sceneProgressiveSteps[currentSceneIndex] != 0) {
            if (sceneProgressiveOffsets[currentSceneIndex] != 0) {
                // Progressive offset scene
                sceneProgressiveOffsets[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            } else if (sceneProgressiveLengthening[currentSceneIndex] != 0) {
                // Progressive lengthening scene
                sceneProgressiveLengthening[currentSceneIndex] += sceneProgressiveSteps[currentSceneIndex];
            }
        }
        
        // Then advance to next scene in the sequence, cycling back to 0 when reaching the end
        currentSceneIndex = (currentSceneIndex + 1) % scenePatterns.size();
        
        // DEBUG LOGGING: Track scene changes
        {
            std::ofstream logFile("/tmp/accent_debug.log", std::ios::app);
            if (logFile.is_open()) {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                
                logFile << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "." 
                       << std::setfill('0') << std::setw(3) << ms.count() << "] "
                       << "SCENE_CHANGE: from=" << oldSceneIndex
                       << " to=" << currentSceneIndex
                       << " pattern=" << scenePatterns[currentSceneIndex].toStdString()
                       << " globalPos=" << globalAccentPosition
                       << " uiOffset=" << uiAccentOffset
                       << std::endl;
                logFile.close();
            }
        }
        
        // Notify UI that pattern has changed for accent map updates
        patternChanged.store(true);
        
        // Scene advanced with progressive state updated
    }
}

/**
 * Applies the current scene pattern with its progressive state.
 * 
 * Parses the base pattern for the current scene and applies any
 * progressive transformations (offset or lengthening) that have
 * been accumulated for this scene.
 */
void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // Apply the current scene pattern using per-scene progressive state
    if (currentSceneIndex >= static_cast<int>(sceneBasePatterns.size())) {
        return; // Safety check
    }
    
    juce::String basePattern = sceneBasePatterns[currentSceneIndex];
    int progressiveOffset = sceneProgressiveOffsets[currentSceneIndex];
    int progressiveLengthening = sceneProgressiveLengthening[currentSceneIndex];
    
    // Parse the base pattern first (FULL RESTART: scene changes reset everything)
    parseAndApplyUPI(basePattern, true); // Scene changes trigger full restart
    
    // REFERENCE POINT FIX: UI now uses same global reference as MIDI - no sync needed
    
    // Apply progressive transformations if any
    if (progressiveOffset != 0)
    {
        // Apply progressive offset by rotating the generated pattern
        auto currentPattern = patternEngine.getCurrentPattern();
        auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
        patternEngine.setPattern(rotatedPattern);
        
        // CRITICAL: Trigger UI update after pattern transformation
        patternChanged.store(true);
        
        // Scene progressive offset applied
    }
    else if (progressiveLengthening != 0)
    {
        // Apply progressive lengthening
        auto currentPattern = patternEngine.getCurrentPattern();
        
        // Store or retrieve the base pattern for this scene
        if (sceneBaseLengthPatterns[currentSceneIndex].empty()) {
            // First time - store the base pattern
            sceneBaseLengthPatterns[currentSceneIndex] = currentPattern;
        }
        
        // Apply lengthening to the stored base pattern
        auto lengthenedPattern = lengthenPattern(sceneBaseLengthPatterns[currentSceneIndex], progressiveLengthening);
        patternEngine.setPattern(lengthenedPattern);
        
        // CRITICAL: Trigger UI update after pattern transformation
        patternChanged.store(true);
        
        // Scene progressive lengthening applied
    }
}

/**
 * Gets the current progressive trigger count for UI display.
 * 
 * Returns the step count for progressive transformations using the
 * newer ">" syntax, or falls back to the pattern engine's count for
 * older "@#" syntax patterns.
 * 
 * @return Current step count in the progressive sequence
 */
int RhythmPatternExplorerAudioProcessor::getProgressiveTriggerCount() const
{
    // For progressive transformations with ">" syntax, get step count from UPIParser
    if (!currentProgressivePatternKey.isEmpty())
    {
        return UPIParser::getProgressiveStepCount(currentProgressivePatternKey);
    }
    
    // Fall back to PatternEngine's trigger count for old "@#" syntax
    return patternEngine.getProgressiveTriggerCount();
}

//==============================================================================
// Accent Map Generation

/**
 * Generates the current accent map for UI display.
 * 
 * CRITICAL ANTI-SWIRLING PROTECTION:
 * This function MUST NEVER use globalAccentPosition directly as it changes every step.
 * Using globalAccentPosition causes "swirling" where accent markers move constantly.
 * 
 * ONLY use uiAccentOffset which is stable and only updates at specific synchronization points:
 * - Pattern changes (new patterns, progressive transformations, scene switching)
 * - Cycle boundaries (for polymetric patterns)
 * - Manual triggers (Enter, MIDI input)
 * 
 * @return Vector of boolean values indicating accented steps (STABLE, no swirling)
 */
std::vector<bool> RhythmPatternExplorerAudioProcessor::getCurrentAccentMap() const
{
    // ANTI-SWIRLING: Use ONLY uiAccentOffset, NEVER globalAccentPosition
    auto pattern = patternEngine.getCurrentPattern();
    std::vector<bool> accentMap(pattern.size(), false);

    if (!hasAccentPattern || currentAccentPattern.empty()) {
        return accentMap;
    }

    // STABLE UI: Use uiAccentOffset for stable display that doesn't change every step
    // NEVER use globalAccentPosition here - it causes swirling!
    int accentLen = static_cast<int>(currentAccentPattern.size());
    int accentCounter = 0;
    
    for (size_t i = 0; i < pattern.size(); ++i)
    {
        if (pattern[i])
        {
            // SIMPLE ALIGNMENT: Since all triggers restart to step 0, use simple offset approach
            int accentStep = (uiAccentOffset + accentCounter) % accentLen;
            accentMap[i] = currentAccentPattern[accentStep];
            
            // DEBUG LOGGING: Track UI accent calculation
            {
                std::ofstream logFile("/tmp/accent_debug.log", std::ios::app);
                if (logFile.is_open()) {
                    auto now = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(now);
                    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
                    
                    logFile << "[" << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "." 
                           << std::setfill('0') << std::setw(3) << ms.count() << "] "
                           << "UI: step=" << i 
                           << " uiOffset=" << uiAccentOffset
                           << " accentCounter=" << accentCounter 
                           << " accentStep=" << accentStep
                           << " shouldAccent=" << (accentMap[i] ? "YES" : "NO")
                           << " currentStep=" << currentStep.load()
                           << std::endl;
                    logFile.close();
                }
            }
            
            accentCounter++;
        }
    }
    
    return accentMap;
}

//==============================================================================
// Audio Processing Helper Functions

/**
 * Converts pattern length choice parameter to float value.
 * 
 * Maps the discrete choice parameter indices to precise float values
 * for timing calculations. Supports fractional values like 0.125 for
 * fine-grained timing control.
 * 
 * @return The float value corresponding to the current choice selection
 */
float RhythmPatternExplorerAudioProcessor::getPatternLengthValue() const
{
    // Convert choice index to precise float values
    static const float values[] = {
        0.125f, 0.25f, 0.5f, 0.75f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f,
        11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
    };
    
    int index = patternLengthValueParam->getIndex();
    if (index >= 0 && index < static_cast<int>(sizeof(values) / sizeof(values[0]))) {
        return values[index];
    }
    return 8.0f; // Default fallback
}

//==============================================================================
/**
 * Factory function to create new plugin instances.
 * 
 * Required by JUCE framework for plugin instantiation.
 * 
 * @return New instance of the audio processor
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}