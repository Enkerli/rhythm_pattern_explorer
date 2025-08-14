/*
  ==============================================================================

    Rhythm Pattern Explorer - Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

// Disable debug output for production performance
#ifdef NDEBUG
#undef DBG
#endif

// Adaptive Color Schemes for Pattern Visualization
namespace PatternColors {
    
    // Get adaptive colors based on current background
    inline juce::Colour getUnaccentedColor(RhythmPatternExplorerAudioProcessorEditor::BackgroundColor bgColor) {
        switch (bgColor) {
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Blue:
                // Blue background: use warm orange for strong contrast
                return juce::Colour(0xfff97316);  // Warm orange - high contrast with blue bg
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Green:
                // Green background: use deep purple for contrast  
                return juce::Colour(0xff7c3aed);  // Deep purple - contrasts with green
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Orange:
                // Orange background: use deep blue for contrast
                return juce::Colour(0xff1e40af);  // Deep blue - contrasts with orange
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Purple:
                // Purple background: use yellow-green for contrast
                return juce::Colour(0xff65a30d);  // Yellow-green - contrasts with purple
            default:
                // Dark/White backgrounds: use our standard cool blue
                return juce::Colour(0xff4a90e2);  // Cool blue - works on dark/white
        }
    }
    
    inline juce::Colour getAccentedColor(RhythmPatternExplorerAudioProcessorEditor::BackgroundColor bgColor) {
        switch (bgColor) {
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Blue:
                // Blue background: use bright yellow for maximum accent contrast
                return juce::Colour(0xfffbbf24);  // Bright yellow - maximum visibility on blue
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Green:
                // Green background: use bright pink for accent
                return juce::Colour(0xffec4899);  // Bright pink - high contrast accent
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Orange:
                // Orange background: use cyan for accent  
                return juce::Colour(0xff06b6d4);  // Cyan - complementary accent
            case RhythmPatternExplorerAudioProcessorEditor::BackgroundColor::Purple:
                // Purple background: use bright lime for accent
                return juce::Colour(0xff84cc16);  // Bright lime - high visibility accent
            default:
                // Dark/White backgrounds: use our standard warm amber
                return juce::Colour(0xfff5a623);  // Warm amber - works on dark/white
        }
    }
    
    // Static colors that work on all backgrounds
    const juce::Colour ACCENT_OUTLINE = juce::Colours::white;       // White outline for maximum contrast
    const juce::Colour SEPARATOR_LINES = juce::Colour(0xff7a7a7a);  // Light grey - neutral
    const juce::Colour STEP_MARKERS = juce::Colour(0xff2d3748);     // Dark slate - neutral  
    const juce::Colour STEP_MARKER_TEXT = juce::Colours::white;     // White text - readable on dark
}

//==============================================================================
RhythmPatternExplorerAudioProcessorEditor::RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Plugin size - resizable with minimum and maximum constraints
    setSize (500, 600);
    setResizable(true, true);
    setResizeLimits(150, 150, 1200, 1000); // min width, min height, max width, max height - small min for Easter egg
    
    
    // UPI Pattern Input
    upiLabel.setText("UPI:", juce::dontSendNotification);
    upiLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(upiLabel);
    
    upiTextEditor.setMultiLine(false);
    upiTextEditor.setReturnKeyStartsNewLine(false);
    upiTextEditor.setTextToShowWhenEmpty("Enter pattern: E(3,8), P(5,0), etc.", juce::Colours::grey);
    upiTextEditor.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain)));
    upiTextEditor.onReturnKey = [this]() { parseUPIPattern(); };
    upiTextEditor.setText(audioProcessor.getUPIInput(), juce::dontSendNotification);
    addAndMakeVisible(upiTextEditor);
    
    
    
    
    
    // Scene/Step Button (equivalent to Parse/Tick) - smaller and shows current step/scene
    tickButton.setButtonText("1"); // Will be updated in timer
    tickButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    tickButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    tickButton.setTooltip("Current step/scene number. Click to advance.");
    addAndMakeVisible(tickButton);
    
    // Pattern Display Editor - copyable and readable
    patternDisplayEditor.setMultiLine(true);
    patternDisplayEditor.setReadOnly(true);
    patternDisplayEditor.setScrollbarsShown(false);
    patternDisplayEditor.setCaretVisible(false);
    patternDisplayEditor.setJustification(juce::Justification::centred);
    patternDisplayEditor.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain)));
    patternDisplayEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    patternDisplayEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    patternDisplayEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    patternDisplayEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5568));
    addAndMakeVisible(patternDisplayEditor);
    
    // Analysis Label - improved readability
    analysisLabel.setJustificationType(juce::Justification::centred);
    analysisLabel.setFont(juce::FontOptions(13.0f));
    analysisLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(analysisLabel);
    
    // Version Editor - copyable and selectable
    // Add timestamp to verify we're testing the right build (clean build 11:16)
    juce::String buildTimestamp = __DATE__ " " __TIME__;
    // Safe substring extraction to avoid String assertion
    juce::String dateStr = buildTimestamp.length() >= 6 ? buildTimestamp.substring(0, 6) : "unknown";
    juce::String timeStr = buildTimestamp.length() >= 17 ? buildTimestamp.substring(12, 17) : "00:00";
    versionEditor.setText("v1.3.5-" + dateStr + "-" + timeStr, juce::dontSendNotification);
    versionEditor.setMultiLine(false);
    versionEditor.setReadOnly(true);
    versionEditor.setScrollbarsShown(false);
    versionEditor.setCaretVisible(false);
    versionEditor.setJustification(juce::Justification::centredLeft);
    versionEditor.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    versionEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    versionEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    versionEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a5568));
    versionEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5568));
    addAndMakeVisible(versionEditor);
    
    // Documentation toggle button
    // TEMPORARILY DISABLED: WebView causing GPU process issues on iPadOS
    docsToggleButton.setButtonText("Docs");
    docsToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    docsToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    docsToggleButton.onClick = [this]() { /* toggleDocumentation(); */ }; // Disabled
    docsToggleButton.setEnabled(false); // Visually disabled
    addAndMakeVisible(docsToggleButton);
    
    // History toggle button (ticker tape feature)
    historyToggleButton.setButtonText("History");
    historyToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    historyToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    historyToggleButton.onClick = [this]() { toggleHistory(); };
    addAndMakeVisible(historyToggleButton);
    
    // History list setup
    historyListModel = std::make_unique<UPIHistoryListModel>(*this);
    upiHistoryList.setModel(historyListModel.get());
    upiHistoryList.setRowHeight(24);
    upiHistoryList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2d3748));
    upiHistoryList.setColour(juce::ListBox::textColourId, juce::Colours::white);
    addAndMakeVisible(upiHistoryList);
    upiHistoryList.setVisible(false);
    
    // History label
    historyLabel.setText("UPI History", juce::dontSendNotification);
    historyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    historyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(historyLabel);
    historyLabel.setVisible(false);
    
    // Preset browser toggle button
    presetToggleButton.setButtonText("Presets");
    presetToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    presetToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    presetToggleButton.setTooltip("Toggle preset browser");
    presetToggleButton.onClick = [this]() { togglePresets(); };
    addAndMakeVisible(presetToggleButton);
    
    // Preset list setup
    presetListModel = std::make_unique<PresetBrowserListModel>(*this);
    presetBrowserList.setModel(presetListModel.get());
    presetBrowserList.setRowHeight(24);
    presetBrowserList.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff2d3748));
    presetBrowserList.setColour(juce::ListBox::textColourId, juce::Colours::white);
    addAndMakeVisible(presetBrowserList);
    presetBrowserList.setVisible(false);
    
    // Preset label
    presetLabel.setText("Presets", juce::dontSendNotification);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    presetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetLabel);
    presetLabel.setVisible(false);
    
    // Preset management buttons
    savePresetButton.setButtonText("Save");
    savePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff48bb78));
    savePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    savePresetButton.setTooltip("Save current pattern as preset");
    savePresetButton.onClick = [this]() { showSavePresetDialog(); };
    addAndMakeVisible(savePresetButton);
    savePresetButton.setVisible(false);
    
    deletePresetButton.setButtonText("Delete");
    deletePresetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffef4444));
    deletePresetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    deletePresetButton.setTooltip("Delete selected preset");
    deletePresetButton.onClick = [this]() { deleteSelectedPreset(); };
    addAndMakeVisible(deletePresetButton);
    deletePresetButton.setVisible(false);
    
    // ========================================================================
    // iPad AUv3 PRESET SYSTEM: iOS Sandbox-Compatible Implementation
    // ========================================================================
    // PLATFORM DIFFERENCE: iPad uses sandbox-constrained preset management
    // - Desktop: Direct file system access to unrestricted directories
    // - iPad: Must use PresetManager with sandboxed app directories only
    // - Cannot programmatically create factory preset directories
    // - All preset operations require user interaction through file picker
    // FUTURE: Consider iOS-specific file sharing and document provider integration
    
    // iOS-compatible inline preset saving components
    presetNameEditor.setTextToShowWhenEmpty("Enter preset name...", juce::Colours::grey);
    presetNameEditor.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 14.0f, juce::Font::plain)));
    presetNameEditor.onReturnKey = [this]() { 
        auto name = presetNameEditor.getText().trim();
        if (name.isNotEmpty()) {
            saveCurrentPreset(name);
            // Hide inline input
            showingInlineInput = false;
            savePresetButton.setVisible(true);
            deletePresetButton.setVisible(true);
            presetNameEditor.setVisible(false);
            confirmSaveButton.setVisible(false);
            cancelSaveButton.setVisible(false);
            resized();
        }
    };
    addAndMakeVisible(presetNameEditor);
    presetNameEditor.setVisible(false);
    
    confirmSaveButton.setButtonText("OK");
    confirmSaveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff48bb78));
    confirmSaveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    confirmSaveButton.setTooltip("Save preset");
    confirmSaveButton.onClick = [this]() {
        auto name = presetNameEditor.getText().trim();
        if (name.isNotEmpty()) {
            saveCurrentPreset(name);
            // Hide inline input
            showingInlineInput = false;
            savePresetButton.setVisible(true);
            deletePresetButton.setVisible(true);
            presetNameEditor.setVisible(false);
            confirmSaveButton.setVisible(false);
            cancelSaveButton.setVisible(false);
            resized();
        }
    };
    addAndMakeVisible(confirmSaveButton);
    confirmSaveButton.setVisible(false);
    
    cancelSaveButton.setButtonText("X");
    cancelSaveButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffef4444));
    cancelSaveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    cancelSaveButton.setTooltip("Cancel");
    cancelSaveButton.onClick = [this]() {
        // Hide inline input
        showingInlineInput = false;
        savePresetButton.setVisible(true);
        deletePresetButton.setVisible(true);
        presetNameEditor.setVisible(false);
        confirmSaveButton.setVisible(false);
        cancelSaveButton.setVisible(false);
        resized();
    };
    addAndMakeVisible(cancelSaveButton);
    cancelSaveButton.setVisible(false);
    
    // ========================================================================
    // iPad AUv3 WEBVIEW LIMITATION: Currently Disabled
    // ========================================================================
    // ISSUE: WebView causes GPU process warnings and CFNetwork errors on iPadOS
    // SYMPTOMS: 
    //   - "LLDB is likely reading from device memory to resolve symbols"
    //   - GPU process isolation warnings
    //   - CFNetwork connection errors
    // DESKTOP: WebView works normally for inline documentation
    // iPad LIMITATION: iOS WebView has stricter process isolation
    // FUTURE SOLUTION: Consider native iOS WebView or file-based documentation
    
    // Initialize WebView documentation (currently disabled on iPad)
