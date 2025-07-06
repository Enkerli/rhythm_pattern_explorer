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
    // Initialize parameters using original working approach
    addParameter(bpmParam = new juce::AudioParameterFloat("bpm", "BPM", 60.0f, 1000.0f, 120.0f));
    addParameter(patternTypeParam = new juce::AudioParameterChoice("patternType", "Pattern Type", 
        juce::StringArray("Euclidean", "Polygon", "Random", "Binary", "UPI"), 0));
    addParameter(onsetsParam = new juce::AudioParameterInt("onsets", "Onsets", 1, 16, 3));
    addParameter(stepsParam = new juce::AudioParameterInt("steps", "Steps", 4, 32, 8));
    addParameter(playingParam = new juce::AudioParameterBool("playing", "Playing", false));
    addParameter(useHostTransportParam = new juce::AudioParameterBool("useHostTransport", "Use Host Transport", true));
    
    // Initialize pattern engine with default Euclidean pattern
    patternEngine.generateEuclideanPattern(3, 8);
    
    // Set up progressive offset engine for UPI parser
    UPIParser::setProgressiveOffsetEngine(&patternEngine);
    
    DBG("RhythmPatternExplorer: Plugin initialized");
    
    // BITWIG FILE DEBUG: Write to file since console may be blocked
    FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
    if (debugFile) {
        time_t now = time(0);
        char* timeStr = ctime(&now);
        timeStr[strlen(timeStr)-1] = '\0'; // Remove newline
        fprintf(debugFile, "BITWIG INIT: Plugin constructor called at %s! Debug version v0.03a3.DBG active.\n", timeStr);
        fflush(debugFile);
        fclose(debugFile);
    }
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
    int currentBPM = static_cast<int>(bpmParam ? bpmParam->get() : 120);
    
    // More frequent logging at high BPMs
    bool shouldLog = false;
    if (currentBPM >= 200) {
        shouldLog = (debugCallCount % 25 == 0); // Every 25 calls at high BPM
    } else {
        shouldLog = (debugCallCount % 100 == 0); // Every 100 calls normally
    }
    
    if (shouldLog || std::abs(currentBPM - lastLoggedBPM) > 2) {
        FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "BITWIG PROCESS: BPM=%d, SamplesPerStep=%d, BufferSize=%d, CurrentSample=%d, CurrentStep=%d, SampleRate=%.0f, CallCount=%d\n",
                currentBPM, samplesPerStep, currentBufferSize, currentSample, currentStep.load(), currentSampleRate, debugCallCount);
            fflush(debugFile);
            fclose(debugFile);
        }
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
        float currentBPM = bpmParam ? bpmParam->get() : 120.0f;
        if (currentBPM >= 200.0f) {
            static int transportDebugCount = 0;
            if (++transportDebugCount % 100 == 0) {
                FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
                if (debugFile) {
                    fprintf(debugFile, "TRANSPORT DEBUG: hasValidPosition=%s, isPlaying=%s, isRecording=%s, ppqPosition=%.3f, bpm=%.1f\n",
                        hasValidPosition ? "TRUE" : "FALSE",
                        posInfo.isPlaying ? "TRUE" : "FALSE", 
                        posInfo.isRecording ? "TRUE" : "FALSE",
                        posInfo.ppqPosition, posInfo.bpm);
                    fflush(debugFile);
                    fclose(debugFile);
                }
            }
        }
        
        // Transport sync with host
    }
    else
    {
        // No playhead available
        float currentBPM = bpmParam ? bpmParam->get() : 120.0f;
        if (currentBPM >= 200.0f) {
            static int noPlayheadCount = 0;
            if (++noPlayheadCount % 100 == 0) {
                FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
                if (debugFile) {
                    fprintf(debugFile, "NO PLAYHEAD: Bitwig not providing transport info\n");
                    fflush(debugFile);
                    fclose(debugFile);
                }
            }
        }
    }

    // Determine if we should be playing
    bool isPlaying;
    if (useHostTransportParam && useHostTransportParam->get() && hasValidPosition)
    {
        // FIXED: Manual play button should work alongside host transport (OR logic)
        isPlaying = posInfo.isPlaying || playingParam->get();
        
        // BITWIG FIX: Always sync BPM, but conditionally sync position
        syncBPMWithHost(posInfo);
        syncPositionWithHost(posInfo);
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
    
    // Update timing if BPM changed - but preserve currentSample ratio
    static float lastBPM = 120.0f;
    float currentBPMFloat = bpmParam ? bpmParam->get() : 120.0f;
    if (std::abs(currentBPMFloat - lastBPM) > 0.1f) {
        // BITWIG FIX: Preserve timing position when BPM changes
        double sampleRatio = (samplesPerStep > 0) ? (double)currentSample / samplesPerStep : 0.0;
        updateTiming();
        currentSample = static_cast<int>(sampleRatio * samplesPerStep);
        
        // Log BPM changes for debugging
        if (currentBPMFloat >= 180.0f) {
            FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "BPM CHANGE: %.1f->%.1f, sampleRatio=%.3f, newCurrentSample=%d, newSamplesPerStep=%d\n",
                    lastBPM, currentBPMFloat, sampleRatio, currentSample, samplesPerStep);
                fflush(debugFile);
                fclose(debugFile);
            }
        }
        lastBPM = currentBPMFloat;
    }
    
    // Clean production version
    
    // Clean production version - no aggressive debugging

    // BITWIG PLAYING STATE DEBUG: Log playing detection
    if (currentBPMFloat >= 180.0f) {
        static int playingDebugCount = 0;
        if (++playingDebugCount % 50 == 0) {
            FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "PLAYING DEBUG: isPlaying=%s, hostIsPlaying=%s, playingParam=%s, useHostTransport=%s\n",
                    isPlaying ? "TRUE" : "FALSE",
                    hostIsPlaying ? "TRUE" : "FALSE", 
                    (playingParam->get() ? "TRUE" : "FALSE"),
                    (useHostTransportParam->get() ? "TRUE" : "FALSE"));
                fflush(debugFile);
                fclose(debugFile);
            }
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
                if (newStep == 0 && patternEngine.hasProgressiveOffsetEnabled())
                {
                    patternEngine.triggerProgressiveOffset();
                    // Re-apply the UPI pattern with new progressive offset
                    parseAndApplyUPI(currentUPIInput);
                }
                
                // BITWIG 210 DEBUG: Log step triggers to file at high BPMs
                float currentBPM = bpmParam ? bpmParam->get() : 120.0f;
                if (currentBPM >= 180.0f) {
                    FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
                    if (debugFile) {
                        fprintf(debugFile, "STEP TRIGGER: BPM=%.1f, Step %d->%d, samplesPerStep=%d, currentSampleBefore=%d, currentSampleAfter=%d, buffer=%d/%d\n",
                            currentBPM, currentStep.load(), newStep, samplesPerStep, 
                            currentSample + samplesPerStep, currentSample, sample, buffer.getNumSamples());
                        fflush(debugFile);
                        fclose(debugFile);
                    }
                }
                
                // Safety break to prevent infinite loops
                if (samplesPerStep <= 1) {
                    std::cout << "HIGH BPM ERROR: samplesPerStep too small (" << samplesPerStep << "), breaking loop" << std::endl;
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
    
    // HIGH BPM FIX: Enhanced timing validation for Bitwig testing
    if (samplesPerStep <= 0) {
        std::cout << "HIGH BPM ERROR: Invalid samplesPerStep=" << samplesPerStep << " at BPM=" << bpm << 
            " sampleRate=" << currentSampleRate << std::endl;
        samplesPerStep = static_cast<int>(currentSampleRate / 60.0); // Default to 1Hz fallback
    } else if (samplesPerStep < 10) {
        std::cout << "HIGH BPM CRITICAL: Extremely fast timing - samplesPerStep=" << samplesPerStep << 
            " at BPM=" << bpm << " (>" << (60.0 * currentSampleRate / (samplesPerStep * 4)) << " BPM equivalent)" << std::endl;
    } else if (samplesPerStep < 100) {
        std::cout << "HIGH BPM WARNING: Very fast timing - samplesPerStep=" << samplesPerStep << 
            " at BPM=" << bpm << " (" << (60.0 * currentSampleRate / (samplesPerStep * 4)) << " BPM equivalent)" << std::endl;
    }
        
    DBG("RhythmPatternExplorer: Updated timing - BPM: " << bpm << ", Samples per step: " << samplesPerStep);
}

