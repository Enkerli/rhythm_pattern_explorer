/*
  ==============================================================================

    Rhythm Pattern Explorer - AUv3 Plugin
    Main Audio Processor Header (Fixed)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatternEngine.h"
#include "UPIParser.h"
#include "SceneManager.h"
#include "ProgressiveManager.h"

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
class RhythmPatternExplorerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    RhythmPatternExplorerAudioProcessor();
    ~RhythmPatternExplorerAudioProcessor() override;

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
    
    // Internal state access for editor
    float getCurrentBPM() const { return currentBPM; }
    void setCurrentBPM(float bpm) { currentBPM = bpm; updateTiming(); }
    bool getInternalPlaying() const { return internalPlaying; }
    void setInternalPlaying(bool playing) { internalPlaying = playing; }
    void triggerPatternUpdate() { if (!currentUPIInput.isEmpty()) parseAndApplyUPI(currentUPIInput); }
    
    // Playback state
    int getCurrentStep() const { return currentStep.load(); }
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
    
    // UPI pattern input methods
    void setUPIInput(const juce::String& upiPattern);
    juce::String getUPIInput() const { return currentUPIInput; }
    void parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition = true);
    void applyCurrentScenePattern();
    
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
    void resetAccentSystem();

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
    
    // DAW transport sync
    bool useHostTransport = true;
    double lastHostPosition = 0.0;
    bool hostIsPlaying = false;
    mutable double lastProcessBlockTime = 0.0;
    
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
    
    // Helper methods
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented = false);
    void syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void checkMidiInputForTriggers(juce::MidiBuffer& midiMessages);
    
    
    // Pattern manipulation
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    std::vector<bool> lengthenPattern(const std::vector<bool>& pattern, int additionalSteps);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};