#if JUCE_WEB_BROWSER && 0  // Disabled until iOS WebView issues are resolved
    docsBrowser = std::make_unique<juce::WebBrowserComponent>();
    docsBrowser->setVisible(false); // Explicitly hidden initially
    addAndMakeVisible(*docsBrowser);
    // Don't load HTML immediately - wait until first toggle to avoid CFNetwork error
#endif
    
    // Close buttons for overlay views
    docsCloseButton.setButtonText("X");
    docsCloseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    docsCloseButton.setColour(juce::TextButton::textColourOffId, juce::Colours::red);
    docsCloseButton.setTooltip("Close documentation");
    docsCloseButton.onClick = [this]() { closeDocumentation(); };
    addAndMakeVisible(docsCloseButton);
    docsCloseButton.setVisible(false);
    
    historyCloseButton.setButtonText("X");
    historyCloseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    historyCloseButton.setColour(juce::TextButton::textColourOffId, juce::Colours::red);
    historyCloseButton.setTooltip("Close history");
    historyCloseButton.onClick = [this]() { closeHistory(); };
    addAndMakeVisible(historyCloseButton);
    historyCloseButton.setVisible(false);
    
    presetCloseButton.setButtonText("X");
    presetCloseButton.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    presetCloseButton.setColour(juce::TextButton::textColourOffId, juce::Colours::red);
    presetCloseButton.setTooltip("Close presets");
    presetCloseButton.onClick = [this]() { closePresets(); };
    addAndMakeVisible(presetCloseButton);
    presetCloseButton.setVisible(false);
    
    // Step Counter Display - removed for clean production interface
    
    
    
    
    tickButton.onClick = [this]()
    {
        // Only trigger the parameter - this will handle the advancement properly
        if (audioProcessor.getTickParameter()) {
            audioProcessor.getTickParameter()->setValueNotifyingHost(1.0f); // Trigger tick parameter
        }
        // Note: parseUPIPattern() removed to avoid double-triggering
        // The parameter change will handle the advancement through the host
    };
    
    // Initial display update
    updatePatternDisplay();
    updateAnalysisDisplay();
    
    // Initialize background color from processor state
    currentBackgroundColor = static_cast<BackgroundColor>(audioProcessor.getCurrentBackgroundColor());
    
    // Trigger pattern update to ensure restored patterns are applied
    audioProcessor.triggerPatternUpdate();
    
    // Start timer for regular UI updates
    startTimer(16); // Update every 16ms (~60fps) for even smoother animation
    
}

RhythmPatternExplorerAudioProcessorEditor::~RhythmPatternExplorerAudioProcessorEditor()
{
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background with color option
    g.fillAll(getBackgroundColour());
    
    // Title (only in normal mode)
    if (!minimalMode)
    {
        g.setColour (juce::Colours::white);
        g.setFont (juce::FontOptions(18.0f).withStyle("Bold"));
        g.drawText ("Rhythm Pattern Explorer", 0, 10, getWidth(), 30, 
                    juce::Justification::centred);
    }
    
    // Pattern circle area - now uses dynamic sizing from resized()
    if (!circleArea.isEmpty())
        drawPatternCircle(g, circleArea);
}

void RhythmPatternExplorerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Check for minimal mode (Easter egg for very small windows)
    bool shouldBeMinimal = (getWidth() <= MINIMAL_MODE_THRESHOLD || getHeight() <= MINIMAL_MODE_THRESHOLD);
    
    if (shouldBeMinimal != minimalMode)
    {
        minimalMode = shouldBeMinimal;
        // Force all controls to update visibility
        upiLabel.setVisible(!minimalMode);
        upiTextEditor.setVisible(!minimalMode);
        tickButton.setVisible(!minimalMode);
        patternDisplayEditor.setVisible(!minimalMode);
        docsToggleButton.setVisible(!minimalMode);
        historyToggleButton.setVisible(!minimalMode);
        presetToggleButton.setVisible(!minimalMode);
        versionEditor.setVisible(!minimalMode);
        
        
    }
    
    if (minimalMode)
    {
        // MINIMAL MODE: Just the circle, maximum size
        circleArea = getLocalBounds().reduced(5); // Small margin for visual breathing room
        
        // Hide WebView in minimal mode
#if JUCE_WEB_BROWSER
        if (docsBrowser)
            docsBrowser->setVisible(false);
#endif
        return; // Early exit - no other layout needed
    }
    
    // NORMAL MODE: Full UI layout
    
    // Title area
    area.removeFromTop(50);
    
    // Control area - compact layout with UPI input and controls
    auto controlArea = area.removeFromTop(70);
    controlArea.reduce(20, 10);
    
    // UPI Pattern Input row - primary control with compact extras
    auto upiRow = controlArea.removeFromTop(40);
    
    // Calculate available space and determine responsive layout
    int availableWidth = upiRow.getWidth();
    int upiLabelWidth = 40; // Reduced from 100 for "UPI:" instead of "UPI Pattern:"
    int minUpiWidth = 120; // Minimum width for UPI text editor
    int tickButtonWidth = 40; // Smaller for step/scene number display
    int nameFieldWidth = 90;
    int noteFieldWidth = 110; // Label + slider
    
    
    // Responsive layout: hide controls progressively when space is limited
    bool showTickButton = availableWidth >= (upiLabelWidth + minUpiWidth + tickButtonWidth + 30);
    bool showNameField = availableWidth >= (upiLabelWidth + minUpiWidth + (showTickButton ? tickButtonWidth : 0) + nameFieldWidth + 40);
    bool showNoteField = availableWidth >= (upiLabelWidth + minUpiWidth + (showTickButton ? tickButtonWidth : 0) + 
                                           (showNameField ? nameFieldWidth : 0) + noteFieldWidth + 50);
    
    // Set visibility based on available space
    tickButton.setVisible(showTickButton);
    
    // Layout UPI label (always visible)
    upiLabel.setBounds(upiRow.removeFromLeft(upiLabelWidth));
    
    // Calculate space for right controls
    int rightControlsWidth = 0;
    if (showTickButton) rightControlsWidth += tickButtonWidth + 10;
    if (showNameField) rightControlsWidth += nameFieldWidth + 10;
    if (showNoteField) rightControlsWidth += noteFieldWidth + 10;
    
    // Layout right controls if there's space
    juce::Rectangle<int> rightControls;
    if (rightControlsWidth > 0 && upiRow.getWidth() > rightControlsWidth)
    {
        rightControls = upiRow.removeFromRight(rightControlsWidth);
        
        if (showTickButton)
        {
            tickButton.setBounds(rightControls.removeFromLeft(tickButtonWidth).reduced(5));
            rightControls.removeFromLeft(10); // spacing
        }
        
        
    }
    
    
    // UPI text field gets remaining space
    upiTextEditor.setBounds(upiRow.reduced(5));
    
    
    // Pattern display area (text results) - increased size for onset/accent info
    auto displayArea = area.removeFromTop(80);
    patternDisplayEditor.setBounds(displayArea.reduced(10));
    
    // Button area - always visible, positioned right after pattern display, aligned right
    auto buttonArea = area.removeFromTop(30);
    presetToggleButton.setBounds(buttonArea.removeFromRight(80).reduced(5));
    historyToggleButton.setBounds(buttonArea.removeFromRight(80).reduced(5));
    docsToggleButton.setBounds(buttonArea.removeFromRight(80).reduced(5));
    
    // Analysis area - hidden
    auto analysisArea = area.removeFromTop(0);
    analysisLabel.setBounds(analysisArea.reduced(10));
    
    // Version display at bottom left (only when not showing docs)
    if (!showingDocs)
    {
        auto bottomArea = getLocalBounds().removeFromBottom(25);
        versionEditor.setBounds(bottomArea.removeFromLeft(100));
    }
    
    // History sidebar layout (do this BEFORE circle area calculation)
    if (showingHistory)
    {
        // When history is showing, create a sidebar on the right
        auto historyArea = getLocalBounds();
        historyArea.removeFromTop(50); // Leave space for title
        historyArea.removeFromBottom(30); // Leave space for main button area
        
        // Sidebar takes 250px on the right - this reduces the available area
        auto sidebar = historyArea.removeFromRight(250);
        
        // History label and close button at top of sidebar
        auto headerArea = sidebar.removeFromTop(25);
        historyLabel.setBounds(headerArea.removeFromLeft(headerArea.getWidth() - 30));
        historyCloseButton.setBounds(headerArea.reduced(2));
        
        // History list takes remaining sidebar space
        upiHistoryList.setBounds(sidebar.reduced(5));
        
        // Adjust the remaining area for circle (account for sidebar)
        area = area.withTrimmedRight(250);
    }
    
    // Preset browser sidebar layout (do this BEFORE circle area calculation)
    if (showingPresets)
    {
        // When presets are showing, create a sidebar on the right
        auto presetArea = getLocalBounds();
        presetArea.removeFromTop(50); // Leave space for title
        presetArea.removeFromBottom(30); // Leave space for main button area
        
        // Sidebar takes 250px on the right - this reduces the available area
        auto sidebar = presetArea.removeFromRight(250);
        
        // Preset label and close button at top of sidebar
        auto headerArea = sidebar.removeFromTop(25);
        presetLabel.setBounds(headerArea.removeFromLeft(headerArea.getWidth() - 30));
        presetCloseButton.setBounds(headerArea.reduced(2));
        
        // Preset management buttons below label
        auto buttonRow = sidebar.removeFromTop(30);
        
        if (showingInlineInput)
        {
            // Layout for inline input: text editor with confirm/cancel buttons
            auto inputArea = buttonRow.removeFromLeft(180);
            presetNameEditor.setBounds(inputArea.reduced(2));
            
            auto confirmArea = buttonRow.removeFromLeft(30);
            confirmSaveButton.setBounds(confirmArea.reduced(2));
            
            cancelSaveButton.setBounds(buttonRow.reduced(2));
        }
        else
        {
            // Normal layout: save and delete buttons
            savePresetButton.setBounds(buttonRow.removeFromLeft(120).reduced(2));
            deletePresetButton.setBounds(buttonRow.reduced(2));
        }
        
        // Preset list takes remaining sidebar space
        presetBrowserList.setBounds(sidebar.reduced(5));
        
        // Adjust the remaining area for circle (account for sidebar)
        area = area.withTrimmedRight(250);
    }
    
    // Remaining area is for the circle - MAXIMIZED for clean interface
    circleArea = area.expanded(100);
    
    // WebView documentation area (full plugin area when shown)