void RhythmPatternExplorerAudioProcessor::processStep(juce::MidiBuffer& midiBuffer, int samplePosition)
{
    auto pattern = patternEngine.getCurrentPattern();
    
    // BITWIG 210 DEBUG: Log pattern state at high BPMs
    float currentBPM = bpmParam ? bpmParam->get() : 120.0f;
    static int stepCallCount = 0;
    
    if (currentBPM >= 200.0f && ++stepCallCount % 3 == 0) {
        std::cout << "PROCESS STEP: BPM=" << currentBPM << 
            ", Step=" << currentStep.load() << 
            ", PatternSize=" << pattern.size() << 
            ", HasOnset=" << (currentStep.load() < pattern.size() && pattern[currentStep.load()] ? "YES" : "NO") << 
            ", SamplePos=" << samplePosition << std::endl;
    }
    
    if (currentStep < pattern.size() && pattern[currentStep])
    {
        triggerNote(midiBuffer, samplePosition);
        
        // Log note triggers at high BPM
        if (currentBPM >= 200.0f) {
            std::cout << "NOTE TRIGGERED: Step=" << currentStep.load() << 
                ", BPM=" << currentBPM << 
                ", SamplePos=" << samplePosition << std::endl;
        }
    }
}

void RhythmPatternExplorerAudioProcessor::triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition)
{
    // Get MIDI note number from editor, default to 36 (C2) if no editor
    int noteNumber = 36; // Default kick drum note
    if (auto* editor = getActiveEditor())
    {
        if (auto* rhythmEditor = dynamic_cast<RhythmPatternExplorerAudioProcessorEditor*>(editor))
        {
            noteNumber = rhythmEditor->getMidiNoteNumber();
        }
    }
    
    // Send MIDI note
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, noteNumber, 0.8f);
    juce::MidiMessage noteOff = juce::MidiMessage::noteOff(1, noteNumber, 0.0f);
    
    midiBuffer.addEvent(noteOn, samplePosition);
    midiBuffer.addEvent(noteOff, samplePosition + 100); // 100 samples note duration
    
    // MIDI effect mode - no audio synthesis
    
    DBG("RhythmPatternExplorer: Note " << noteNumber << " triggered at step " << currentStep.load());
}

