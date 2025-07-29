/*
  ==============================================================================

    Rhythm Pattern Explorer iPad - AUv3 Plugin
    Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RhythmPatternExplorerAudioProcessorEditor::RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up title label
    titleLabel.setText("Rhythm Pattern Explorer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up UPI input field
    upiInputField.setMultiLine(false);
    upiInputField.setReturnKeyStartsNewLine(false);
    upiInputField.setReadOnly(false);
    upiInputField.setScrollbarsShown(false);
    upiInputField.setCaretVisible(true);
    upiInputField.setPopupMenuEnabled(true);
    upiInputField.setText("E(3,8)", false);
    upiInputField.setFont(juce::Font(18.0f, juce::Font::plain));
    upiInputField.setColour(juce::TextEditor::backgroundColourId, juce::Colours::darkgrey);
    upiInputField.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    upiInputField.setColour(juce::TextEditor::outlineColourId, juce::Colours::lightblue);
    upiInputField.onTextChange = [this] { upiInputChanged(); };
    upiInputField.onReturnKey = [this] { triggerButtonClicked(); };
    
    // Add context menu for special characters
    upiInputField.setPopupMenuEnabled(true);
    addAndMakeVisible(upiInputField);
    
    // Set up trigger button with iPad-optimized sizing
    triggerButton.setButtonText("TRIGGER");
    triggerButton.onClick = [this] { triggerButtonClicked(); };
    triggerButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green.darker());
    triggerButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(triggerButton);
    
    
    // Set up pattern display
    patternDisplay.setText("X . . X . . X .", juce::dontSendNotification);
    patternDisplay.setFont(juce::Font(20.0f, juce::Font::plain));
    patternDisplay.setJustificationType(juce::Justification::centred);
    patternDisplay.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    addAndMakeVisible(patternDisplay);
    
    // Set up status label
    statusLabel.setText("Enter UPI pattern and tap TRIGGER", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    
    // Set up debug display - multi-line, scrollable, copyable
    debugDisplay.setText("Debug: Ready\nTiming info will appear here during playback", false);
    debugDisplay.setMultiLine(true);
    debugDisplay.setReadOnly(true);
    debugDisplay.setScrollbarsShown(true);
    debugDisplay.setCaretVisible(true);
    debugDisplay.setPopupMenuEnabled(true);
    debugDisplay.setFont(juce::Font("Courier New", 9.0f, juce::Font::plain));
    debugDisplay.setColour(juce::TextEditor::textColourId, juce::Colours::yellow);
    debugDisplay.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    debugDisplay.setColour(juce::TextEditor::outlineColourId, juce::Colours::grey);
    addAndMakeVisible(debugDisplay);
    
    // Set size suitable for iPad - responsive design
    setSize (500, 500); // Increased height for test results
    
    // Make resizable for different iPad orientations
    setResizable(true, true);
    setResizeLimits(400, 350, 800, 700);
    
    // Initialize pattern display
    updatePatternDisplay();
    
    // Start timer for updating debug display (10 FPS)
    startTimer(100);
}

RhythmPatternExplorerAudioProcessorEditor::~RhythmPatternExplorerAudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Modern dark background suitable for iPad
    g.fillAll (juce::Colours::darkgrey.darker());
    
    // Add subtle gradient for visual interest
    juce::ColourGradient gradient(juce::Colours::darkgrey.darker(), 0, 0,
                                  juce::Colours::darkgrey.darker().brighter(0.1f), 0, getHeight(),
                                  false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Add a subtle border
    g.setColour (juce::Colours::lightgrey.withAlpha(0.3f));
    g.drawRect (getLocalBounds(), 2);
}

void RhythmPatternExplorerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(25, 25);  // Add padding for touch-friendly spacing
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(50));
    bounds.removeFromTop(15);  // Spacing
    
    // UPI input field - full width for easy typing
    auto inputBounds = bounds.removeFromTop(50);
    upiInputField.setBounds(inputBounds);
    bounds.removeFromTop(15);  // Spacing
    
    // Trigger button - centered, minimum 44pt touch target
    auto buttonBounds = bounds.removeFromTop(60);
    auto buttonWidth = juce::jmax(120, buttonBounds.getWidth() / 3);
    auto buttonHeight = juce::jmax(44, 55); // Minimum 44pt for touch
    
    // Center the trigger button
    auto triggerBounds = buttonBounds.withSizeKeepingCentre(buttonWidth, buttonHeight);
    triggerButton.setBounds(triggerBounds);
    
    bounds.removeFromTop(20);  // Spacing
    
    // Pattern visualization 
    patternDisplay.setBounds(bounds.removeFromTop(50));
    bounds.removeFromTop(15);  // Spacing
    
    // Status 
    statusLabel.setBounds(bounds.removeFromTop(40));
    
    bounds.removeFromTop(15);  // Spacing
    
    // Debug display - take remaining space (minimum 100px)
    auto debugHeight = juce::jmax(100, bounds.getHeight());
    debugDisplay.setBounds(bounds.removeFromTop(debugHeight));
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::triggerButtonClicked()
{
    // Get current UPI input with safe string handling
    juce::String upiInput;
    try 
    {
        upiInput = upiInputField.getText().trim();
    }
    catch (...)
    {
        statusLabel.setText("Text input error - please re-enter pattern", juce::dontSendNotification);
        return;
    }
    
    if (upiInput.isNotEmpty() && upiInput.length() > 0 && upiInput.length() < 100)
    {
        // Send UPI pattern to processor
        audioProcessor.setUPIInput(upiInput);
        
        // Trigger pattern playback
        auto* tickParam = audioProcessor.getTickParameter();
        if (tickParam != nullptr)
        {
            tickParam->setValueNotifyingHost(1.0f);
        }
        
        // Update status with confirmation - use safe string handling
        try
        {
            juce::String statusText = juce::String("Pattern: ") + upiInput;
            statusLabel.setText(statusText, juce::dontSendNotification);
        }
        catch (...)
        {
            statusLabel.setText("Pattern triggered", juce::dontSendNotification);
        }
        
        // Update pattern display
        updatePatternDisplay();
        
        // Reset status after a delay
        juce::Timer::callAfterDelay(2000, [this]() {
            if (statusLabel.isShowing())
            {
                statusLabel.setText("Enter UPI pattern and tap TRIGGER", juce::dontSendNotification);
            }
        });
    }
    else
    {
        statusLabel.setText("Please enter a valid pattern (1-100 chars)", juce::dontSendNotification);
    }
}

void RhythmPatternExplorerAudioProcessorEditor::upiInputChanged()
{
    // Real-time pattern validation and preview
    juce::String upiInput = upiInputField.getText();
    
    if (upiInput.isNotEmpty())
    {
        // Update pattern display in real-time
        updatePatternDisplay();
    }
}

void RhythmPatternExplorerAudioProcessorEditor::updatePatternDisplay()
{
    // Get current pattern from processor
    auto currentPattern = audioProcessor.getCurrentPattern();
    
    if (currentPattern.empty())
    {
        patternDisplay.setText("No pattern", juce::dontSendNotification);
        return;
    }
    
    // Create visual representation using filled/empty circles
    juce::String visualPattern;
    for (size_t i = 0; i < currentPattern.size(); ++i)
    {
        if (i > 0) visualPattern += " ";
        visualPattern += currentPattern[i] ? "X" : ".";  // X .
    }
    
    patternDisplay.setText(visualPattern, juce::dontSendNotification);
}


void RhythmPatternExplorerAudioProcessorEditor::timerCallback()
{
    // Get current debug info from processor
    juce::String currentInfo = audioProcessor.getDebugInfo();
    
    // Accumulate debug info with timestamps (keep last 20 lines)
    static juce::StringArray debugLines;
    
    if (currentInfo != "Ready" && !currentInfo.isEmpty())
    {
        juce::String timestamp = juce::Time::getCurrentTime().toString(false, true, true, true);
        debugLines.add("[" + timestamp + "] " + currentInfo);
        
        // Keep only last 20 lines
        while (debugLines.size() > 20)
            debugLines.remove(0);
        
        // Update display
        juce::String displayText = "Debug Log (copyable):\n" + debugLines.joinIntoString("\n");
        debugDisplay.setText(displayText, false);
        
        // Auto-scroll to bottom
        debugDisplay.moveCaretToEnd();
    }
}