#if JUCE_WEB_BROWSER
    if (docsBrowser)
    {
        if (showingDocs)
        {
            // When docs are showing, take over most of the plugin area
            auto docsArea = getLocalBounds();
            docsArea.removeFromTop(50); // Leave space for title
            docsArea.removeFromBottom(30); // Leave space for main button area
            
            // Position close button in top-right corner
            auto topRightArea = docsArea.removeFromTop(30);
            docsCloseButton.setBounds(topRightArea.removeFromRight(30).reduced(2));
            
            docsBrowser->setBounds(docsArea);
        }
        docsBrowser->setVisible(showingDocs && !minimalMode);
    }
#endif
}

void RhythmPatternExplorerAudioProcessorEditor::timerCallback()
{
    // Update pattern display and animation
    static auto lastUpdateHash = std::hash<std::string>{}("");
    static int lastCurrentStep = -1;
    static bool lastPlayingState = false;
    static int frameCount = 0;
    
    frameCount++;
    
    // Handle click animation countdown
    if (clickAnimationFrames > 0)
    {
        clickAnimationFrames--;
        if (clickAnimationFrames == 0)
        {
            clickedStepIndex = -1; // Clear animation state
        }
        repaint(); // Trigger redraw for animation
    }
    
    // Sync UI sliders with parameter values (for host automation support)
    if (audioProcessor.getMidiNoteParameter()) {
    }
    
    
    // Update step/scene button text
    updateStepSceneButton();
    
    
    auto currentHash = std::hash<std::string>{}(audioProcessor.getPatternEngine().getBinaryString().toStdString());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Step counter display removed for clean production interface
    
    
    // Always repaint when playing to ensure smooth animation
    bool shouldRepaint = false;
    
    // Check for pattern changes (including accent pattern changes)
    bool patternChanged = audioProcessor.checkPatternChanged(); // This will return true if pattern changed and reset the flag
    
    if (currentHash != lastUpdateHash || patternChanged)
    {
        updatePatternDisplay();
        updateAnalysisDisplay();
        shouldRepaint = true;
        // Pattern changed - forcing repaint
    }
    
    
    if (currentStep != lastCurrentStep)
    {
        shouldRepaint = true;
        // Current step changed - clean production version
    }
    
    // Force repaint during playback for smooth animation
    if (isPlaying)
    {
        shouldRepaint = true;
        // Force repaint during playback for smooth animation
    }
    
    // Animation test code removed for clean production version
    
    if (isPlaying != lastPlayingState)
    {
        shouldRepaint = true;
    }
    
    // Force repaint every frame when playing for smooth animation
    if (isPlaying)
    {
        shouldRepaint = true;
    }
    
    if (shouldRepaint)
    {
        repaint();
    }
    
    lastUpdateHash = currentHash;
    lastCurrentStep = currentStep;
    lastPlayingState = isPlaying;
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    if (pattern.empty())
        return;
    
    auto center = bounds.getCentre();
    float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.3f;
    float innerRadius = maxRadius * 0.15f;  // Smaller inner hole for more room for step slices
    float outerRadius = maxRadius;
    float markerRadius = maxRadius * 0.85f;
    
    int numSteps = static_cast<int>(pattern.size());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    
    // Draw the background circle first - white for green theme, background color for others
    if (currentBackgroundColor == BackgroundColor::Green)
    {
        g.setColour(juce::Colours::white);
    }
    else
    {
        g.setColour(getBackgroundColour());
    }
    g.fillEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2);
    
    // Get accent map for educational visualization (single source of truth)
    auto accentMap = audioProcessor.getCurrentAccentMap();
    
    // Pattern visualization with accent support
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        // Start at 12 o'clock (north) and center slice 0 there: offset by half slice angle
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi - (sliceAngle * 0.5f);
        
        // Only draw onset slices
        if (pattern[i])
        {
            // Create simple onset slice
            juce::Path slice;
            slice.startNewSubPath(center.x + innerRadius * std::cos(startAngle), 
                                 center.y + innerRadius * std::sin(startAngle));
            
            int numSegments = std::max(8, int(sliceAngle * 20));
            for (int seg = 0; seg <= numSegments; ++seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + outerRadius * std::cos(angle);
                float y = center.y + outerRadius * std::sin(angle);
                slice.lineTo(x, y);
            }
            
            // Back along inner arc
            for (int seg = numSegments; seg >= 0; --seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + innerRadius * std::cos(angle);
                float y = center.y + innerRadius * std::sin(angle);
                slice.lineTo(x, y);
            }
            slice.closeSubPath();
            
            // Enhanced accent visualization with radial split and bold outline
            bool isAccented = (i < accentMap.size()) ? accentMap[i] : false;
            bool isHovered = (hoveredStepIndex == i);
            bool isClicked = (clickedStepIndex == i && clickAnimationFrames > 0);
            
            if (isAccented) {
                // ACCENTED ONSET: Radial split with contrasting colors
                
                // Calculate mid-radius for split
                float midRadius = (innerRadius + outerRadius) * 0.5f;
                
                // Inner half: Green (same as unaccented)
                juce::Path innerHalf;
                innerHalf.startNewSubPath(center.x + innerRadius * std::cos(startAngle), 
                                         center.y + innerRadius * std::sin(startAngle));
                
                int numSegments = std::max(8, int(sliceAngle * 20));
                for (int seg = 0; seg <= numSegments; ++seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + midRadius * std::cos(angle);
                    float y = center.y + midRadius * std::sin(angle);
                    innerHalf.lineTo(x, y);
                }
                
                // Back along inner arc
                for (int seg = numSegments; seg >= 0; --seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + innerRadius * std::cos(angle);
                    float y = center.y + innerRadius * std::sin(angle);
                    innerHalf.lineTo(x, y);
                }
                innerHalf.closeSubPath();
                
                // Fill inner half with adaptive base color (brightened if hovered or clicked)
                juce::Colour innerColor = PatternColors::getUnaccentedColor(getCurrentBackgroundColor());
                if (isClicked) {
                    // Click animation: bright flash that fades
                    float intensity = (float)clickAnimationFrames / 8.0f;
                    innerColor = innerColor.brighter(0.6f * intensity);
                } else if (isHovered) {
                    innerColor = innerColor.brighter(0.3f);
                }
                g.setColour(innerColor);
                g.fillPath(innerHalf);
                
                // Outer half: Warm amber accent color
                juce::Path outerHalf;
                outerHalf.startNewSubPath(center.x + midRadius * std::cos(startAngle), 
                                         center.y + midRadius * std::sin(startAngle));
                
                for (int seg = 0; seg <= numSegments; ++seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + outerRadius * std::cos(angle);
                    float y = center.y + outerRadius * std::sin(angle);
                    outerHalf.lineTo(x, y);
                }
                
                // Back along mid arc
                for (int seg = numSegments; seg >= 0; --seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + midRadius * std::cos(angle);
                    float y = center.y + midRadius * std::sin(angle);
                    outerHalf.lineTo(x, y);
                }
                outerHalf.closeSubPath();
                
                // Fill outer half with adaptive accent color (brightened if hovered or clicked)
                juce::Colour accentColor = PatternColors::getAccentedColor(getCurrentBackgroundColor());
                if (isClicked) {
                    // Click animation: bright flash that fades
                    float intensity = (float)clickAnimationFrames / 8.0f;
                    accentColor = accentColor.brighter(0.6f * intensity);
                } else if (isHovered) {
                    accentColor = accentColor.brighter(0.3f);
                }
                g.setColour(accentColor);
                g.fillPath(outerHalf);
                
                // Bold outline for accented onsets - draw after filling
                g.setColour(PatternColors::ACCENT_OUTLINE);  // White outline for maximum contrast
                g.strokePath(slice, juce::PathStrokeType(3.0f));  // Thick outline
                
            } else {
                // UNACCENTED ONSET: Solid adaptive base color (brightened if hovered or clicked)
                juce::Colour unaccentedColor = PatternColors::getUnaccentedColor(getCurrentBackgroundColor());
                if (isClicked) {
                    // Click animation: bright flash that fades
                    float intensity = (float)clickAnimationFrames / 8.0f;
                    unaccentedColor = unaccentedColor.brighter(0.6f * intensity);
                } else if (isHovered) {
                    unaccentedColor = unaccentedColor.brighter(0.3f);
                }
                g.setColour(unaccentedColor);
                g.fillPath(slice);
            }
        }
        
        // Draw hover highlighting for rest steps (empty slices)
        if (!pattern[i] && (hoveredStepIndex == i || (clickedStepIndex == i && clickAnimationFrames > 0)))
        {
            // Create slice path for rest step
            juce::Path restSlice;
            restSlice.startNewSubPath(center.x + innerRadius * std::cos(startAngle), 
                                     center.y + innerRadius * std::sin(startAngle));
            
            int numSegments = std::max(8, int(sliceAngle * 20));
            for (int seg = 0; seg <= numSegments; ++seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + outerRadius * std::cos(angle);
                float y = center.y + outerRadius * std::sin(angle);
                restSlice.lineTo(x, y);
            }
            
            // Back along inner arc
            for (int seg = numSegments; seg >= 0; --seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + innerRadius * std::cos(angle);
                float y = center.y + innerRadius * std::sin(angle);
                restSlice.lineTo(x, y);
            }
            restSlice.closeSubPath();
            
            // Subtle highlight for hovered/clicked rest steps - semi-transparent overlay
            juce::Colour restColor = PatternColors::getUnaccentedColor(getCurrentBackgroundColor());
            if (clickedStepIndex == i && clickAnimationFrames > 0) {
                // Click animation: bright flash that fades
                float intensity = (float)clickAnimationFrames / 8.0f;
                restColor = restColor.brighter(0.4f * intensity).withAlpha(0.6f * intensity);
            } else {
                // Regular hover: subtle semi-transparent
                restColor = restColor.withAlpha(0.3f);
            }
            g.setColour(restColor);
            g.fillPath(restSlice);
        }
    }
    
    // Draw inner circle to create donut effect AFTER all slices
    if (innerRadius > 0)
    {
        g.setColour(getBackgroundColour()); // Same as background
        g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                     innerRadius * 2, innerRadius * 2);
    }
    
    // Draw playback highlighting OVER everything - use audio step for accuracy
    int highlightStep = currentStep; // Use actual audio step for highlighting
    
    // Make highlighting more persistent - show during playback or recent playback
    static int drawFrameCount = 0;
    drawFrameCount++;
    bool showHighlight = isPlaying || (drawFrameCount % 240 < 120); // Show for half the cycle even when stopped
    
    if (showHighlight && highlightStep >= 0 && highlightStep < numSteps)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        // Use same alignment as main slices: start at 12 o'clock and center slice 0 there
        float startAngle = (highlightStep * sliceAngle) - juce::MathConstants<float>::halfPi - (sliceAngle * 0.5f);
        // float endAngle = startAngle + sliceAngle; // Unused variable removed
        
        // Create highlight sector same way as regular slices
        juce::Path highlightSlice;
        highlightSlice.startNewSubPath(center.x, center.y);
        
        int numSegments = std::max(8, int(sliceAngle * 20));
        for (int seg = 0; seg <= numSegments; ++seg)
        {
            float angle = startAngle + (sliceAngle * seg / numSegments);
            float x = center.x + outerRadius * std::cos(angle);
            float y = center.y + outerRadius * std::sin(angle);
            highlightSlice.lineTo(x, y);
        }
        highlightSlice.closeSubPath();
        
        // Use thick outline instead of fill to preserve pattern state visibility
        juce::Colour highlightColor = isPlaying ? juce::Colour(0xffff6b35) : juce::Colour(0xffcccccc); // Orange when playing, light gray when stopped
        g.setColour(highlightColor);
        
        // Draw thick outline instead of filled area - preserves underlying pattern visibility
        g.strokePath(highlightSlice, juce::PathStrokeType(5.0f)); // Thick stroke for clear visibility
    }
    
    // Draw slice separator lines AFTER filling
    g.setColour(PatternColors::SEPARATOR_LINES);
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        // Lines between slices: offset by half slice to place between slice boundaries
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi + (sliceAngle * 0.5f);
        
        float lineStartX = center.x + innerRadius * std::cos(angle);
        float lineStartY = center.y + innerRadius * std::sin(angle);
        float lineEndX = center.x + outerRadius * std::cos(angle);
        float lineEndY = center.y + outerRadius * std::sin(angle);
        g.drawLine(lineStartX, lineStartY, lineEndX, lineEndY, 1.5f);
    }
    
    // Draw outer and inner circle outlines
    g.setColour(PatternColors::SEPARATOR_LINES);
    g.drawEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2, 2.0f);
    if (innerRadius > 0)
        g.drawEllipse(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2, 2.0f);
    
    // Draw step markers INSIDE the available space (only in normal mode)
    // Use webapp's modulo-based approach for even distribution
    if (!minimalMode)
    {
        // Helper function to determine if step marker should be shown (matches webapp logic)
        auto shouldShowStepNumber = [](int stepIndex, int numSteps) -> bool {
            if (numSteps <= 16) {
                return true;  // Show all numbers for 16 or fewer steps
            } else if (numSteps <= 32) {
                return stepIndex % 2 == 0;  // Show every 2nd step for 17-32 steps
            } else if (numSteps <= 48) {
                return stepIndex % 3 == 0;  // Show every 3rd step for 33-48 steps
            } else {
                return stepIndex % 4 == 0;  // Show every 4th step for 49+ steps
            }
        };
        
        for (int stepIndex = 0; stepIndex < numSteps; ++stepIndex)
        {
            if (!shouldShowStepNumber(stepIndex, numSteps)) continue;
            
            float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
            // Position marker at the CENTER of each slice, aligned with 12 o'clock
            float centerAngle = (stepIndex * sliceAngle) - juce::MathConstants<float>::halfPi;
            float x = center.x + markerRadius * std::cos(centerAngle);
            float y = center.y + markerRadius * std::sin(centerAngle);
            
            // Draw step marker circles
            g.setColour(PatternColors::STEP_MARKERS);
            g.fillEllipse(x - 8, y - 8, 16, 16);
            
            // Draw step numbers (show actual step index for larger patterns)
            juce::String stepNumber = juce::String(stepIndex);
            g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            g.setColour(PatternColors::STEP_MARKER_TEXT);
            
            // Center the text in the marker
            juce::Rectangle<float> textBounds(x - 8, y - 6, 16, 12);
            g.drawText(stepNumber, textBounds, juce::Justification::centred);
        }
    }
    
}