void RhythmPatternExplorerAudioProcessor::syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo)
{
    // ALWAYS sync BPM - this should never be disabled
    if (posInfo.bpm > 0)
    {
        float hostBPM = static_cast<float>(posInfo.bpm);
        float currentBPM = bpmParam->get();
        
        // BITWIG BPM SYNC DEBUG: Log BPM sync attempts  
        if (hostBPM >= 200.0f) {
            FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "BPM SYNC: hostBPM=%.1f, currentBPM=%.1f, diff=%.3f\n",
                    hostBPM, currentBPM, std::abs(currentBPM - hostBPM));
                fflush(debugFile);
                fclose(debugFile);
            }
        }
        
        // Update our BPM parameter to match host
        if (std::abs(currentBPM - hostBPM) > 0.1f)
        {
            bpmParam->setValueNotifyingHost(bpmParam->convertTo0to1(hostBPM));
            updateTiming();
            
            // Log successful BPM updates
            if (hostBPM >= 200.0f) {
                FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
                if (debugFile) {
                    fprintf(debugFile, "BPM UPDATED: %.1f->%.1f, newSamplesPerStep=%d\n",
                        currentBPM, hostBPM, samplesPerStep);
                    fflush(debugFile);
                    fclose(debugFile);
                }
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
        float currentBPM = bpmParam ? bpmParam->get() : 120.0f;
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
            FILE* debugFile = fopen("/tmp/bitwig_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "POSITION SYNC DISABLED: BPM=%.1f, targetStep=%d, currentStep=%d\n",
                    currentBPM, targetStep, currentStep.load());
                fflush(debugFile);
                fclose(debugFile);
            }
        }
    }
    
    hostIsPlaying = posInfo.isPlaying;
    lastHostPosition = posInfo.ppqPosition;
}

