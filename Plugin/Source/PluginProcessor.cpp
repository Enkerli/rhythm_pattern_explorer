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

// Debug output disabled for production performance
#ifdef DEBUG
    // Keep debug output in debug builds
#else
    #undef DBG
    #define DBG(textToWrite) do { } while (false)
#endif

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
    addParameter(patternLengthValueParam = new juce::AudioParameterFloat("patternLengthValue", "Pattern Length Value", 0.125f, 32.0f, 8.0f));
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    // Set up progressive offset engine for UPI parser
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    DBG("RhythmPatternExplorer: Plugin initialized");
    
    // Debug logging for plugin initialization
    logDebug(DebugCategory::BITWIG_INIT, "Plugin constructor called! Debug version v0.03a4.DBG active.");
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
    globalAccentPosition = 0;
    wasPlaying = false;
    
    // Force early initialization to work around Logic Pro loading order issues
    lastProcessBlockTime = juce::Time::getMillisecondCounter();
    
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
    // Handle tick parameter (equivalent to pressing Parse)
    bool currentTickState = tickParam ? tickParam->get() : false;
    if (currentTickState && !lastTickState) {
        // Tick edge detected - reparse current UPI pattern
        DBG("RhythmPatternExplorer: Tick parameter activated! UPI input: '" << currentUPIInput << "'");
        if (!currentUPIInput.isEmpty()) {
            setUPIInput(currentUPIInput);
            DBG("RhythmPatternExplorer: Pattern reparsed via tick");
        } else {
            DBG("RhythmPatternExplorer: No UPI pattern to tick - currentUPIInput is empty");
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
    
    // BITWIG 210 BPM DEBUG: Focus on the specific problem
    static int debugCallCount = 0;
    static int lastLoggedBPM = 0;
    
    int currentBufferSize = buffer.getNumSamples();
    if (currentBufferSize <= 0) {
        return;
    }
    
    // Log every 50 calls or when BPM changes, with focus on 200+ BPM
    debugCallCount++;
    int currentBPMInt = static_cast<int>(currentBPM);
    
    // More frequent logging at high BPMs
    bool shouldLog = false;
    if (currentBPM >= 200) {
        shouldLog = (debugCallCount % 25 == 0); // Every 25 calls at high BPM
    } else {
        shouldLog = (debugCallCount % 100 == 0); // Every 100 calls normally
    }
    
    if (shouldLog || std::abs(currentBPM - lastLoggedBPM) > 2) {
        juce::String message = juce::String::formatted("BPM=%.1f, SamplesPerStep=%d, BufferSize=%d, CurrentSample=%d, CurrentStep=%d, SampleRate=%.0f, CallCount=%d",
            currentBPM, samplesPerStep, currentBufferSize, currentSample, currentStep.load(), currentSampleRate, debugCallCount);
        logDebug(DebugCategory::BITWIG_PROCESS, message);
        lastLoggedBPM = currentBPM;
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
        
        // BITWIG TRANSPORT DEBUG: Log transport details
        // Use internal currentBPM variable
        if (currentBPM >= 200.0f) {
            static int transportDebugCount = 0;
            if (++transportDebugCount % 100 == 0) {
                juce::String message = juce::String::formatted("hasValidPosition=%s, isPlaying=%s, isRecording=%s, ppqPosition=%.3f, bpm=%.1f",
                    hasValidPosition ? "TRUE" : "FALSE",
                    posInfo.isPlaying ? "TRUE" : "FALSE", 
                    posInfo.isRecording ? "TRUE" : "FALSE",
                    posInfo.ppqPosition, posInfo.bpm);
                logDebug(DebugCategory::TRANSPORT, message);
            }
        }
        
        // Transport sync with host
    }
    else
    {
        // No playhead available
        // Use internal currentBPM variable
        if (currentBPM >= 200.0f) {
            static int noPlayheadCount = 0;
            if (++noPlayheadCount % 100 == 0) {
                logDebug(DebugCategory::TRANSPORT, "Bitwig not providing transport info");
            }
        }
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
    float currentPatternLengthValue = patternLengthValueParam->get();
    
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
        
        // Log BPM changes for debugging
        if (bpmChanged && currentBPM >= 180.0f) {
            juce::String message = juce::String::formatted("%.1f->%.1f, sampleRatio=%.3f, newCurrentSample=%d, newSamplesPerStep=%d",
                lastBPM, currentBPM, sampleRatio, currentSample, samplesPerStep);
            logDebug(DebugCategory::BPM_SYNC, message);
        }
    }
    
    // Always update lastBPM outside the if block
    if (bpmChanged) {
        lastBPM = currentBPM;
    }
    
    // Clean production version - no aggressive debugging

    // BITWIG PLAYING STATE DEBUG: Log playing detection
    if (currentBPM >= 180.0f) {
        static int playingDebugCount = 0;
        if (++playingDebugCount % 50 == 0) {
            logDebug(DebugCategory::BITWIG_PROCESS, 
                "PLAYING DEBUG: isPlaying=" + juce::String(isPlaying ? "TRUE" : "FALSE") +
                ", hostIsPlaying=" + juce::String(hostIsPlaying ? "TRUE" : "FALSE") + 
                ", internalPlaying=" + juce::String(internalPlaying ? "TRUE" : "FALSE") +
                ", useHostTransport=" + juce::String(useHostTransportParam->get() ? "TRUE" : "FALSE"));
        }
    }

    // BITWIG HIGH BPM FIX: Process samples with multiple step checking
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        if (isPlaying)
        {
            // CRITICAL FIX: Check for multiple steps per buffer at high BPM
            while (currentSample >= samplesPerStep && samplesPerStep > 0)
            {
                processStep(midiMessages, sample);
                currentSample -= samplesPerStep; // Subtract instead of reset to maintain fractional timing
                int newStep = (currentStep.load() + 1) % patternEngine.getStepCount();
                currentStep.store(newStep);
                
                // Trigger progressive offset advancement when pattern completes a cycle
                // But only for old "@#" syntax, not new ">" syntax
                if (newStep == 0 && patternEngine.hasProgressiveOffsetEnabled() && !currentUPIInput.contains(">"))
                {
                    patternEngine.triggerProgressiveOffset();
                    // Re-apply the UPI pattern with new progressive offset (preserve accent position)
                    parseAndApplyUPI(currentUPIInput, false);
                }
                
                // Log step triggers at high BPMs
                if (currentBPM >= 180.0f) {
                    juce::String message = juce::String::formatted("BPM=%.1f, Step %d->%d, samplesPerStep=%d, currentSampleBefore=%d, currentSampleAfter=%d, buffer=%d/%d",
                        currentBPM, currentStep.load(), newStep, samplesPerStep, 
                        currentSample + samplesPerStep, currentSample, sample, buffer.getNumSamples());
                    logDebug(DebugCategory::STEP_TRIGGER, message);
                }
                
                // Safety break to prevent infinite loops
                if (samplesPerStep <= 1) {
#ifdef DEBUG
                    std::cout << "HIGH BPM ERROR: samplesPerStep too small (" << samplesPerStep << "), breaking loop" << std::endl;
#endif
                    break;
                }
            }
            currentSample++;
        }
        else if (wasPlaying)
        {
            // Just stopped playing - reset position
            currentSample = 0;
            currentStep.store(0);
            globalAccentPosition = 0;
            // Stopped playing - resetting position
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
void RhythmPatternExplorerAudioProcessor::updateTiming()
{
    float bpm = currentBPM;
    
    // Get pattern length parameters for Phase 2 temporal control
    int lengthUnit = patternLengthUnitParam->getIndex(); // 0=Steps, 1=Beats, 2=Bars
    float lengthValue = patternLengthValueParam->get();
    
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
#ifdef DEBUG
        std::cout << "HIGH BPM ERROR: Invalid samplesPerStep=" << samplesPerStep << " at BPM=" << bpm << 
            " sampleRate=" << currentSampleRate << std::endl;
#endif
        samplesPerStep = static_cast<int>(currentSampleRate / 60.0); // Default to 1Hz fallback
    } else if (samplesPerStep < 10) {
#ifdef DEBUG
        std::cout << "HIGH BPM CRITICAL: Extremely fast timing - samplesPerStep=" << samplesPerStep << 
            " at BPM=" << bpm << " (>" << (60.0 * currentSampleRate / (samplesPerStep * 4)) << " BPM equivalent)" << std::endl;
#endif
    } else if (samplesPerStep < 100) {
#ifdef DEBUG
        std::cout << "HIGH BPM WARNING: Very fast timing - samplesPerStep=" << samplesPerStep << 
            " at BPM=" << bpm << " (" << (60.0 * currentSampleRate / (samplesPerStep * 4)) << " BPM equivalent)" << std::endl;
#endif
    }
        
    DBG("RhythmPatternExplorer: Updated timing - BPM: " << bpm << ", Samples per step: " << samplesPerStep);
}

void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    // BITWIG 210 DEBUG: Log pattern state at high BPMs
    // Use internal currentBPM variable
    static int stepCallCount = 0;
    
#ifdef DEBUG
    if (currentBPM >= 200.0f && ++stepCallCount % 3 == 0) {
        std::cout << "PROCESS STEP: BPM=" << currentBPM << 
            ", Step=" << currentStep.load() << 
            ", PatternSize=" << pattern.size() << 
            ", HasOnset=" << (currentStep.load() < pattern.size() && pattern[currentStep.load()] ? "YES" : "NO") << 
            ", SamplePos=" << samplePosition << std::endl;
    }
#endif
    
    if (currentStep < pattern.size() && pattern[currentStep])
    {
        // Check if this step should be accented
        bool shouldAccent = false;
        if (hasAccentPattern && !currentAccentPattern.empty())
        {
            // Use global accent position (persists across pattern cycles)
            int accentStep = globalAccentPosition % currentAccentPattern.size();
            shouldAccent = currentAccentPattern[accentStep];
            
            // Increment global accent position for next onset
            globalAccentPosition++;
            
            // Debug accent detection
            DBG("ACCENT DEBUG: Step=" << currentStep << 
                ", GlobalAccentPos=" << (globalAccentPosition - 1) <<
                ", AccentPatternSize=" << currentAccentPattern.size() <<
                ", AccentStep=" << accentStep <<
                ", ShouldAccent=" << (shouldAccent ? "YES" : "NO"));
        }
        else
        {
            DBG("ACCENT DEBUG: No accent pattern - hasAccentPattern=" << (hasAccentPattern ? "true" : "false") <<
                ", accentPatternSize=" << currentAccentPattern.size());
        }
        
        triggerNote(midiBuffer, samplePosition, shouldAccent);
        
#ifdef DEBUG
        // Log note triggers at high BPM
        if (currentBPM >= 200.0f) {
            std::cout << "NOTE TRIGGERED: Step=" << currentStep.load() << 
                ", BPM=" << currentBPM << 
                ", Accented=" << (shouldAccent ? "YES" : "NO") <<
                ", SamplePos=" << samplePosition << std::endl;
        }
#endif
    }
}

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
    
    DBG("RhythmPatternExplorer: " << (isAccented ? "ACCENTED " : "REGULAR ") << "Note " << noteNumber << 
        " (base: " << baseNoteNumber << ") triggered at step " << currentStep.load() << " with velocity " << velocity <<
        " (isAccented=" << (isAccented ? "true" : "false") << ")");
}

void RhythmPatternExplorerAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // ALWAYS sync BPM - this should never be disabled
    if (posInfo.bpm > 0)
    {
        float hostBPM = static_cast<float>(posInfo.bpm);
        float currentBPMValue = currentBPM;
        
        // BITWIG BPM SYNC DEBUG: Log BPM sync attempts  
        if (hostBPM >= 200.0f) {
            logDebug(DebugCategory::BPM_SYNC, 
                "BPM SYNC: hostBPM=" + juce::String(hostBPM, 1) + 
                ", currentBPM=" + juce::String(currentBPMValue, 1) + 
                ", diff=" + juce::String(std::abs(currentBPMValue - hostBPM), 3));
        }
        
        // Update our internal BPM to match host
        if (std::abs(currentBPMValue - hostBPM) > 0.1f)
        {
            currentBPM = hostBPM;
            updateTiming();
            
            // Log successful BPM updates
            if (hostBPM >= 200.0f) {
                logDebug(DebugCategory::BPM_SYNC, 
                    "BPM UPDATED: " + juce::String(currentBPMValue, 1) + "->" + 
                    juce::String(hostBPM, 1) + ", newSamplesPerStep=" + juce::String(samplesPerStep));
            }
        }
    }
}

void RhythmPatternExplorerAudioProcessor::syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // Calculate pattern position based on host timeline
    if (posInfo.ppqPosition >= 0.0)
    {
        // Calculate which step we should be on based on timeline position
        double beatsPerStep = 0.25; // 16th note = quarter beat
        double currentBeat = posInfo.ppqPosition;
        double stepsFromStart = currentBeat / beatsPerStep;
        
        int targetStep = static_cast<int>(stepsFromStart) % patternEngine.getStepCount();
        
        // CRITICAL FIX: Disable position sync entirely - it interferes with natural step advancement
        // Use internal currentBPM variable
        bool allowPositionSync = false;  // DISABLED: Position sync was resetting currentSample constantly
        
        // CRITICAL FIX: Only sync position when significantly out of sync (not every block)
        int stepDifference = std::abs(targetStep - currentStep.load());
        if (allowPositionSync && stepDifference > 2)  // Increased threshold from 1 to 2
        {
            currentStep.store(targetStep);
            currentSample = static_cast<int>((stepsFromStart - std::floor(stepsFromStart)) * samplesPerStep);
            DBG("RhythmPatternExplorer: Synced to host position - Step: " << targetStep << ", Sample: " << currentSample);
        }
        else if (!allowPositionSync && currentBPM >= 180.0f)
        {
            // Log that we're skipping position sync at high BPM
            logDebug(DebugCategory::POSITION_SYNC, 
                "POSITION SYNC DISABLED: BPM=" + juce::String(currentBPM, 1) + 
                ", targetStep=" + juce::String(targetStep) + 
                ", currentStep=" + juce::String(currentStep.load()));
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Reset global accent position when setting new UPI input
    globalAccentPosition = 0;
    
    DBG("RhythmPatternExplorerAudioProcessor::setUPIInput called with: '" << upiPattern << "'");
    
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
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // Log scene cycling detection
        juce::String sceneList;
        for (int i = 0; i < scenes.size(); ++i) {
            sceneList += juce::String::formatted("  Scene %d: %s\n", i, scenes[i].trim().toRawUTF8());
        }
        logDebug(DebugCategory::SCENE_CYCLING, juce::String::formatted("Scene cycling detected: %d scenes\n%s", static_cast<int>(scenes.size()), sceneList.toRawUTF8()));
        
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
            
            // Debug log advancement
            logDebug(DebugCategory::SCENE_CYCLING, 
                "Advanced to scene " + juce::String(currentSceneIndex) + ": " + scenePatterns[currentSceneIndex]);
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
            logDebug(DebugCategory::SCENE_CYCLING, scenesInfo);
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
        
        // Progressive offset debug logging
        logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
            "Progressive offset detected: " + pattern + " (base: " + newBasePattern + ", step: +" + juce::String(newStep) + ")");
        
        // If same base pattern, advance offset; if different, reset
        if (basePattern == newBasePattern && progressiveStep == newStep)
        {
            advanceProgressiveOffset();
            
            // Debug log advancement
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "Advanced offset to: " + juce::String(progressiveOffset));
        }
        else
        {
            // New progressive pattern - reset and start
            basePattern = newBasePattern;
            progressiveStep = newStep;
            progressiveOffset = newStep; // Start with first offset
            
            // Debug log reset
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "New progressive pattern - reset offset to: " + juce::String(progressiveOffset));
        }
        
        // Parse the base pattern first, then apply progressive offset via rotation
        parseAndApplyUPI(basePattern);
        
        // Apply progressive offset by rotating the generated pattern
        if (progressiveOffset != 0)
        {
            auto currentPattern = patternEngine.getCurrentPattern();
            auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
            patternEngine.setPattern(rotatedPattern);
            
            // Debug log rotation
            logDebug(DebugCategory::PROGRESSIVE_OFFSET, 
                "Applied rotation: offset=" + juce::String(progressiveOffset));
        }
    }
    else if (isProgressiveLengthening)
    {
        // Handle progressive lengthening: pattern*N
        int starIndex = pattern.lastIndexOf("*");
        juce::String newBasePattern = pattern.substring(0, starIndex).trim();
        juce::String lengthStr = pattern.substring(starIndex + 1).trim();
        int newLengthening = lengthStr.getIntValue();
        
        // Progressive lengthening debug logging
        logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
            "Progressive lengthening detected: " + pattern + " (base: " + newBasePattern + ", add: *" + juce::String(newLengthening) + ")");
        
        // If same base pattern, advance lengthening; if different, reset
        if (basePattern == newBasePattern && progressiveLengthening == newLengthening)
        {
            advanceProgressiveLengthening();
            
            // Debug log advancement
            logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
                "Advanced lengthening - pattern now has " + juce::String(static_cast<int>(baseLengthPattern.size())) + " steps");
        }
        else
        {
            // New progressive lengthening pattern - reset and start
            basePattern = newBasePattern;
            progressiveLengthening = newLengthening;
            
            // Parse base pattern and store for lengthening
            parseAndApplyUPI(basePattern);
            baseLengthPattern = patternEngine.getCurrentPattern();
            
            // Debug log reset
            logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
                "New progressive lengthening - starting with " + juce::String(static_cast<int>(baseLengthPattern.size())) + " steps");
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
        parseAndApplyUPI(pattern);
    }
    
    currentUPIInput = upiPattern;
}

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition)
{
    if (upiPattern.isEmpty())
        return;
    
    // Only reset global accent position for truly new patterns
    if (resetAccentPosition) {
        globalAccentPosition = 0;
    }
    
    DBG("parseAndApplyUPI called with: '" + upiPattern + "'");
    
    // For progressive patterns, always re-parse to advance state
    bool isProgressive = upiPattern.contains("#");
    DBG("   Is progressive: " + juce::String(isProgressive ? "YES" : "NO"));
    
    // Check if this is a progressive transformation with ">" syntax
    bool isProgressiveTransformation = upiPattern.contains(">");
    if (isProgressiveTransformation) {
        // Store the pattern key for step tracking
        currentProgressivePatternKey = upiPattern;
    }
    
    auto parseResult = UPIParser::parse(upiPattern);
    
    if (parseResult.isValid())
    {
        DBG("Parse successful in processor");
        DBG("   Pattern: " + UPIParser::patternToBinary(parseResult.pattern));
        DBG("   Name: " + parseResult.patternName);
        
        // Apply the parsed pattern to the engine
        patternEngine.setPattern(parseResult.pattern);
        
        // Set up progressive offset if present
        if (parseResult.hasProgressiveOffset)
        {
            patternEngine.setProgressiveOffset(true, parseResult.initialOffset, parseResult.progressiveOffset);
            DBG("   Progressive offset enabled: initial=" << parseResult.initialOffset 
                << ", progressive=" << parseResult.progressiveOffset);
                
            // Store progressive pattern key for step tracking
            if (!parseResult.progressivePatternKey.isEmpty())
            {
                currentProgressivePatternKey = parseResult.progressivePatternKey;
                DBG("   Progressive pattern key stored: " << currentProgressivePatternKey);
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
            DBG("   Accent pattern enabled: " << currentAccentPatternName << 
                " (" << UPIParser::patternToBinary(currentAccentPattern) << ")");
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
        
        DBG("   Onsets: " + juce::String(onsets) + ", Steps: " + juce::String(steps));
        
        // Pattern applied successfully via UPI
        
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
                DBG("RhythmPatternExplorer: Captured MIDI note " << noteNumber << " for output");
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
                
                // Skip auto-advancement for progressive transformations with ">" syntax
                if (hasProgressiveTransformation)
                {
                    DBG("RhythmPatternExplorer: MIDI input ignored for progressive transformation - manual trigger only");
                    // Don't re-parse - progressive transformations should only advance on manual trigger
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
                {
                    // Trigger progressive pattern advancement
                    if (hasProgressiveOffset || hasOldProgressiveOffset)
                    {
                        // Advance progressive offset first, then re-parse
                        if (patternEngine.hasProgressiveOffsetEnabled())
                        {
                            patternEngine.triggerProgressiveOffset();
                            DBG("RhythmPatternExplorer: MIDI triggered progressive offset advancement to " 
                                << patternEngine.getCurrentOffset());
                        }
                    }
                    
                    if (hasProgressiveLengthening)
                    {
                        // Advance progressive lengthening
                        advanceProgressiveLengthening();
                        DBG("RhythmPatternExplorer: MIDI triggered progressive lengthening advancement");
                    }
                    
                    if (hasScenes)
                    {
                        // Advance scene cycling and apply the new current scene
                        advanceScene();
                        applyCurrentScenePattern();
                        DBG("RhythmPatternExplorer: MIDI triggered scene advancement to scene " 
                            << currentSceneIndex);
                    }
                    else
                    {
                        // Force re-parsing to apply new progressive transformation (preserve accent position)
                        parseAndApplyUPI(currentUPIInput, false);
                    }
                }
                else
                {
                    // Re-parse other UPI patterns to trigger regeneration (preserve accent position)
                    parseAndApplyUPI(currentUPIInput, false);
                }
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
                
                // Skip auto-advancement for progressive transformations with ">" syntax
                if (hasProgressiveTransformation)
                {
                    DBG("RhythmPatternExplorer: CC input ignored for progressive transformation - manual trigger only");
                    // Don't re-parse - progressive transformations should only advance on manual trigger
                }
                else if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
                {
                    if (hasProgressiveOffset || hasOldProgressiveOffset)
                    {
                        if (patternEngine.hasProgressiveOffsetEnabled())
                        {
                            patternEngine.triggerProgressiveOffset();
                            DBG("RhythmPatternExplorer: CC triggered progressive offset advancement to " 
                                << patternEngine.getCurrentOffset());
                        }
                    }
                    
                    if (hasProgressiveLengthening)
                    {
                        advanceProgressiveLengthening();
                        DBG("RhythmPatternExplorer: CC triggered progressive lengthening advancement");
                    }
                    
                    if (hasScenes)
                    {
                        advanceScene();
                        applyCurrentScenePattern();
                        DBG("RhythmPatternExplorer: CC triggered scene advancement to scene " 
                            << currentSceneIndex);
                    }
                    else
                    {
                        parseAndApplyUPI(currentUPIInput, false);
                    }
                }
                else
                {
                    parseAndApplyUPI(currentUPIInput, false);
                }
            }
            // Pattern updates are handled via UPI only
        }
    }
}

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

void RhythmPatternExplorerAudioProcessor::advanceProgressiveLengthening()
{
    if (progressiveLengthening > 0 && !baseLengthPattern.empty())
    {
        // Generate random steps using bell curve distribution
        auto randomSteps = generateBellCurveRandomSteps(progressiveLengthening);
        
        // Append the random steps to the pattern
        baseLengthPattern.insert(baseLengthPattern.end(), randomSteps.begin(), randomSteps.end());
        
        logDebug(DebugCategory::PROGRESSIVE_LENGTHENING, 
            "Added " + juce::String(progressiveLengthening) + " random steps, total length now: " + 
            juce::String(static_cast<int>(baseLengthPattern.size())));
    }
}

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

std::vector<bool> RhythmPatternExplorerAudioProcessor::lengthenPattern(const std::vector<bool>& pattern, int additionalSteps)
{
    auto lengthened = pattern;
    auto randomSteps = generateBellCurveRandomSteps(additionalSteps);
    lengthened.insert(lengthened.end(), randomSteps.begin(), randomSteps.end());
    return lengthened;
}

void RhythmPatternExplorerAudioProcessor::advanceScene()
{
    if (!scenePatterns.isEmpty() && currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size()))
    {
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
        
        juce::String sceneInfo = "Advanced to scene " + juce::String(currentSceneIndex) + "/" + 
            juce::String(static_cast<int>(scenePatterns.size())) + ": " + scenePatterns[currentSceneIndex];
        if (currentSceneIndex < static_cast<int>(sceneProgressiveSteps.size())) {
            sceneInfo += "\n  Scene progressive state - offset: " + juce::String(sceneProgressiveOffsets[currentSceneIndex]) + 
                        ", lengthening: " + juce::String(sceneProgressiveLengthening[currentSceneIndex]);
        }
        logDebug(DebugCategory::SCENE_CYCLING, sceneInfo);
    }
}