void RhythmPatternExplorerAudioProcessorEditor::updatePatternDisplay()
{
    juce::String binary = audioProcessor.getPatternEngine().getBinaryString();
    juce::String hex = audioProcessor.getPatternEngine().getHexString();
    juce::String octal = audioProcessor.getPatternEngine().getOctalString();
    juce::String decimal = audioProcessor.getPatternEngine().getDecimalString();
    juce::String description = audioProcessor.getPatternEngine().getPatternDescription();
    
    // Enhanced description for quantized patterns
    if (audioProcessor.getHasQuantization()) {
        int originalSteps = audioProcessor.getOriginalStepCount();
        int quantizedSteps = audioProcessor.getQuantizedStepCount();
        bool clockwise = audioProcessor.getQuantizationClockwise();
        juce::String directionText = clockwise ? juce::String::fromUTF8("↻") : juce::String::fromUTF8("↺");
        
        description += " [" + juce::String(originalSteps) + "→" + juce::String(quantizedSteps) + " " + directionText + "]";
    }
    
    // Get onset positions for display
    auto currentPattern = audioProcessor.getPatternEngine().getCurrentPattern();
    juce::String onsetPositions = "[";
    bool first = true;
    for (int i = 0; i < static_cast<int>(currentPattern.size()); ++i) {
        if (currentPattern[i]) {
            if (!first) onsetPositions += ",";
            onsetPositions += juce::String(i);
            first = false;
        }
    }
    onsetPositions += "]";
    
    // Get accent information if present
    juce::String accentInfo = "";
    if (audioProcessor.getHasAccentPattern()) {
        auto accentMap = audioProcessor.getCurrentAccentMap();
        juce::String accentPositions = "[";
        bool firstAccent = true;
        for (int i = 0; i < static_cast<int>(accentMap.size()); ++i) {
            if (accentMap[i]) {
                if (!firstAccent) accentPositions += ",";
                accentPositions += juce::String(i);
                firstAccent = false;
            }
        }
        accentPositions += "]";
        accentInfo = " | Accents: " + accentPositions;
    }
    
    // Display pattern with onsets, accents, and all notations on two lines
    juce::String displayText = binary + " | " + description + "\n" +
                              "Onsets: " + onsetPositions + accentInfo + " | " + hex + " | " + octal + " | " + decimal;
    
    patternDisplayEditor.setText(displayText, juce::dontSendNotification);
}

void RhythmPatternExplorerAudioProcessorEditor::updateAnalysisDisplay()
{
    juce::String analysisText = "";
    
    // Show quantization information if present
    if (audioProcessor.getHasQuantization()) {
        int originalSteps = audioProcessor.getOriginalStepCount();
        int quantizedSteps = audioProcessor.getQuantizedStepCount();
        bool clockwise = audioProcessor.getQuantizationClockwise();
        int originalOnsets = audioProcessor.getOriginalOnsetCount();
        int quantizedOnsets = audioProcessor.getQuantizedOnsetCount();
        
        juce::String directionText = clockwise ? juce::String::fromUTF8("↻") : juce::String::fromUTF8("↺");
        
        analysisText += "Quantization: " + juce::String(originalSteps) + "→" + juce::String(quantizedSteps) + " steps " + directionText + "\n";
        analysisText += "Onsets: " + juce::String(originalOnsets) + "→" + juce::String(quantizedOnsets) + " preserved";
    }
    
    analysisLabel.setText(analysisText, juce::dontSendNotification);
}

void RhythmPatternExplorerAudioProcessorEditor::updateStepSceneButton()
{
    juce::String buttonText;
    juce::String tooltip;
    
    // Check if we have scene cycling (multiple scenes)
    int sceneCount = audioProcessor.getSceneCount();
    if (sceneCount > 1)
    {
        // Scene cycling: show current scene index (1-based)
        int currentScene = audioProcessor.getCurrentSceneIndex() + 1; // 1-based for display
        buttonText = juce::String(currentScene);
        tooltip = "Scene " + juce::String(currentScene) + " of " + juce::String(sceneCount) + ". Click to advance to next scene.";
    }
    else if (audioProcessor.hasProgressiveOffset())
    {
        // Progressive transformation: show progression step (already 1-based from UPIParser)
        int progressionStep = audioProcessor.getProgressiveTriggerCount(); // Already 1-based
        buttonText = juce::String(progressionStep);
        tooltip = "Progressive step: " + juce::String(progressionStep) + ". Click to advance progression.";
    }
    else
    {
        // Regular pattern: show current step in pattern
        int currentStep = audioProcessor.getCurrentStep() + 1; // 1-based for display
        buttonText = juce::String(currentStep);
        tooltip = "Current step: " + juce::String(currentStep) + ". Click to advance pattern.";
    }
    
    tickButton.setButtonText(buttonText);
    tickButton.setTooltip(tooltip);
}

