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
 * UPI History List Model for ticker tape feature
 * 
 * Displays chronological history of all UPI patterns entered by the user.
 * Supports click-to-reload functionality for quick pattern recall.
 * Integrates with plugin state persistence to maintain history across sessions.
 */
class UPIHistoryListModel : public juce::ListBoxModel
{
public:
    UPIHistoryListModel(class RhythmPatternExplorerAudioProcessorEditor& editor) : editorRef(editor) {}
    
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    
private:
    class RhythmPatternExplorerAudioProcessorEditor& editorRef;
};

//==============================================================================
/**
 * Preset Browser List Model for preset management
 * 
 * Provides rich preset browsing with visual feature indicators:
 * - Unicode icons for scenes (▸), progressive transforms (↻), and accents (◆)
 * - Step count display for pattern complexity assessment
 * - Factory preset protection and user preset management
 * - Smart progressive offset advancement for repeated clicks
 * - Automatic feature detection and visual feedback
 */
class PresetBrowserListModel : public juce::ListBoxModel
{
public:
    PresetBrowserListModel(class RhythmPatternExplorerAudioProcessorEditor& editor) : editorRef(editor) {}
    
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    
    void refreshPresetList();
    
private:
    class RhythmPatternExplorerAudioProcessorEditor& editorRef;
    juce::StringArray currentPresetNames;
    
    // Helper method to parse step count from UPI pattern
    int parseStepCount(const juce::String& upiPattern);
};

//==============================================================================
/**
 * Plugin editor with simplified interface for mobile/desktop use
 * 
 * Core Features:
 * - UPI pattern input and parsing with comprehensive syntax support
 * - Visual pattern display with circular step layout and accent indicators
 * - Real-time step indicator and pattern progression
 * - Background color cycling for visual customization
 * 
 * Advanced UI Features (v0.03g):
 * - Preset browser with rich visual feedback and feature icons
 * - UPI history ticker tape for pattern recall and experimentation
 * - Integrated HTML documentation with WebView component
 * - Mutual exclusion overlay system with close buttons
 * - Progressive offset advancement for repeated preset clicks
 * - State persistence across plugin sessions
 * 
 * Responsive Design:
 * - Adaptive layout for various window sizes
 * - Minimal mode for very small windows
 * - Mobile-friendly touch interactions
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
    
    // Public access for ListBoxModel
    void onHistoryItemClicked(int index);
    void onPresetItemClicked(int index);
    RhythmPatternExplorerAudioProcessor& getAudioProcessor() { return audioProcessor; }
    
    // Preset management
    void togglePresets();
    void showSavePresetDialog();
    void saveCurrentPreset(const juce::String& name);
    void deleteSelectedPreset();
    

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
    juce::TextButton docsCloseButton;
    bool showingDocs = false;
    
    // Ticker tape UPI history sidebar
    juce::ListBox upiHistoryList;
    juce::TextButton historyToggleButton;
    juce::TextButton historyCloseButton;
    bool showingHistory = false;
    juce::Label historyLabel;
    std::unique_ptr<UPIHistoryListModel> historyListModel;
    
    // Preset browser sidebar
    juce::ListBox presetBrowserList;
    juce::TextButton presetToggleButton;
    juce::TextButton presetCloseButton;
    bool showingPresets = false;
    juce::Label presetLabel;
    std::unique_ptr<PresetBrowserListModel> presetListModel;
    juce::TextButton savePresetButton;
    juce::TextButton deletePresetButton;
    
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
    void closeDocumentation();
    void createDocumentationHTML();
    
    // History handling
    void toggleHistory();
    void closeHistory();
    
    // Preset handling  
    void closePresets();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RhythmPatternExplorerAudioProcessorEditor)
};