/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - AUv3 Plugin
    Main Audio Processor Header

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PatternEngine.h"
#include "UPIParser.h"
#include "SceneManager.h"
#include "ProgressiveManager.h"
#include "PresetManager.h"

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer iPad AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - MIDI effect processing (no audio synthesis)
 * - Pattern generation and analysis
 * - iPad-optimized UI integration
 */
class RhythmPatternExplorerAudioProcessor : public juce::AudioProcessor
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
    
    // UPI Input for pattern creation
    void setUPIInput(const juce::String& upiString);
    void parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition = true);
    juce::String getCurrentUPIInput() const { return currentUPIInput; }
    
    // Current pattern access
    std::vector<bool> getCurrentPattern() const { return currentPattern; }
    int getCurrentStep() const { return currentStep; }
    
    // Scene management access
    int getSceneCount() const {
        return (sceneManager && sceneManager->hasScenes()) ? sceneManager->getSceneCount() : 1;
    }
    int getCurrentSceneIndex() const {
        return (sceneManager && sceneManager->hasScenes()) ? sceneManager->getCurrentSceneIndex() : 0;
    }
    
    // Progressive transformation access  
    bool hasProgressiveOffset() const {
        if (progressiveManager && progressiveManager->hasProgressiveState(currentUPIInput)) {
            return progressiveManager->hasProgressiveOffset(currentUPIInput);
        }
        return false;
    }
    
    // Progressive offset support (universal for all patterns)
    void resetProgressiveOffset() { 
        if (progressiveManager) {
            progressiveManager->resetProgressiveOffset(currentUPIInput);
        }
    }
    void advanceProgressiveOffset() { 
        if (progressiveManager) {
            progressiveManager->triggerProgressive(currentUPIInput, patternEngine);
        }
    }
    int getProgressiveOffset() const { 
        if (progressiveManager && progressiveManager->hasProgressiveState(currentUPIInput)) {
            return progressiveManager->getProgressiveOffsetValue(currentUPIInput);
        }
        return 0;
    }
    
    // Progressive lengthening support
    void resetProgressiveLengthening() { 
        if (progressiveManager) {
            progressiveManager->resetProgressiveLengthening(currentUPIInput);
        }
    }
    
    // Basic parameters
    juce::AudioParameterInt* getMidiNoteParameter() const { return midiNoteParam; }
    juce::AudioParameterBool* getTickParameter() const { return tickParam; }
    
    // Debug info for UI
    juce::String getDebugInfo() const { return debugInfo; }
    
    // Advanced management system access
    PresetManager& getPresetManager() { return presetManager; }
    const PresetManager& getPresetManager() const { return presetManager; }
    
    // Scene management functions
    void advanceScene();
    void applyCurrentScenePattern();
    
private:
    //==============================================================================
    // Core pattern engine
    PatternEngine patternEngine;
    
    // Current state
    juce::String currentUPIInput;
    juce::String originalUPIInput; // Preserve original pattern with progressive/scene syntax
    std::vector<bool> currentPattern;
    
    // MIDI processing
    juce::MidiMessageCollector midiCollector;
    double currentSampleRate { 44100.0 };
    
    // Timing and playback
    int currentStep { 0 };
    double samplesPerStep { 0.0 };
    double sampleCounter { 0.0 };
    
    // Parameters
    juce::AudioParameterInt* midiNoteParam;
    juce::AudioParameterBool* tickParam;
    
    // Advanced management systems
    PresetManager presetManager;
    std::unique_ptr<SceneManager> sceneManager;
    std::unique_ptr<ProgressiveManager> progressiveManager;
    
    // Helper methods
    void updateTiming(juce::Optional<juce::AudioPlayHead::PositionInfo> position = {});
    void processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber);
    void processPatternStep(juce::MidiBuffer& midiMessages, int sampleNumber, int stepToProcess);
    
private:
    juce::String debugInfo = "Ready";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessor)
};