void RhythmPatternExplorerAudioProcessorEditor::parseUPIPattern()
{
    juce::String upiText = upiTextEditor.getText().trim();
    if (upiText.isEmpty())
        return;
    
    
    // Set the UPI input on the processor, which will parse and apply it
    audioProcessor.setUPIInput(upiText);
    
    // Update the UI to reflect the changes
    updatePatternDisplay();
    updateAnalysisDisplay();
    
    // Update accent controls visibility immediately after pattern parsing
    
    // Clear the text editor for next input (optional)
    // upiTextEditor.clear();
}

void RhythmPatternExplorerAudioProcessorEditor::onParseButtonClicked()
{
    parseUPIPattern();
}


void RhythmPatternExplorerAudioProcessorEditor::toggleDocumentation()
{
#if JUCE_WEB_BROWSER
    if (!docsBrowser) 
    {
        return;
    }
    
    // Close other views first (mutual exclusion)
    if (showingHistory) {
        showingHistory = false;
        upiHistoryList.setVisible(false);
        historyLabel.setVisible(false);
        historyToggleButton.setButtonText("History");
    }
    if (showingPresets) {
        showingPresets = false;
        presetBrowserList.setVisible(false);
        presetLabel.setVisible(false);
        savePresetButton.setVisible(false);
        deletePresetButton.setVisible(false);
        presetToggleButton.setButtonText("Presets");
    }
    
    showingDocs = !showingDocs;
    
    // Load HTML content on first use to avoid CFNetwork errors
    if (showingDocs)
    {
        createDocumentationHTML();
    }
    
    // Update button text immediately
    docsToggleButton.setButtonText(showingDocs ? "Pattern" : "Docs");
    
    // Update visibility immediately
    docsBrowser->setVisible(showingDocs);
    docsCloseButton.setVisible(showingDocs);
    
    // Force layout update
    resized();
    repaint();
#endif
}

void RhythmPatternExplorerAudioProcessorEditor::closeDocumentation()
{
#if JUCE_WEB_BROWSER
    if (!docsBrowser) return;
    
    showingDocs = false;
    docsToggleButton.setButtonText("Docs");
    docsBrowser->setVisible(false);
    docsCloseButton.setVisible(false);
    
    resized();
    repaint();
#endif
}

