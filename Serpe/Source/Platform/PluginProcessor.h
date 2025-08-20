/*
  ==============================================================================

    Serpe - Sequence Editor, Rhythmic Pattern Explorer
    Unified Audio Processor Header
    (Adapted from RhythmPatternExplorer original implementation)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Core/PatternEngine.h"
#include "../Core/UPIParser.h"
#include "../Managers/SceneManager.h"
#include "../Managers/ProgressiveManager.h"
#include "../Managers/PresetManager.h"
#include "PlatformSpecific.h"
#include <atomic>
#include <array>

//==============================================================================
// PHASE 2: Lock-Free Pattern Update Queue

struct PatternUpdate {
    std::vector<bool> rhythmPattern;
    std::vector<bool> accentPattern;
    bool hasAccent = false;
    int accentPhaseOffset = 0;  // User-controlled offset
    
    PatternUpdate() = default;
    PatternUpdate(const std::vector<bool>& rhythm, const std::vector<bool>& accent, bool accent_enabled, int offset = 0)
        : rhythmPattern(rhythm), accentPattern(accent), hasAccent(accent_enabled), accentPhaseOffset(offset) {}
};

class PatternUpdateQueue {
    static constexpr size_t QUEUE_SIZE = 16;
    std::array<PatternUpdate, QUEUE_SIZE> queue;
    std::atomic<size_t> writeIndex{0};
    std::atomic<size_t> readIndex{0};
    
public:
    bool enqueue(const PatternUpdate& update);
    bool dequeue(PatternUpdate& update);
    bool isEmpty() const;
};

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - MIDI effect processing (no audio synthesis)
 * - Parameter management
 * - Pattern generation and analysis
 * - DAW transport synchronization
 */
class SerpeAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SerpeAudioProcessor();
    ~SerpeAudioProcessor() override;

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
    // Pattern Engine Access
    const PatternEngine& getPatternEngine() const { return patternEngine; }
    PatternEngine& getPatternEngine() { return patternEngine; }
    
    // Pattern display information for UI
    juce::String getCurrentPatternDisplay() const;
    
    
    // Parameter access for editor
    juce::AudioParameterBool* getUseHostTransportParameter() const { return useHostTransportParam; }
    juce::AudioParameterInt* getMidiNoteParameter() const { return midiNoteParam; }
    juce::AudioParameterBool* getTickParameter() const { return tickParam; }
    juce::AudioParameterChoice* getPatternLengthUnitParameter() const { return patternLengthUnitParam; }
    juce::AudioParameterChoice* getPatternLengthValueParameter() const { return patternLengthValueParam; }
    juce::AudioParameterChoice* getSubdivisionParameter() const { return subdivisionParam; }
    
    // Accent parameter access for editor
    juce::AudioParameterFloat* getAccentVelocityParameter() const { return accentVelocityParam; }
    juce::AudioParameterFloat* getUnaccentedVelocityParameter() const { return unaccentedVelocityParam; }
    juce::AudioParameterInt* getAccentPitchOffsetParameter() const { return accentPitchOffsetParam; }
    
    // Helper to convert pattern length choice to float value
    float getPatternLengthValue() const;
    
    // Helper to convert subdivision choice to beat fraction
    double getSubdivisionInBeats(int subdivisionIndex) const;
    
    // Phase 3: Advanced Host Sync - Automatic pattern length adjustment
    double calculateAutoPatternLength(const std::vector<bool>& pattern) const;
    
    // Internal state access for editor
    float getCurrentBPM() const { return currentBPM; }
    void setCurrentBPM(float bpm) { currentBPM = bpm; updateTiming(); }
    bool getInternalPlaying() const { return internalPlaying; }
    void setInternalPlaying(bool playing) { internalPlaying = playing; }
    void triggerPatternUpdate() { if (!currentUPIInput.isEmpty()) parseAndApplyUPI(currentUPIInput); }
    
    // Phase 3: Advanced Host Sync - Loop sync access
    bool getHostIsLooping() const { return hostIsLooping; }
    bool getEnableLoopSync() const { return enableLoopSync; }
    void setEnableLoopSync(bool enable) { enableLoopSync = enable; }
    
    // Playback state
    int getCurrentStep() const { 
        // PHASE 0.2: Test derived approach - keep legacy for validation
        return getDerivedRhythmStep(); 
    }
    bool isCurrentlyPlaying() const { 
        // REFINED: Check if we're getting recent processBlock calls AND transport is playing
        double currentTime = juce::Time::getMillisecondCounter();
        bool recentProcessBlock = (currentTime - lastProcessBlockTime) < 100.0; // Within last 100ms
        
        // First try host transport, fall back to recent processBlock activity
        if (useHostTransportParam && useHostTransportParam->get()) {
            return hostIsPlaying && recentProcessBlock;
        } else {
            return internalPlaying && recentProcessBlock;
        }
    }
    bool isPlaying() const { return isCurrentlyPlaying(); } // Alias for compatibility
    
    // UPI pattern input methods
    void setUPIInput(const juce::String& upiPattern);
    juce::String getUPIInput() const { return currentUPIInput; }
    juce::String getCurrentUPIInput() const { return currentUPIInput; } // Alias for compatibility
    void parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition = true);
    void applyCurrentScenePattern();
    
    // Background color persistence
    int getCurrentBackgroundColor() const { return currentBackgroundColor; }
    void setCurrentBackgroundColor(int color) { currentBackgroundColor = color; }
    
    // UPI history (ticker tape feature)
    const juce::StringArray& getUPIHistory() const { return upiHistory; }
    void addToUPIHistory(const juce::String& upiPattern);
    void clearUPIHistory() { upiHistory.clear(); }
    
    // Preset management
    PresetManager& getPresetManager() { return presetManager; }
    const PresetManager& getPresetManager() const { return presetManager; }
    
    // Interactive pattern modification methods
    void togglePatternStep(int stepIndex);
    void toggleAccentAtStep(int stepIndex);
    bool isValidStepIndex(int stepIndex) const;
    void updateUPIFromCurrentPattern();
    
    // Progressive offset support (universal for all patterns)
    void resetProgressiveOffset() { 
        if (progressiveManager) {
            progressiveManager->resetProgressiveOffset(currentUPIInput);
        }
        progressiveOffset = 0; // Legacy fallback
    }
    void advanceProgressiveOffset() { 
        if (progressiveManager) {
            progressiveManager->triggerProgressive(currentUPIInput, patternEngine);
        }
        progressiveOffset += progressiveStep; // Legacy fallback
    }
    int getProgressiveOffset() const { 
        // TRANSITION: Use ProgressiveManager if available, fallback to legacy for safety
        if (progressiveManager && progressiveManager->hasProgressiveState(currentUPIInput)) {
            return progressiveManager->getProgressiveOffsetValue(currentUPIInput);
        }
        return progressiveOffset; // Legacy fallback
    }
    
    // Scene information access for UI
    int getCurrentSceneIndex() const { 
        // TRANSITION: Use SceneManager if available, fallback to legacy for safety
        return sceneManager ? sceneManager->getCurrentSceneIndex() : currentSceneIndex; 
    }
    int getSceneCount() const { 
        // TRANSITION: Use SceneManager if available, fallback to legacy for safety
        return sceneManager ? sceneManager->getSceneCount() : static_cast<int>(scenePatterns.size()); 
    }
    
    // Progressive transformation access for UI  
    int getProgressiveTriggerCount() const;
    bool hasProgressiveOffset() const { 
        // TRANSITION: Use ProgressiveManager if available, fallback to legacy for safety
        if (progressiveManager && progressiveManager->hasProgressiveState(currentUPIInput)) {
            return progressiveManager->hasProgressiveOffset(currentUPIInput);
        }
        return patternEngine.hasProgressiveOffsetEnabled(); // Legacy fallback
    }
    
    // Progressive lengthening support (universal for all patterns)
    void resetProgressiveLengthening() { 
        if (progressiveManager) {
            progressiveManager->resetProgressiveLengthening(currentUPIInput);
        }
        progressiveLengthening = 0; baseLengthPattern.clear(); // Legacy fallback
    }
    void advanceProgressiveLengthening();
    
    // Scene cycling support (universal for all patterns)
    void resetScenes() { 
        currentSceneIndex = 0; 
        scenePatterns.clear(); 
        sceneProgressiveOffsets.clear();
        sceneProgressiveSteps.clear();
        sceneBasePatterns.clear();
        sceneProgressiveLengthening.clear();
        sceneBaseLengthPatterns.clear();
    }
    void advanceScene();
    
    // Accent system access for UI and processing
    bool getHasAccentPattern() const { return hasAccentPattern; }
    const std::vector<bool>& getCurrentAccentPattern() const { return currentAccentPattern; }
    int getGlobalOnsetCounter() const { 
        // PHASE 3: Use derived onset count
        return static_cast<int>(getCurrentOnsetCount()); 
    }
    bool shouldOnsetBeAccented(int onsetNumber) const; // DEPRECATED: onset-based logic
    bool shouldStepBeAccented(int stepIndex) const;    // NEW: step-based logic for MIDI alignment
    std::vector<bool> getCurrentAccentMap() const;
    bool checkPatternChanged(); // Check and reset pattern changed flag
    void resetAccentSystem();
    
    // Debug info for UI display
    int getDebugTriggerCount() const { return debugTriggerCount; }
    int getDebugActiveNotesCount() const { return static_cast<int>(activeNotes.size()); }
    int getDebugNoteOffsSent() const { return debugNoteOffsSent; }
    int getDebugAbsoluteSamplePos() const { return absoluteSamplePosition; }
    
    // Lascabettes quantization access for UI and processing
    bool getHasQuantization() const { return hasQuantization; }
    int getOriginalStepCount() const { return originalStepCount; }
    int getQuantizedStepCount() const { return quantizedStepCount; }
    bool getQuantizationClockwise() const { return quantizationClockwise; }
    int getOriginalOnsetCount() const { return originalOnsetCount; }
    int getQuantizedOnsetCount() const { return quantizedOnsetCount; }