void RhythmPatternExplorerAudioProcessor::setUPIInput(const juce::String& upiPattern)
{
    juce::ScopedLock lock(processingLock);
    
    // Check for progressive syntax: scenes first (pattern|pattern|pattern), then pattern+N (offset), pattern*N (lengthening)
    juce::String pattern = upiPattern.trim();
    bool isScenes = pattern.contains("|");
    bool isProgressiveOffset = !isScenes && pattern.contains("+") && pattern.lastIndexOf("+") > 0;
    bool isProgressiveLengthening = !isScenes && pattern.contains("*") && pattern.lastIndexOf("*") > 0;
    
    if (isScenes)
    {
        // Handle scene cycling: pattern1|pattern2|pattern3
        auto scenes = juce::StringArray::fromTokens(pattern, "|", "");
        
        // File-based debug logging
        FILE* debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "Scene cycling detected: %d scenes\n", static_cast<int>(scenes.size()));
            for (int i = 0; i < scenes.size(); ++i) {
                fprintf(debugFile, "  Scene %d: %s\n", i, scenes[i].trim().toRawUTF8());
            }
            fflush(debugFile);
            fclose(debugFile);
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
            
            // Debug log advancement
            debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "Advanced to scene %d: %s\n", 
                    currentSceneIndex, scenePatterns[currentSceneIndex].toRawUTF8());
                fflush(debugFile);
                fclose(debugFile);
            }
        }
        else
        {
            // New scene sequence - reset and start with first scene
            scenePatterns.clear();
            for (const auto& scene : scenes) {
                scenePatterns.add(scene.trim());
            }
            currentSceneIndex = 0;
            
            // Debug log reset
            debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "New scene sequence - starting with scene 0: %s\n", 
                    scenePatterns[0].toRawUTF8());
                fflush(debugFile);
                fclose(debugFile);
            }
        }
        
        // Parse and apply the current scene pattern
        if (!scenePatterns.isEmpty() && currentSceneIndex < scenePatterns.size()) {
            parseAndApplyUPI(scenePatterns[currentSceneIndex]);
        }
    }
    else if (isProgressiveOffset)
    {
        // Handle progressive offset: pattern+N
        int plusIndex = pattern.lastIndexOf("+");
        juce::String newBasePattern = pattern.substring(0, plusIndex).trim();
        juce::String stepStr = pattern.substring(plusIndex + 1).trim();
        int newStep = stepStr.getIntValue();
        
        // File-based debug logging
        FILE* debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "Progressive offset detected: %s (base: %s, step: +%d)\n", 
                pattern.toRawUTF8(), newBasePattern.toRawUTF8(), newStep);
            fflush(debugFile);
            fclose(debugFile);
        }
        
        // If same base pattern, advance offset; if different, reset
        if (basePattern == newBasePattern && progressiveStep == newStep)
        {
            advanceProgressiveOffset();
            
            // Debug log advancement
            if (debugFile) {
                debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
                if (debugFile) {
                    fprintf(debugFile, "Advanced offset to: %d\n", progressiveOffset);
                    fflush(debugFile);
                    fclose(debugFile);
                }
            }
        }
        else
        {
            // New progressive pattern - reset and start
            basePattern = newBasePattern;
            progressiveStep = newStep;
            progressiveOffset = newStep; // Start with first offset
            
            // Debug log reset
            if (debugFile) {
                debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
                if (debugFile) {
                    fprintf(debugFile, "New progressive pattern - reset offset to: %d\n", progressiveOffset);
                    fflush(debugFile);
                    fclose(debugFile);
                }
            }
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
            FILE* debugFile2 = fopen("/tmp/rhythm_progressive_debug.log", "a");
            if (debugFile2) {
                fprintf(debugFile2, "Applied rotation: offset=%d\n", progressiveOffset);
                fflush(debugFile2);
                fclose(debugFile2);
            }
        }
    }
    else if (isProgressiveLengthening)
    {
        // Handle progressive lengthening: pattern*N
        int starIndex = pattern.lastIndexOf("*");
        juce::String newBasePattern = pattern.substring(0, starIndex).trim();
        juce::String lengthStr = pattern.substring(starIndex + 1).trim();
        int newLengthening = lengthStr.getIntValue();
        
        // File-based debug logging
        FILE* debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "Progressive lengthening detected: %s (base: %s, add: *%d)\n", 
                pattern.toRawUTF8(), newBasePattern.toRawUTF8(), newLengthening);
            fflush(debugFile);
            fclose(debugFile);
        }
        
        // If same base pattern, advance lengthening; if different, reset
        if (basePattern == newBasePattern && progressiveLengthening == newLengthening)
        {
            advanceProgressiveLengthening();
            
            // Debug log advancement
            debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "Advanced lengthening - pattern now has %d steps\n", 
                    static_cast<int>(baseLengthPattern.size()));
                fflush(debugFile);
                fclose(debugFile);
            }
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
            debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
            if (debugFile) {
                fprintf(debugFile, "New progressive lengthening - starting with %d steps\n", 
                    static_cast<int>(baseLengthPattern.size()));
                fflush(debugFile);
                fclose(debugFile);
            }
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

