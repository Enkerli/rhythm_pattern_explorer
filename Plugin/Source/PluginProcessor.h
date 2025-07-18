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
    PatternEngine& getPatternEngine() { return patternEngine; }
    
    // Accent Pattern Access
    bool getHasAccentPattern() const { return hasAccentPattern; }
    const std::vector<bool>& getCurrentAccentPattern() const { return currentAccentPattern; }
    const juce::String& getCurrentAccentPatternName() const { return currentAccentPatternName; }
    int getGlobalAccentPosition() const { return globalAccentPosition; }
    
    // Check if a specific onset should be accented based on current global accent position
    bool shouldOnsetBeAccented(int onsetIndex) const {
        if (!hasAccentPattern || currentAccentPattern.empty()) return false;
        
        // Calculate what the global accent position will be for this onset
        int accentStep = (globalAccentPosition + onsetIndex) % currentAccentPattern.size();
        return currentAccentPattern[accentStep];
    }
    
    /** Returns a vector of bools indicating which steps will be accented in the current pattern cycle. */
    std::vector<bool> getCurrentAccentMap() const;
    
    // UI update notification for accent map changes
    bool shouldUpdateAccentDisplay() const { return patternChanged.load(); }
    void clearAccentDisplayUpdate() { patternChanged.store(false); }
    
    // Get accent position for current pattern cycle (updates only at cycle boundaries)
    int getCurrentCycleAccentStart() const {
        // Calculate how many complete pattern cycles we've been through
        const auto& pattern = patternEngine.getCurrentPattern();
        if (pattern.empty()) return 0;
        
        // Count total onsets that have occurred before current cycle
        int onsetsInPattern = 0;
        for (bool onset : pattern) {
            if (onset) onsetsInPattern++;
        }
        
        if (onsetsInPattern == 0) return 0;
        
        // Calculate completed cycles and accent offset for this cycle
        int currentStep = getCurrentStep();
        int completedCycles = currentStep / pattern.size();
        int cycleAccentOffset = (completedCycles * onsetsInPattern) % currentAccentPattern.size();
        
        return cycleAccentOffset;
    }
    
    // Parameter access for editor
    juce::AudioParameterBool* getUseHostTransportParameter() { return useHostTransportParam; }
    juce::AudioParameterInt* getMidiNoteParameter() { return midiNoteParam; }
    juce::AudioParameterBool* getTickParameter() { return tickParam; }
    juce::AudioParameterInt* getAccentPitchOffsetParameter() { return accentPitchOffsetParam; }
    juce::AudioParameterFloat* getAccentVelocityParameter() { return accentVelocityParam; }
    juce::AudioParameterFloat* getUnaccentedVelocityParameter() { return unaccentedVelocityParam; }
    juce::AudioParameterChoice* getPatternLengthUnitParameter() { return patternLengthUnitParam; }
    juce::AudioParameterChoice* getPatternLengthValueParameter() { return patternLengthValueParam; }
    
    // Helper to convert pattern length choice to float value
    float getPatternLengthValue() const;
    
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
    
    // Progressive offset support (universal for all patterns)
    void resetProgressiveOffset() { progressiveOffset = 0; }
    void advanceProgressiveOffset() { progressiveOffset += progressiveStep; }
    int getProgressiveOffset() const { return progressiveOffset; }
    
    // Scene information access for UI
    int getCurrentSceneIndex() const { return currentSceneIndex; }
    int getSceneCount() const { return scenePatterns.size(); }
    
    // Progressive transformation access for UI  
    int getProgressiveTriggerCount() const;
    bool hasProgressiveOffset() const { return patternEngine.hasProgressiveOffsetEnabled(); }
    
    // Progressive lengthening support (universal for all patterns)
    void resetProgressiveLengthening() { progressiveLengthening = 0; baseLengthPattern.clear(); }
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
    
    // Parameters - accessible to editor
    juce::AudioParameterBool* useHostTransportParam;
    juce::AudioParameterInt* midiNoteParam;
    juce::AudioParameterBool* tickParam;
    juce::AudioParameterInt* accentPitchOffsetParam;
    juce::AudioParameterFloat* accentVelocityParam;
    juce::AudioParameterFloat* unaccentedVelocityParam;
    
    // Pattern Length parameters for Phase 2 temporal control
    juce::AudioParameterChoice* patternLengthUnitParam;
    juce::AudioParameterChoice* patternLengthValueParam;

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
    juce::String lastParsedUPI;
    juce::String currentProgressivePatternKey; // Track current progressive pattern for step counting
    
    // Accent pattern support
    bool hasAccentPattern = false;
    std::vector<bool> currentAccentPattern;
    juce::String currentAccentPatternName;
    int globalAccentPosition = 0;  // Global accent position counter (persists across pattern cycles)
    int uiAccentOffset = 0;         // Stable accent offset for UI display (updates only at cycle boundaries)
    
    // Progressive offset support (works for any pattern)
    int progressiveOffset = 0;      // Current accumulated offset
    int progressiveStep = 0;        // How much to advance each time
    juce::String basePattern;       // Pattern without progressive syntax
    
    // Progressive lengthening support (works for any pattern)
    int progressiveLengthening = 0; // How many steps to add each time
    std::vector<bool> baseLengthPattern; // Original pattern for lengthening
    std::mt19937 randomGenerator;   // For bell curve random step generation
    
    // Scene cycling support (works for any pattern)
    juce::StringArray scenePatterns; // List of patterns to cycle through
    int currentSceneIndex = 0;      // Current scene position
    
    // Per-scene progressive state tracking
    std::vector<int> sceneProgressiveOffsets;     // Current offset for each scene
    std::vector<int> sceneProgressiveSteps;       // Step size for each scene
    std::vector<juce::String> sceneBasePatterns;  // Base pattern for each scene
    std::vector<int> sceneProgressiveLengthening; // Current lengthening for each scene
    std::vector<std::vector<bool>> sceneBaseLengthPatterns; // Base patterns for lengthening
    
    // Thread safety
    juce::CriticalSection processingLock;
    
    // Pattern change notification for UI updates
    std::atomic<bool> patternChanged{false};
    
    // Helper methods
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented = false);
    void syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void checkMidiInputForTriggers(juce::MidiBuffer& midiMessages);
    
    // Audio processing helper functions
    
    // Pattern manipulation
    std::vector<bool> rotatePatternBySteps(const std::vector<bool>& pattern, int steps);
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    std::vector<bool> lengthenPattern(const std::vector<bool>& pattern, int additionalSteps);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};