void RhythmPatternExplorerAudioProcessorEditor::createDocumentationHTML()
{
#if JUCE_WEB_BROWSER
    if (!docsBrowser) return;
    
    // Build HTML content using string concatenation to avoid raw string issues
    juce::String htmlContent;
    htmlContent += "<!DOCTYPE html>\n";
    htmlContent += "<html>\n<head>\n";
    htmlContent += "<meta charset=\"UTF-8\">\n";
    htmlContent += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    htmlContent += "<title>UPI Pattern Documentation</title>\n";
    htmlContent += "<style>\n";
    htmlContent += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; background: #2d3748; color: #e2e8f0; margin: 0; padding: 15px; line-height: 1.6; box-sizing: border-box; }\n";
    htmlContent += ".container { width: 100%; margin: 0 auto; overflow-x: hidden; }\n";
    htmlContent += "h1, h2, h3 { color: #48bb78; margin-top: 2em; }\n";
    htmlContent += "h1 { border-bottom: 2px solid #48bb78; padding-bottom: 0.5em; }\n";
    htmlContent += ".pattern-example { background: #1a202c; border: 1px solid #4a5568; border-radius: 8px; padding: 15px; margin: 15px 0; font-family: 'Monaco', 'Menlo', monospace; }\n";
    htmlContent += ".pattern-code { color: #68d391; font-weight: bold; font-size: 1.1em; }\n";
    htmlContent += ".pattern-description { color: #a0aec0; margin-top: 8px; }\n";
    htmlContent += ".syntax-highlight { color: #f7fafc; background: #2d3748; padding: 2px 6px; border-radius: 4px; font-family: monospace; }\n";
    htmlContent += ".quick-ref { background: #1a365d; border-left: 4px solid #3182ce; padding: 15px; margin: 20px 0; }\n";
    htmlContent += ".warning { background: #744210; border-left: 4px solid #d69e2e; padding: 15px; margin: 20px 0; }\n";
    htmlContent += "table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n";
    htmlContent += "th, td { border: 1px solid #4a5568; padding: 12px; text-align: left; }\n";
    htmlContent += "th { background: #1a202c; color: #48bb78; font-weight: bold; }\n";
    htmlContent += "tr:nth-child(even) { background: #1a202c; }\n";
    htmlContent += ".copy-btn { background: #48bb78; color: white; border: none; padding: 4px 8px; border-radius: 4px; cursor: pointer; font-size: 0.8em; margin-left: 10px; }\n";
    htmlContent += ".copy-btn:hover { background: #38a169; }\n";
    htmlContent += "</style>\n</head>\n<body>\n";
    htmlContent += "<div class=\"container\">\n";
    htmlContent += "<h1>Universal Pattern Interface (UPI) Documentation</h1>\n";
    htmlContent += "<div class=\"quick-ref\">\n";
    htmlContent += "<h3>Quick Reference</h3>\n";
    htmlContent += "<p>UPI provides a comprehensive language for rhythm patterns using Euclidean, Barlow, Wolrab, Dilcue, Polygon, and Random algorithms, plus hex/binary notation, quantization, accents, progressive transformations, and scene cycling.</p>\n";
    htmlContent += "<p><strong>Download:</strong> <a href=\"https://github.com/Enkerli/rhythm_pattern_explorer/releases\" target=\"_blank\" style=\"color: #68d391; text-decoration: underline;\">Latest Releases</a> | <a href=\"https://github.com/Enkerli/rhythm_pattern_explorer\" target=\"_blank\" style=\"color: #68d391; text-decoration: underline;\">Source Code</a></p>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Basic Patterns</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Euclidean: 3 onsets distributed evenly across 8 steps (classic tresillo)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">B(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Barlow: 3 onsets using indispensability theory (metric hierarchy)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">W(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Wolrab: 3 onsets using anti-Barlow (groove-oriented, anti-metric)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">D(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Dilcue: 3 onsets using anti-Euclidean distribution</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">P(5,0)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Polygon: Pentagon rhythm (5 equally spaced onsets)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">R(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Random: 3 randomly placed onsets across 8 steps</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Hex/Octal/Binary Notation</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">0x94:8</div>\n";
    htmlContent += "<div class=\"pattern-description\">Hex: Left-to-right bit mapping (0x94 = 10010010 tresillo)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">0o222:8</div>\n";
    htmlContent += "<div class=\"pattern-description\">Octal: Base-8 notation with left-to-right mapping</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">10010010</div>\n";
    htmlContent += "<div class=\"pattern-description\">Binary: Direct binary pattern (1=onset, 0=rest)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Lascabettes Quantization</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(5,17);13</div>\n";
    htmlContent += "<div class=\"pattern-description\">Quantize: Project 5 onsets from 17 steps onto 13 steps (clockwise ↻)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8);-12</div>\n";
    htmlContent += "<div class=\"pattern-description\">Quantize: Project onto 12 steps counterclockwise (negative = ↺)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Accent Patterns</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">{100}E(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Accent: Accent first onset of tresillo pattern</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">{E(2,5)}E(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Accent: Use Euclidean pattern as accent layer (polyrhythmic)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Progressive Transformations</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(1,8)>8</div>\n";
    htmlContent += "<div class=\"pattern-description\">Progressive: Transform from 1 to 8 onsets via Euclidean algorithm</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(1,8)B>8</div>\n";
    htmlContent += "<div class=\"pattern-description\">Progressive: Transform using Barlow indispensability</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">1000000+2</div>\n";
    htmlContent += "<div class=\"pattern-description\">Progressive Offset: Advance pattern by 2 steps each trigger</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)*3</div>\n";
    htmlContent += "<div class=\"pattern-description\">Progressive Lengthening: Add 3 random steps each trigger</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Scene Cycling</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)|B(5,13)|W(2,7)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Scenes: Cycle through multiple patterns with manual triggers</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">{101}E(3,8)|{110}B(5,13)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Scenes: Each scene can have unique accent patterns</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"warning\">\n";
    htmlContent += "<strong>Note:</strong> Progressive patterns and scenes advance only via manual triggers (Enter, Tick, MIDI input).\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Pattern Combinations</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8) + E(2,5)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Combine two Euclidean patterns using OR logic</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Complete Syntax Reference</h2>\n";
    htmlContent += "<table>\n";
    htmlContent += "<tr><th>Pattern Type</th><th>Syntax</th><th>Example</th><th>Description</th></tr>\n";
    htmlContent += "<tr><td>Euclidean</td><td>E(onsets,steps)</td><td>E(5,13)</td><td>Even distribution algorithm</td></tr>\n";
    htmlContent += "<tr><td>Barlow</td><td>B(onsets,steps)</td><td>B(3,8)</td><td>Metric indispensability</td></tr>\n";
    htmlContent += "<tr><td>Wolrab</td><td>W(onsets,steps)</td><td>W(3,8)</td><td>Anti-metric (groove)</td></tr>\n";
    htmlContent += "<tr><td>Dilcue</td><td>D(onsets,steps)</td><td>D(3,8)</td><td>Anti-Euclidean</td></tr>\n";
    htmlContent += "<tr><td>Polygon</td><td>P(sides,offset,steps?)</td><td>P(7,2,16)</td><td>Geometric patterns</td></tr>\n";
    htmlContent += "<tr><td>Random</td><td>R(onsets,steps)</td><td>R(3,8)</td><td>Random placement</td></tr>\n";
    htmlContent += "<tr><td>Binary</td><td>10110100</td><td>10010010</td><td>Direct binary input</td></tr>\n";
    htmlContent += "<tr><td>Hexadecimal</td><td>0xHEX:steps</td><td>0x94:8</td><td>Left-to-right mapping</td></tr>\n";
    htmlContent += "<tr><td>Octal</td><td>0oOCT:steps</td><td>0o222:8</td><td>Base-8 notation</td></tr>\n";
    htmlContent += "<tr><td>Quantization</td><td>Pattern;steps</td><td>E(5,17);13</td><td>Angular projection</td></tr>\n";
    htmlContent += "<tr><td>Accents</td><td>{accent}pattern</td><td>{100}E(3,8)</td><td>Suprasegmental layer</td></tr>\n";
    htmlContent += "<tr><td>Progressive Transform</td><td>Pattern>target</td><td>E(1,8)>8</td><td>Gradual evolution</td></tr>\n";
    htmlContent += "<tr><td>Progressive Offset</td><td>Pattern+step</td><td>E(3,8)+2</td><td>Rotation per trigger</td></tr>\n";
    htmlContent += "<tr><td>Progressive Length</td><td>Pattern*add</td><td>E(3,8)*3</td><td>Growth per trigger</td></tr>\n";
    htmlContent += "<tr><td>Scenes</td><td>Pat1|Pat2|Pat3</td><td>E(3,8)|B(5,13)</td><td>Manual cycling</td></tr>\n";
    htmlContent += "<tr><td>Combination</td><td>Pattern + Pattern</td><td>E(3,8) + B(2,5)</td><td>OR logic merge</td></tr>\n";
    htmlContent += "</table>\n";
    htmlContent += "<h2>Musical Examples</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Classic tresillo rhythm</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(5,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Cinquillo pattern</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Tips and Tricks</h2>\n";
    htmlContent += "<ul>\n";
    htmlContent += "<li><strong>MIDI Triggering:</strong> Any MIDI note advances progressive patterns and scenes</li>\n";
    htmlContent += "<li><strong>Hex Notation:</strong> 0x94:8 gives tresillo (10010010) - left bit = LSB</li>\n";
    htmlContent += "<li><strong>Quantization:</strong> E(5,17);13 projects complex rhythms onto simpler grids</li>\n";
    htmlContent += "<li><strong>Accents:</strong> {101}E(3,8) creates polyrhythmic accent patterns</li>\n";
    htmlContent += "<li><strong>Algorithm Comparison:</strong> Try E(3,8), B(3,8), W(3,8), D(3,8) for different feels</li>\n";
    htmlContent += "<li><strong>Progressive Looping:</strong> E(1,8)>8 cycles from sparse to dense and back</li>\n";
    htmlContent += "<li><strong>Scene Performance:</strong> Use | to create live-triggerable pattern sequences</li>\n";
    htmlContent += "<li><strong>Complex Combinations:</strong> {E(2,5)}E(3,8)|{B(1,3)}W(5,13) for evolving textures</li>\n";
    htmlContent += "</ul>\n";
    htmlContent += "<div class=\"quick-ref\">\n";
    htmlContent += "<h3>Getting Started</h3>\n";
    htmlContent += "<ol>\n";
    htmlContent += "<li>Try E(3,8) for a basic Euclidean rhythm</li>\n";
    htmlContent += "<li>Experiment with P(5,0) for polygon patterns</li>\n";
    htmlContent += "<li>Add + E(2,5) to combine patterns</li>\n";
    htmlContent += "<li>Use progressive patterns like E(3,8)E.8 for evolution</li>\n";
    htmlContent += "</ol>\n";
    htmlContent += "</div>\n";
    htmlContent += "</div>\n</body>\n</html>";
    
    // Load HTML content by writing to a temporary file
    juce::File tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    juce::File htmlFile = tempDir.getChildFile("rhythm_pattern_docs.html");
    
    if (htmlFile.replaceWithText(htmlContent))
    {
        juce::URL fileURL = juce::URL(htmlFile);
        docsBrowser->goToURL(fileURL.toString(false));
    }
    else
    {
        // Fallback: try direct HTML loading if temp file fails
        juce::String simpleHtml = "<html><head><style>body{background:#2d3748;color:#e2e8f0;font-family:Arial;padding:20px;}</style></head>";
        simpleHtml += "<body><h1 style='color:#48bb78'>UPI Documentation</h1>";
        simpleHtml += "<p>Documentation loading failed. Check console for errors.</p></body></html>";
        docsBrowser->goToURL("data:text/html," + juce::URL::addEscapeChars(simpleHtml, false));
    }
#endif
}

juce::Colour RhythmPatternExplorerAudioProcessorEditor::getBackgroundColour() const
{
    switch (currentBackgroundColor)
    {
        case BackgroundColor::Dark:   return juce::Colour(0xff2d3748);  // Current dark background
        case BackgroundColor::White:  return juce::Colours::white;      // White background
        case BackgroundColor::Green:  return juce::Colour(0xff48bb78);  // Same as onset color
        case BackgroundColor::Orange: return juce::Colour(0xffff6b35);  // Same as highlight color
        case BackgroundColor::Blue:   return juce::Colour(0xff4299e1);  // Contrasting blue
        case BackgroundColor::Purple: return juce::Colour(0xff9f7aea);  // Contrasting purple
        default: return juce::Colour(0xff2d3748);
    }
}


void RhythmPatternExplorerAudioProcessorEditor::cycleBackgroundColor()
{
    int current = static_cast<int>(currentBackgroundColor);
    current = (current + 1) % 6; // 6 total colors
    currentBackgroundColor = static_cast<BackgroundColor>(current);
    
    // Update processor with new background color for persistence
    audioProcessor.setCurrentBackgroundColor(current);
    
    repaint(); // Trigger redraw with new background color
}

void RhythmPatternExplorerAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    // Double-click anywhere to cycle background colors
    cycleBackgroundColor();
}

void RhythmPatternExplorerAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Handle single clicks on pattern circle for step toggling and accent control
    if (event.mods.isLeftButtonDown() && !circleArea.isEmpty())
    {
        int mouseX = event.getMouseDownX();
        int mouseY = event.getMouseDownY();
        
        // Get detailed click information including inner/outer half detection
        ClickResult clickResult = getStepClickDetails(mouseX, mouseY, circleArea);
        
        if (clickResult.stepIndex >= 0)
        {
            // Get current pattern to check if step is an onset
            auto currentPattern = audioProcessor.getPatternEngine().getCurrentPattern();
            bool isOnset = (clickResult.stepIndex < currentPattern.size()) && currentPattern[clickResult.stepIndex];
            
            if (!isOnset) {
                // Empty step: clicking anywhere creates an onset
                audioProcessor.togglePatternStep(clickResult.stepIndex);
            } else {
                // Existing onset: inner half toggles onset, outer half toggles accent
                if (clickResult.isInOuterHalf) {
                    // Outer half: toggle accent
                    audioProcessor.toggleAccentAtStep(clickResult.stepIndex);
                } else {
                    // Inner half: toggle onset (remove it)
                    audioProcessor.togglePatternStep(clickResult.stepIndex);
                }
            }
            
            // Start click animation
            clickedStepIndex = clickResult.stepIndex;
            clickAnimationFrames = 8; // Animation duration in frames
            
            // Force immediate UI update
            repaint();
        }
    }
}

void RhythmPatternExplorerAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
    // Update hover state for visual feedback
    if (!circleArea.isEmpty())
    {
        int mouseX = event.x;
        int mouseY = event.y;
        
        // Check if mouse is within the circle area and get step index
        int stepIndex = getStepIndexFromCoordinates(mouseX, mouseY, circleArea);
        
        bool wasInCircleArea = isMouseInCircleArea;
        int previousHoveredStep = hoveredStepIndex;
        
        if (stepIndex >= 0)
        {
            // Mouse is over a valid step
            isMouseInCircleArea = true;
            hoveredStepIndex = stepIndex;
            
            // Change cursor to indicate interactivity
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
        else
        {
            // Mouse is not over a valid step
            isMouseInCircleArea = false;
            hoveredStepIndex = -1;
            
            // Restore default cursor
            setMouseCursor(juce::MouseCursor::NormalCursor);
        }
        
        // Repaint if hover state changed
        if (wasInCircleArea != isMouseInCircleArea || previousHoveredStep != hoveredStepIndex)
        {
            repaint();
        }
    }
}

void RhythmPatternExplorerAudioProcessorEditor::mouseExit(const juce::MouseEvent& event)
{
    // Clear hover state when mouse leaves the component
    bool needsRepaint = isMouseInCircleArea || hoveredStepIndex >= 0;
    
    isMouseInCircleArea = false;
    hoveredStepIndex = -1;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    
    if (needsRepaint)
    {
        repaint();
    }
}

//==============================================================================
// Pattern Editing via Mouse Clicks
//==============================================================================

int RhythmPatternExplorerAudioProcessorEditor::getStepIndexFromCoordinates(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const
{
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    int numSteps = static_cast<int>(pattern.size());
    
    if (numSteps <= 0) return -1; // Invalid pattern
    
    // Calculate circle dimensions (matching drawPatternCircle logic)
    juce::Point<float> center(circleArea.getCentreX(), circleArea.getCentreY());
    
    // Use same radius calculations as drawPatternCircle
    float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
    float outerRadius = radius;
    float innerRadius = radius * 0.15f; // 15% inner radius for smaller donut hole
    
    // Calculate distance from center
    float dx = mouseX - center.x;
    float dy = mouseY - center.y;
    float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
    
    // Check if click is within the ring (between inner and outer radius)
    if (distanceFromCenter < innerRadius || distanceFromCenter > outerRadius) {
        return -1; // Outside clickable area
    }
    
    // Calculate angle from center
    // atan2 returns angle in range [-π, π], we need [0, 2π]
    float angleFromCenter = std::atan2(dy, dx);
    if (angleFromCenter < 0) {
        angleFromCenter += 2.0f * juce::MathConstants<float>::pi;
    }
    
    // Adjust for 12 o'clock alignment (matching drawPatternCircle)
    // The pattern starts at 12 o'clock (north), so we need to offset by +π/2
    // This converts from standard math coordinates to our 12-o'clock-first system
    float adjustedAngle = angleFromCenter + juce::MathConstants<float>::halfPi;
    if (adjustedAngle >= 2.0f * juce::MathConstants<float>::pi) {
        adjustedAngle -= 2.0f * juce::MathConstants<float>::pi;
    }
    
    // Calculate slice angle and step index
    float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
    
    // Add half slice offset to ensure we're detecting the center of slices correctly
    adjustedAngle += sliceAngle * 0.5f;
    if (adjustedAngle >= 2.0f * juce::MathConstants<float>::pi) {
        adjustedAngle -= 2.0f * juce::MathConstants<float>::pi;
    }
    int stepIndex = static_cast<int>(adjustedAngle / sliceAngle);
    
    // Clamp to valid range
    if (stepIndex < 0) stepIndex = 0;
    if (stepIndex >= numSteps) stepIndex = numSteps - 1;
    
    return stepIndex;
}

bool RhythmPatternExplorerAudioProcessorEditor::isCoordinateInCircleArea(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const
{
    return getStepIndexFromCoordinates(mouseX, mouseY, circleArea) >= 0;
}

RhythmPatternExplorerAudioProcessorEditor::ClickResult RhythmPatternExplorerAudioProcessorEditor::getStepClickDetails(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const
{
    ClickResult result;
    
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    int numSteps = static_cast<int>(pattern.size());
    
    if (numSteps <= 0) return result; // Invalid pattern
    
    // Calculate circle dimensions (matching drawPatternCircle logic)
    juce::Point<float> center(circleArea.getCentreX(), circleArea.getCentreY());
    
    // Use same radius calculations as drawPatternCircle
    float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
    float outerRadius = radius;
    float innerRadius = radius * 0.15f; // 15% inner radius for smaller donut hole
    
    // IMPROVED TOUCH TARGETS: Make outer half larger for easier accent clicking
    // Split the ring 70% outer (accent) / 30% inner (onset toggle) instead of 50/50
    float midRadius = innerRadius + (outerRadius - innerRadius) * 0.3f; // 70% is outer half
    
    // Calculate distance from center
    float dx = mouseX - center.x;
    float dy = mouseY - center.y;
    float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
    
    // Check if click is within the ring (between inner and outer radius)
    if (distanceFromCenter < innerRadius || distanceFromCenter > outerRadius) {
        return result; // Outside clickable area
    }
    
    // Calculate angle from center
    float angleFromCenter = std::atan2(dy, dx);
    if (angleFromCenter < 0) {
        angleFromCenter += 2.0f * juce::MathConstants<float>::pi;
    }
    
    // Adjust for 12 o'clock alignment (matching drawPatternCircle)
    float adjustedAngle = angleFromCenter + juce::MathConstants<float>::halfPi;
    if (adjustedAngle >= 2.0f * juce::MathConstants<float>::pi) {
        adjustedAngle -= 2.0f * juce::MathConstants<float>::pi;
    }
    
    // Calculate slice angle and step index
    float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
    adjustedAngle += sliceAngle * 0.5f;
    if (adjustedAngle >= 2.0f * juce::MathConstants<float>::pi) {
        adjustedAngle -= 2.0f * juce::MathConstants<float>::pi;
    }
    int stepIndex = static_cast<int>(adjustedAngle / sliceAngle);
    
    // Clamp to valid range
    if (stepIndex < 0) stepIndex = 0;
    if (stepIndex >= numSteps) stepIndex = numSteps - 1;
    
    // Determine if click is in outer half (accent area)
    bool isInOuterHalf = distanceFromCenter > midRadius;
    
    result.stepIndex = stepIndex;
    result.isInOuterHalf = isInOuterHalf;
    return result;
}

//==============================================================================
// UPI History (Ticker Tape) Implementation

void RhythmPatternExplorerAudioProcessorEditor::toggleHistory()
{
    // Close other views first (mutual exclusion)
    if (showingDocs) {
        showingDocs = false;
#if JUCE_WEB_BROWSER
        if (docsBrowser) docsBrowser->setVisible(false);
#endif
        docsToggleButton.setButtonText("Docs");
    }
    if (showingPresets) {
        showingPresets = false;
        presetBrowserList.setVisible(false);
        presetLabel.setVisible(false);
        savePresetButton.setVisible(false);
        deletePresetButton.setVisible(false);
        presetToggleButton.setButtonText("Presets");
    }
    
    showingHistory = !showingHistory;
    
    upiHistoryList.setVisible(showingHistory);
    historyLabel.setVisible(showingHistory);
    historyCloseButton.setVisible(showingHistory);
    historyToggleButton.setButtonText(showingHistory ? "Pattern" : "History");
    
    // Update list content
    if (showingHistory)
    {
        upiHistoryList.updateContent();
    }
    
    // Force immediate layout recalculation
    resized();
    
    // Force immediate repaint to prevent misalignment
    repaint();
}

void RhythmPatternExplorerAudioProcessorEditor::closeHistory()
{
    showingHistory = false;
    historyToggleButton.setButtonText("History");
    upiHistoryList.setVisible(false);
    historyLabel.setVisible(false);
    historyCloseButton.setVisible(false);
    
    resized();
    repaint();
}

void RhythmPatternExplorerAudioProcessorEditor::onHistoryItemClicked(int index)
{
    const auto& history = audioProcessor.getUPIHistory();
    if (index >= 0 && index < history.size())
    {
        // Set the UPI text and apply the pattern
        upiTextEditor.setText(history[index], juce::dontSendNotification);
        audioProcessor.setUPIInput(history[index]);
    }
}

//==============================================================================
// UPI History List Model Implementation

int UPIHistoryListModel::getNumRows()
{
    return editorRef.getAudioProcessor().getUPIHistory().size();
}

void UPIHistoryListModel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    const auto& history = editorRef.getAudioProcessor().getUPIHistory();
    if (rowNumber >= 0 && rowNumber < history.size())
    {
        // Background
        if (rowIsSelected)
            g.setColour(juce::Colour(0xff4a5568));
        else
            g.setColour(juce::Colour(0xff2d3748));
        g.fillRect(0, 0, width, height);
        
        // Text
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 12.0f, juce::Font::plain)));
        
        juce::String text = history[rowNumber];
        // Truncate if too long
        if (text.length() > 25)
            text = text.substring(0, 22) + "...";
        
        g.drawText(text, 5, 0, width - 10, height, juce::Justification::centredLeft);
    }
}

void UPIHistoryListModel::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    editorRef.onHistoryItemClicked(row);
}