private:
    //==============================================================================
    // Pattern Engine
    PatternEngine patternEngine;
    
    // PHASE 2: Pattern Update Queue for thread-safe pattern changes
    PatternUpdateQueue patternUpdateQueue;
    
    // MIDI effect mode - no audio synthesis components needed
    
    // Timing and sequencing
    double currentSampleRate = 44100.0;
    int samplesPerStep = 0;
    int currentSample = 0;
    std::atomic<int> currentStep{0};  // Legacy - will be replaced by derived indices
    bool wasPlaying = false;
    
    /**
     * PHASE 1: MONOTONIC TRANSPORT TICK SYSTEM
     * 
     * This is the heart of the derived indices architecture that eliminates accent swirling.
     * All timing calculations derive from these atomic counters, ensuring consistency
     * between UI and MIDI systems.
     * 
     * transportTick: Monotonically increasing step counter, never decreases during playback
     * baseTickRhythm: Reference point for rhythm pattern position calculation  
     * baseTickAccent: Reference point for accent pattern position (can have phase offset)
     * 
     * Key insight: Instead of maintaining mutable step counters that can drift,
     * we calculate current positions mathematically from these base references.
     */
    std::atomic<uint64_t> transportTick{0};           // Monotonic step counter
    std::atomic<uint64_t> baseTickRhythm{0};          // Rhythm reference point  
    std::atomic<uint64_t> baseTickAccent{0};          // Accent reference point
    double samplesPerStepPrecise = 0.0;               // Precise step timing
    uint64_t sampleAccumulator = 0;                   // For precise step timing
    
    // Note tracking system for proper note duration management
    struct ActiveNote {
        int noteNumber;
        int endSample;  // Absolute sample position when note should end
        bool isActive;
        
        ActiveNote() : noteNumber(0), endSample(0), isActive(false) {}
        ActiveNote(int note, int end) : noteNumber(note), endSample(end), isActive(true) {}
    };
    
    std::vector<ActiveNote> activeNotes;
    int absoluteSamplePosition = 0;  // Track absolute sample position across buffers
    
    // DAW transport sync
    bool useHostTransport = true;
    double lastHostPosition = 0.0;
    bool hostIsPlaying = false;
    mutable double lastProcessBlockTime = 0.0;
    
    // Host loop sync (Phase 3: Advanced Host Sync)
    bool hostIsLooping = false;
    double hostLoopStart = 0.0;
    double hostLoopEnd = 0.0;
    bool enableLoopSync = true;
    
    // Internal state (not exposed as parameters)
    float currentBPM = 120.0f;
    bool internalPlaying = false;
    bool lastTickState = false;
    int tickResetCounter = 0;
    
    // UPI pattern input
    juce::String currentUPIInput;
    juce::String originalUPIInput; // Preserve original pattern with progressive/scene syntax
    juce::String lastParsedUPI;
    juce::String currentProgressivePatternKey; // Track current progressive pattern for step counting
    
    // Background color persistence
    int currentBackgroundColor = 0; // Default to Dark background
    
    // UPI history (ticker tape feature)
    juce::StringArray upiHistory;
    static constexpr int MAX_UPI_HISTORY = 20;
    
    // Preset management
    PresetManager presetManager;
    
    // Lascabettes quantization metadata
    bool hasQuantization = false;
    int originalStepCount = 0;
    int quantizedStepCount = 0;
    bool quantizationClockwise = true;
    int originalOnsetCount = 0;
    int quantizedOnsetCount = 0;
    
    
    // Progressive offset support (works for any pattern)
    int progressiveOffset = 0;      // Current accumulated offset
    int progressiveStep = 0;        // How much to advance each time
    juce::String basePattern;       // Pattern without progressive syntax
    
    // Progressive lengthening support (works for any pattern)
    int progressiveLengthening = 0; // How many steps to add each time
    std::vector<bool> baseLengthPattern; // Original pattern for lengthening
    std::mt19937 randomGenerator;   // For bell curve random step generation
    
    // Scene cycling support (works for any pattern)
    juce::StringArray scenePatterns; // List of patterns to cycle through - LEGACY, being replaced
    int currentSceneIndex = 0;      // Current scene position - LEGACY, being replaced
    
    // Per-scene progressive state tracking - LEGACY, being replaced
    std::vector<int> sceneProgressiveOffsets;     // Current offset for each scene
    std::vector<int> sceneProgressiveSteps;       // Step size for each scene
    std::vector<juce::String> sceneBasePatterns;  // Base pattern for each scene
    std::vector<int> sceneProgressiveLengthening; // Current lengthening for each scene
    std::vector<std::vector<bool>> sceneBaseLengthPatterns; // Base patterns for lengthening
    
    // New encapsulated management - TRANSITION: Running parallel with legacy for safety
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<ProgressiveManager> progressiveManager;
    
    // Thread safety
    juce::CriticalSection processingLock;
    
    // Pattern change notification for UI updates
    std::atomic<bool> patternChanged{false};
    
    // Debug counters for UI display
    std::atomic<int> debugTriggerCount{0};
    std::atomic<int> debugNoteOffsSent{0};
    
    // Accent system - single source of truth
    bool hasAccentPattern = false;
    std::vector<bool> currentAccentPattern;
    int globalOnsetCounter = 0;           // Single source of truth: counts all onsets since pattern start
    int uiAccentOffset = 0;               // Stable accent offset for UI display (updates only at cycle boundaries)
    bool accentPatternManuallyModified = false; // Flag to prevent automatic accent cycling after manual edits
    bool patternManuallyModified = false;       // Flag to indicate pattern has been manually edited (suspension mode)
    std::vector<bool> suspendedRhythmPattern;   // Preserve manually modified rhythm pattern
    std::vector<bool> suspendedAccentPattern;   // Preserve manually modified accent pattern
    
    // Parameters - implementation details
    juce::AudioParameterBool* useHostTransportParam;
    juce::AudioParameterInt* midiNoteParam;
    juce::AudioParameterBool* tickParam;
    juce::AudioParameterChoice* patternLengthUnitParam;
    juce::AudioParameterChoice* patternLengthValueParam;
    juce::AudioParameterChoice* subdivisionParam;
    
    // Accent parameters
    juce::AudioParameterFloat* accentVelocityParam;
    juce::AudioParameterFloat* unaccentedVelocityParam;
    juce::AudioParameterInt* accentPitchOffsetParam;
    
    // AudioProcessorValueTreeState for robust state management
    juce::AudioProcessorValueTreeState parameters;
    
    // Helper methods
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented = false);
    void syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void checkMidiInputForTriggers(juce::MidiBuffer& midiMessages);
    
    // PHASE 1: Monotonic Transport Tick helper methods
    void updateTransportTick(int bufferSize);
    uint32_t getCurrentRhythmStep() const;
    uint32_t getCurrentAccentStep() const;
    
    // PHASE 2: Pattern Update Queue methods
    void processPatternUpdates();
    void queuePatternUpdate(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern = {}, bool hasAccent = false, int accentPhaseOffset = 0);
    
    // PHASE 3: Derived onset counting
    uint32_t getCurrentOnsetCount() const;
    
    // PHASE 3: Emergency fix for direct pattern setting
    void setPatternWithPhaseSync(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern = {}, bool hasAccent = false, int accentPhaseOffset = 0);
    
    // PHASE 0: Derived index functions - proof of concept
    uint64_t getMonotonicTick() const;
    uint32_t getDerivedRhythmStep() const;
    uint32_t getDerivedAccentStep() const;
    void validateCounterConsistency() const; // Debug validation
    
    // Note tracking system methods
    void addActiveNote(int noteNumber, int duration);
    void processActiveNotes(juce::MidiBuffer& midiBuffer, int bufferSize);
    void clearAllActiveNotes(juce::MidiBuffer& midiBuffer);
    
    
    // Pattern manipulation
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    std::vector<bool> lengthenPattern(const std::vector<bool>& pattern, int additionalSteps);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpeAudioProcessor)
};