void RhythmPatternExplorerAudioProcessor::applyCurrentScenePattern()
{
    // Apply the current scene pattern using per-scene progressive state
    if (currentSceneIndex >= static_cast<int>(sceneBasePatterns.size())) {
        return; // Safety check
    }
    
    juce::String basePattern = sceneBasePatterns[currentSceneIndex];
    int progressiveOffset = sceneProgressiveOffsets[currentSceneIndex];
    int progressiveLengthening = sceneProgressiveLengthening[currentSceneIndex];
    
    // Parse the base pattern first
    parseAndApplyUPI(basePattern);
    
    // Apply progressive transformations if any
    if (progressiveOffset != 0)
    {
        // Apply progressive offset by rotating the generated pattern
        auto currentPattern = patternEngine.getCurrentPattern();
        auto rotatedPattern = rotatePatternBySteps(currentPattern, progressiveOffset);
        patternEngine.setPattern(rotatedPattern);
        
        logDebug(DebugCategory::SCENE_CYCLING, 
            "Applied scene " + juce::String(currentSceneIndex) + " progressive offset: " + juce::String(progressiveOffset));
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
        
        logDebug(DebugCategory::SCENE_CYCLING, 
            "Applied scene " + juce::String(currentSceneIndex) + " progressive lengthening: " + juce::String(progressiveLengthening) + " steps");
    }
}

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
// Centralized Debug Logging Utility