//==============================================================================
// Preset Browser List Model Implementation

int PresetBrowserListModel::getNumRows()
{
    return currentPresetNames.size();
}

void PresetBrowserListModel::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber >= 0 && rowNumber < currentPresetNames.size())
    {
        // Background
        if (rowIsSelected)
            g.setColour(juce::Colour(0xff4a5568));
        else
            g.setColour(juce::Colour(0xff2d3748));
        g.fillRect(0, 0, width, height);
        
        // Get preset data for feature analysis
        auto& presetManager = editorRef.getAudioProcessor().getPresetManager();
        juce::String presetName = currentPresetNames[rowNumber];
        juce::ValueTree presetState;
        juce::String upiPattern = "";
        
        // Load preset to get UPI pattern for analysis
        if (presetManager.loadPreset(presetName, presetState))
        {
            // Check for both possible property names
            if (presetState.hasProperty("currentUPIInput"))
                upiPattern = presetState.getProperty("currentUPIInput").toString();
            else if (presetState.hasProperty("upiInput"))
                upiPattern = presetState.getProperty("upiInput").toString();
        }
        
        // Layout: Name | Icons (80px) | Factory (15px)  
        int nameWidth = width - 95;
        int iconAreaX = nameWidth;
        int iconWidth = 18;
        int iconY = (height - 12) / 2;
        
        // Preset name (truncated if needed)
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 12.0f, juce::Font::plain)));
        
        juce::String text = presetName;
        if (text.length() > 18)  // Shorter to make room for icons
            text = text.substring(0, 15) + "...";
        
        g.drawText(text, 5, 0, nameWidth - 5, height, juce::Justification::centredLeft);
        
        // Feature icons (right-aligned)
        int currentIconX = iconAreaX;
        
        // Scenes icon: ▸ (if contains |) - more visible than 🎬
        if (upiPattern.contains("|"))
        {
            g.setColour(juce::Colour(0xff00ff00)); // Bright green
            g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::bold)));
            g.drawText(juce::CharPointer_UTF8("\xE2\x96\xB8"), currentIconX, iconY, iconWidth, 12, juce::Justification::centred);
            currentIconX += iconWidth;
        }
        
        // Progressive icon: ⚡ (if contains >) - lightning bolt is good
        if (upiPattern.contains(">"))
        {
            g.setColour(juce::Colour(0xffffff00)); // Bright yellow
            g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::plain)));
            g.drawText(juce::CharPointer_UTF8("\xE2\x9A\xA1"), currentIconX, iconY, iconWidth, 12, juce::Justification::centred);
            currentIconX += iconWidth;
        }
        
        // Accents icon: ● (if contains { and }) - solid circle more visible than 🎵
        if (upiPattern.contains("{") && upiPattern.contains("}"))
        {
            g.setColour(juce::Colour(0xffff00ff)); // Bright magenta
            g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 10.0f, juce::Font::bold)));
            g.drawText(juce::CharPointer_UTF8("\xE2\x97\x8F"), currentIconX, iconY, iconWidth, 12, juce::Justification::centred);
            currentIconX += iconWidth;
        }
        
        // Steps count (parse basic patterns for step count)
        if (!upiPattern.isEmpty())
        {
            int stepCount = parseStepCount(upiPattern);
            if (stepCount > 0)
            {
                g.setColour(juce::Colour(0xff90cdf4)); // Light blue
                g.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultSansSerifFontName(), 9.0f, juce::Font::bold)));
                g.drawText(juce::String(stepCount), currentIconX, iconY, iconWidth, 12, juce::Justification::centred);
            }
        }
        
        // Factory indicator (green dot)
        if (presetManager.isFactoryPreset(presetName))
        {
            g.setColour(juce::Colour(0xff48bb78)); // Green indicator
            g.fillEllipse(width - 12, height / 2 - 2, 4, 4);
        }
    }
}

void PresetBrowserListModel::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    editorRef.onPresetItemClicked(row);
}

void PresetBrowserListModel::refreshPresetList()
{
    const auto& presetManager = editorRef.getAudioProcessor().getPresetManager();
    currentPresetNames = presetManager.getPresetNames();
}

