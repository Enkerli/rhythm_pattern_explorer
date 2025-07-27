/*
  ==============================================================================

    Rhythm Pattern Explorer - Plugin Editor
    User Interface for the AUv3 plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Plugin editor with simplified interface for mobile/desktop use
 * 
 * Features:
 * - Pattern type selection
 * - Onset/step count controls  
 * - BPM control
 * - Play/stop button
 * - Visual pattern display
 * - Pattern analysis display
 */
class RhythmPatternExplorerAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                   public juce::Timer
{
public:
    RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor&);
    ~RhythmPatternExplorerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer callback for UI updates
    void timerCallback() override;
    
    // MIDI note access
    
    // Background color options (public enum for color functions)
    enum class BackgroundColor {
        Dark = 0,     // Current dark background (0xff2d3748)
        White = 1,    // White background
        Green = 2,    // Same as onset color (0xff48bb78)
        Orange = 3,   // Same as highlight color (0xffff6b35)
        Blue = 4,     // Contrasting blue (0xff4299e1)
        Purple = 5    // Contrasting purple (0xff9f7aea)
    };
    
    // Background color access
    BackgroundColor getCurrentBackgroundColor() const { return currentBackgroundColor; }
    

private:
    //==============================================================================
    RhythmPatternExplorerAudioProcessor& audioProcessor;
    
    // UI Components
    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    
    juce::ComboBox patternTypeComboBox;
    juce::Label patternTypeLabel;
    
    juce::Slider onsetsSlider;
    juce::Label onsetsLabel;
    
    juce::Slider stepsSlider;
    juce::Label stepsLabel;
    
    juce::TextButton playButton;
    juce::TextButton generateButton;
    
    // UPI Pattern Input
    juce::TextEditor upiTextEditor;
    juce::Label upiLabel;
    
    
    
    
    // Tick Button
    juce::TextButton tickButton;
    
    juce::TextEditor patternDisplayEditor;
    juce::Label analysisLabel;
    juce::TextEditor versionEditor;
    
    // Documentation WebView
    std::unique_ptr<juce::WebBrowserComponent> docsBrowser;
    juce::TextButton docsToggleButton;
    bool showingDocs = false;
    
    // Minimal mode (Easter egg for very small windows)
    bool minimalMode = false;
    static constexpr int MINIMAL_MODE_THRESHOLD = 250; // Width/height threshold for minimal mode
    
    // Background color state
    BackgroundColor currentBackgroundColor = BackgroundColor::Dark;
    
    // Hover state for visual feedback
    int hoveredStepIndex = -1;  // -1 means no step is hovered
    bool isMouseInCircleArea = false;
    
    // Click animation state
    int clickedStepIndex = -1;  // -1 means no step is being animated
    int clickAnimationFrames = 0;
    
    // Parameter attachments for essential parameters only
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midiNoteAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> tickAttachment;
    
    // Visual pattern display
    void drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds);
    void updatePatternDisplay();
    void updateAnalysisDisplay();
    
    // Background color management
    juce::Colour getBackgroundColour() const;
    void cycleBackgroundColor();
    
    // Mouse interaction
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    
    // Pattern editing via mouse clicks
    int getStepIndexFromCoordinates(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const;
    bool isCoordinateInCircleArea(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const;
    
    // Enhanced click detection for accent control
    struct ClickResult {
        int stepIndex = -1;      // -1 if no valid step
        bool isInOuterHalf = false; // true if click is in outer half (accent area)
    };
    ClickResult getStepClickDetails(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const;
    
    // Circle area for responsive layout
    juce::Rectangle<int> circleArea;
    
    // UPI handling
    void parseUPIPattern();
    void onParseButtonClicked();
    
    // Step/Scene button update
    void updateStepSceneButton();
    
    // Documentation handling
    void toggleDocumentation();
    void createDocumentationHTML();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessorEditor)
};