void RhythmPatternExplorerAudioProcessor::logDebug(DebugCategory category, const juce::String& message) {
    // Debug logging disabled for production performance
    juce::ignoreUnused(category, message);
}

const char* RhythmPatternExplorerAudioProcessor::getCategoryName(DebugCategory category) {
    switch (category) {
        case DebugCategory::BITWIG_INIT: return "BITWIG_INIT";
        case DebugCategory::BITWIG_PROCESS: return "BITWIG_PROCESS";
        case DebugCategory::TRANSPORT: return "TRANSPORT";
        case DebugCategory::BPM_SYNC: return "BPM_SYNC";
        case DebugCategory::STEP_TRIGGER: return "STEP_TRIGGER";
        case DebugCategory::POSITION_SYNC: return "POSITION_SYNC";
        case DebugCategory::SCENE_CYCLING: return "SCENE_CYCLING";
        case DebugCategory::PROGRESSIVE_OFFSET: return "PROGRESSIVE_OFFSET";
        case DebugCategory::PROGRESSIVE_LENGTHENING: return "PROGRESSIVE_LENGTHENING";
        default: return "UNKNOWN";
    }
}

const char* RhythmPatternExplorerAudioProcessor::getLogFile(DebugCategory category) {
    switch (category) {
        case DebugCategory::SCENE_CYCLING:
        case DebugCategory::PROGRESSIVE_OFFSET:
        case DebugCategory::PROGRESSIVE_LENGTHENING:
            return "/tmp/rhythm_progressive_debug.log";
        default:
            return "/tmp/bitwig_debug.log";
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}