void RhythmPatternExplorerAudioProcessor::parseAndApplyUPI(const juce::String& upiPattern)
{
    if (upiPattern.isEmpty())
        return;
    
    DBG("parseAndApplyUPI called with: '" + upiPattern + "'");
    
    // For progressive patterns, always re-parse to advance state
    bool isProgressive = upiPattern.contains("#");
    DBG("   Is progressive: " + juce::String(isProgressive ? "YES" : "NO"));
    
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
        }
        else
        {
            patternEngine.setProgressiveOffset(false);
        }
        
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
                // Check for progressive patterns and scenes
                bool hasProgressiveOffset = currentUPIInput.contains("+") && currentUPIInput.lastIndexOf("+") > 0;
                bool hasProgressiveLengthening = currentUPIInput.contains("*") && currentUPIInput.lastIndexOf("*") > 0;
                bool hasOldProgressiveOffset = currentUPIInput.contains("#");
                bool hasScenes = currentUPIInput.contains("|");
                
                if (hasProgressiveOffset || hasProgressiveLengthening || hasOldProgressiveOffset || hasScenes)
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
                        // Advance scene cycling
                        advanceScene();
                        DBG("RhythmPatternExplorer: MIDI triggered scene advancement to scene " 
                            << currentSceneIndex);
                    }
                    
                    // Force re-parsing to apply new progressive transformation or scene
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
        
        FILE* debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "Added %d random steps, total length now: %d\n", 
                progressiveLengthening, static_cast<int>(baseLengthPattern.size()));
            fflush(debugFile);
            fclose(debugFile);
        }
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
    if (!scenePatterns.isEmpty())
    {
        // Advance to next scene in the sequence, cycling back to 0 when reaching the end
        currentSceneIndex = (currentSceneIndex + 1) % scenePatterns.size();
        
        FILE* debugFile = fopen("/tmp/rhythm_progressive_debug.log", "a");
        if (debugFile) {
            fprintf(debugFile, "Advanced to scene %d/%d: %s\n", 
                currentSceneIndex, static_cast<int>(scenePatterns.size()), 
                scenePatterns[currentSceneIndex].toRawUTF8());
            fflush(debugFile);
            fclose(debugFile);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RhythmPatternExplorerAudioProcessor();
}