int PresetBrowserListModel::parseStepCount(const juce::String& upiPattern)
{
    if (upiPattern.isEmpty()) return 0;
    
    // Handle scenes - take first scene for step count
    juce::String pattern = upiPattern;
    if (pattern.contains("|"))
    {
        pattern = pattern.upToFirstOccurrenceOf("|", false, false).trim();
    }
    
    // Handle accents - extract main pattern
    if (pattern.contains("{") && pattern.contains("}"))
    {
        // Find the main pattern outside accent braces
        int braceStart = pattern.indexOf("{");
        int braceEnd = pattern.indexOf(braceStart, "}");
        if (braceEnd > braceStart)
        {
            // Take pattern after accent definition
            juce::String afterAccent = pattern.substring(braceEnd + 1).trim();
            if (!afterAccent.isEmpty())
                pattern = afterAccent;
        }
    }
    
    // Parse common pattern types
    // E(onsets,steps), B(onsets,steps), W(onsets,steps), D(onsets,steps), P(sides,offset)
    if (pattern.startsWithIgnoreCase("E(") || pattern.startsWithIgnoreCase("B(") || 
        pattern.startsWithIgnoreCase("W(") || pattern.startsWithIgnoreCase("D("))
    {
        int openParen = pattern.indexOf("(");
        int closeParen = pattern.indexOf(openParen, ")");
        if (closeParen > openParen)
        {
            juce::String params = pattern.substring(openParen + 1, closeParen);
            auto parts = juce::StringArray::fromTokens(params, ",", "");
            if (parts.size() >= 2)
            {
                return parts[1].trim().getIntValue(); // Return steps parameter
            }
        }
    }
    
    // P(sides,offset) - polygon patterns
    if (pattern.startsWithIgnoreCase("P("))
    {
        int openParen = pattern.indexOf("(");
        int closeParen = pattern.indexOf(openParen, ")");
        if (closeParen > openParen)
        {
            juce::String params = pattern.substring(openParen + 1, closeParen);
            auto parts = juce::StringArray::fromTokens(params, ",", "");
            if (parts.size() >= 1)
            {
                return parts[0].trim().getIntValue(); // Return sides parameter for polygons
            }
        }
    }
    
    // Hex notation: 0xHH:steps
    if (pattern.startsWith("0x") && pattern.contains(":"))
    {
        int colonPos = pattern.indexOf(":");
        if (colonPos > 2)
        {
            juce::String stepsStr = pattern.substring(colonPos + 1).trim();
            return stepsStr.getIntValue();
        }
    }
    
    // Binary patterns: assume common lengths
    if (pattern.containsOnly("01"))
    {
        return pattern.length();
    }
    
    // Default fallback
    return 0;
}

//==============================================================================
// Preset Browser UI Methods

void RhythmPatternExplorerAudioProcessorEditor::onPresetItemClicked(int index)
{
    auto& presetManager = audioProcessor.getPresetManager();
    auto presetNames = presetManager.getPresetNames();
    
    if (index >= 0 && index < presetNames.size())
    {
        juce::String presetName = presetNames[index];
        
        // Load preset state to get UPI pattern
        juce::ValueTree presetState;
        if (presetManager.loadPreset(presetName, presetState))
        {
            // Get the UPI pattern from the preset
            juce::String upiPattern = "";
            if (presetState.hasProperty("currentUPIInput"))
                upiPattern = presetState.getProperty("currentUPIInput").toString();
            else if (presetState.hasProperty("upiInput"))
                upiPattern = presetState.getProperty("upiInput").toString();
            
            // Check if this is a progressive offset pattern (contains +N)
            bool isProgressiveOffset = false;
            if (upiPattern.contains("+") && upiPattern.lastIndexOf("+") > 0) {
                int lastPlusIndex = upiPattern.lastIndexOf("+");
                juce::String afterPlus = upiPattern.substring(lastPlusIndex + 1).trim();
                isProgressiveOffset = afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
            }
            
            // Check if user is clicking the same progressive offset preset repeatedly
            static juce::String lastClickedPreset = "";
            bool isSameProgressivePreset = (presetName == lastClickedPreset) && isProgressiveOffset;
            
            if (isSameProgressivePreset)
            {
                // Advance the progressive offset by re-triggering the same UPI pattern
                // This will advance the progressiveOffset counter
                audioProcessor.setUPIInput(upiPattern);
            }
            else
            {
                // Different preset or not progressive - do full state load
                auto stateXml = presetState.createXml();
                if (stateXml)
                {
                    auto memoryBlock = juce::MemoryBlock();
                    audioProcessor.copyXmlToBinary(*stateXml, memoryBlock);
                    audioProcessor.setStateInformation(memoryBlock.getData(), static_cast<int>(memoryBlock.getSize()));
                }
            }
            
            // Remember this preset for next click
            lastClickedPreset = presetName;
            
            // Update UI to reflect current state
            upiTextEditor.setText(audioProcessor.getUPIInput(), juce::dontSendNotification);
            updatePatternDisplay();
            updateAnalysisDisplay();
        }
    }
}

void RhythmPatternExplorerAudioProcessorEditor::togglePresets()
{
    // Close other views first (mutual exclusion)
    if (showingDocs) {
        showingDocs = false;
#if JUCE_WEB_BROWSER
        if (docsBrowser) docsBrowser->setVisible(false);
#endif
        docsToggleButton.setButtonText("Docs");
    }
    if (showingHistory) {
        showingHistory = false;
        upiHistoryList.setVisible(false);
        historyLabel.setVisible(false);
        historyToggleButton.setButtonText("History");
    }
    
    showingPresets = !showingPresets;
    
    // Update preset list before showing
    if (showingPresets && presetListModel)
    {
        presetListModel->refreshPresetList();
        presetBrowserList.updateContent();
    }
    
    // Show/hide all preset components
    presetBrowserList.setVisible(showingPresets);
    presetLabel.setVisible(showingPresets);
    savePresetButton.setVisible(showingPresets);
    deletePresetButton.setVisible(showingPresets);
    presetCloseButton.setVisible(showingPresets);
    
    presetToggleButton.setButtonText(showingPresets ? "Pattern" : "Presets");
    
    resized(); // Force layout recalculation
    repaint(); // Force immediate repaint
}

void RhythmPatternExplorerAudioProcessorEditor::closePresets()
{
    showingPresets = false;
    presetToggleButton.setButtonText("Presets");
    presetBrowserList.setVisible(false);
    presetLabel.setVisible(false);
    savePresetButton.setVisible(false);
    deletePresetButton.setVisible(false);
    presetCloseButton.setVisible(false);
    
    resized();
    repaint();
}

void RhythmPatternExplorerAudioProcessorEditor::showSavePresetDialog()
{
    #if JUCE_IOS
        // iOS-compatible inline input method to avoid modal dialog freezing
        showSavePresetInlineInput();
    #else
        // Desktop modal dialog (working fine)
        auto alertWindow = new juce::AlertWindow("Save Preset", 
                                               "Enter preset name:", 
                                               juce::AlertWindow::QuestionIcon);
        
        alertWindow->addTextEditor("presetName", "", "Preset Name:");
        alertWindow->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
        alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        
        alertWindow->enterModalState(true, 
            juce::ModalCallbackFunction::create([this, alertWindow](int result)
            {
                if (result == 1) // Save button clicked
                {
                    auto presetName = alertWindow->getTextEditorContents("presetName");
                    if (presetName.isNotEmpty())
                    {
                        saveCurrentPreset(presetName);
                    }
                }
                delete alertWindow;
            }), true);
    #endif
}

void RhythmPatternExplorerAudioProcessorEditor::showSavePresetInlineInput()
{
    // Show inline input components
    showingInlineInput = true;
    
    // Hide regular preset buttons
    savePresetButton.setVisible(false);
    deletePresetButton.setVisible(false);
    
    // Show inline input components
    presetNameEditor.setVisible(true);
    confirmSaveButton.setVisible(true);
    cancelSaveButton.setVisible(true);
    
    // Focus the text editor
    presetNameEditor.setText("");
    presetNameEditor.grabKeyboardFocus();
    
    resized(); // Recalculate layout
}

void RhythmPatternExplorerAudioProcessorEditor::saveCurrentPreset(const juce::String& name)
{
    auto& presetManager = audioProcessor.getPresetManager();
    
    // Get current plugin state
    juce::MemoryBlock currentState;
    audioProcessor.getStateInformation(currentState);
    
    // Convert to ValueTree
    auto stateXml = audioProcessor.getXmlFromBinary(currentState.getData(), static_cast<int>(currentState.getSize()));
    if (stateXml)
    {
        auto stateTree = juce::ValueTree::fromXml(*stateXml);
        
        // Save preset
        juce::String category = "User";
        juce::String description = "User created preset";
        juce::String upiPattern = audioProcessor.getUPIInput();
        
        if (presetManager.savePreset(name, category, description, stateTree, upiPattern))
        {
            // Refresh preset list
            if (presetListModel)
            {
                presetListModel->refreshPresetList();
                presetBrowserList.updateContent();
            }
        }
    }
}

void RhythmPatternExplorerAudioProcessorEditor::deleteSelectedPreset()
{
    int selectedRow = presetBrowserList.getSelectedRow();
    if (selectedRow >= 0)
    {
        auto& presetManager = audioProcessor.getPresetManager();
        auto presetNames = presetManager.getPresetNames();
        
        if (selectedRow < presetNames.size())
        {
            juce::String presetName = presetNames[selectedRow];
            
            // Don't delete factory presets
            if (presetManager.isFactoryPreset(presetName))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::InfoIcon,
                    "Cannot Delete",
                    "Factory presets cannot be deleted.",
                    "OK"
                );
                return;
            }
            
            // Confirm deletion
            juce::AlertWindow::showAsync(
                juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::QuestionIcon)
                    .withTitle("Delete Preset")
                    .withMessage("Delete preset '" + presetName + "'?")
                    .withButton("Delete")
                    .withButton("Cancel"),
                [this, presetName](int result)
                {
                    if (result == 1) // Delete button clicked
                    {
                        auto& manager = audioProcessor.getPresetManager();
                        if (manager.deletePreset(presetName))
                        {
                            // Refresh preset list
                            if (presetListModel)
                            {
                                presetListModel->refreshPresetList();
                                presetBrowserList.updateContent();
                            }
                        }
                    }
                }
            );
        }
    }
}

