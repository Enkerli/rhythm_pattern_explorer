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
    int getCurrentStep() const { return currentStep.load(); }
    
    // DERIVED INDEXING FUNCTIONS - Pure functions from monotonic clock
    uint32_t getCurrentRhythmIndex() const;
    uint32_t getCurrentAccentIndex() const;
    bool shouldCurrentOnsetBeAccented() const;
    uint64_t getTransportTick() const { return transportTick.load(); }
    
    // PATTERN MASK BUILDING - Canonical pattern creation
    void buildPatternMasksFromUPI(const juce::String& upiInput);
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
    int getGlobalOnsetCounter() const { return globalOnsetCounter; }
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
    
    // MIDI effect mode - no audio synthesis components needed
    
    // Timing and sequencing
    double currentSampleRate = 44100.0;
    int samplesPerStep = 0;
    int currentSample = 0;
    std::atomic<int> currentStep{0};
    bool wasPlaying = false;
    
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
    
    // DERIVED INDEXING SYSTEM - Monotonic clock with phase-locked bases
    std::atomic<uint64_t> transportTick{0};      // Monotonic, never reset, audio-thread only
    std::atomic<uint64_t> baseTickRhythm{0};     // Phase-locked at pattern acceptance
    std::atomic<uint64_t> baseTickAccent{0};     // Phase-locked at accent acceptance
    
    // Immutable pattern data structure
    struct PatternMasks {
        std::vector<bool> rhythmMask;            // [0..nR-1] step-indexed rhythm
        std::vector<bool> accentMask;            // [0..nA-1] accent pattern  
        std::vector<uint32_t> onsetSteps;        // precomputed onset positions
        std::vector<uint32_t> onsetIndexForStep; // inverse mapping step->onset index
        uint32_t rhythmPeriod{0};
        uint32_t accentPeriod{0};
        bool useOnsetIndexedAccents{true};       // mode: onset-indexed vs step-indexed
        
        PatternMasks() = default;
        ~PatternMasks() = default;
        PatternMasks(const PatternMasks&) = delete;
        PatternMasks& operator=(const PatternMasks&) = delete;
    };
    
    std::atomic<PatternMasks*> currentMasks{nullptr};  // Atomic pointer for thread-safe swaps
    
    // Legacy compatibility (will be replaced with derived indices)
    bool hasAccentPattern = false;
    std::vector<bool> currentAccentPattern;
    int globalOnsetCounter = 0;           // DEPRECATED: Will be replaced with derived index
    int uiAccentOffset = 0;               // DEPRECATED: Will be replaced with derived index
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
    
    // Note tracking system methods
    void addActiveNote(int noteNumber, int duration);
    void processActiveNotes(juce::MidiBuffer& midiBuffer, int bufferSize);
    void clearAllActiveNotes(juce::MidiBuffer& midiBuffer);
    
    
    // Pattern manipulation
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    std::vector<bool> lengthenPattern(const std::vector<bool>& pattern, int additionalSteps);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpeAudioProcessor)
};