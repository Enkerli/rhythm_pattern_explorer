/*
  ==============================================================================

    Rhythm Pattern Explorer - Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RhythmPatternExplorerAudioProcessorEditor::RhythmPatternExplorerAudioProcessorEditor (RhythmPatternExplorerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Plugin size - resizable with minimum and maximum constraints
    setSize (500, 600);
    setResizable(true, true);
    setResizeLimits(400, 400, 1200, 1000); // min width, min height, max width, max height
    
    // BPM Slider - COMMENTED OUT for clean interface
    /*
    bpmSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bpmSlider.setRange(60.0, 180.0, 1.0);
    bpmSlider.setValue(120.0);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(bpmSlider);
    
    bpmLabel.setText("BPM", juce::dontSendNotification);
    bpmLabel.attachToComponent(&bpmSlider, true);
    addAndMakeVisible(bpmLabel);
    
    // Pattern Type ComboBox
    patternTypeComboBox.addItem("Euclidean", 1);
    patternTypeComboBox.addItem("Polygon", 2);
    patternTypeComboBox.addItem("Random", 3);
    patternTypeComboBox.addItem("Binary", 4);
    patternTypeComboBox.setSelectedId(1);
    addAndMakeVisible(patternTypeComboBox);
    
    patternTypeLabel.setText("Pattern Type", juce::dontSendNotification);
    patternTypeLabel.attachToComponent(&patternTypeComboBox, true);
    addAndMakeVisible(patternTypeLabel);
    
    // Onsets Slider
    onsetsSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    onsetsSlider.setRange(1.0, 16.0, 1.0);
    onsetsSlider.setValue(3.0);
    onsetsSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(onsetsSlider);
    
    onsetsLabel.setText("Onsets", juce::dontSendNotification);
    onsetsLabel.attachToComponent(&onsetsSlider, true);
    addAndMakeVisible(onsetsLabel);
    
    // Steps Slider
    stepsSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    stepsSlider.setRange(4.0, 32.0, 1.0);
    stepsSlider.setValue(8.0);
    stepsSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    addAndMakeVisible(stepsSlider);
    
    stepsLabel.setText("Steps", juce::dontSendNotification);
    stepsLabel.attachToComponent(&stepsSlider, true);
    addAndMakeVisible(stepsLabel);
    
    // Play Button
    playButton.setButtonText("Play");
    playButton.setClickingTogglesState(true);
    playButton.onClick = [this]()
    {
        bool isPlaying = playButton.getToggleState();
        audioProcessor.getPlayingParameter()->setValueNotifyingHost(isPlaying ? 1.0f : 0.0f);
        playButton.setButtonText(isPlaying ? "Stop" : "Play");
    };
    addAndMakeVisible(playButton);
    
    // Generate Button
    generateButton.setButtonText("Generate New");
    generateButton.onClick = [this]()
    {
        // Force regeneration by changing a parameter slightly
        int currentType = patternTypeComboBox.getSelectedId() - 1;
        if (currentType == 2) // Random type
        {
            // For random patterns, just trigger a regeneration
            audioProcessor.getPatternEngine().generateRandomPattern(
                static_cast<int>(onsetsSlider.getValue()),
                static_cast<int>(stepsSlider.getValue())
            );
            updatePatternDisplay();
            updateAnalysisDisplay();
        }
    };
    addAndMakeVisible(generateButton);
    */
    
    // UPI Pattern Input
    upiLabel.setText("UPI Pattern:", juce::dontSendNotification);
    upiLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(upiLabel);
    
    upiTextEditor.setMultiLine(false);
    upiTextEditor.setReturnKeyStartsNewLine(false);
    upiTextEditor.setTextToShowWhenEmpty("Enter pattern: E(3,8), P(5,0), etc.", juce::Colours::grey);
    upiTextEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    upiTextEditor.onReturnKey = [this]() { parseUPIPattern(); };
    addAndMakeVisible(upiTextEditor);
    
    parseUPIButton.setButtonText("Parse");
    parseUPIButton.onClick = [this]() { onParseButtonClicked(); };
    addAndMakeVisible(parseUPIButton);
    
    // Pattern Display Editor - copyable and readable
    patternDisplayEditor.setMultiLine(true);
    patternDisplayEditor.setReadOnly(true);
    patternDisplayEditor.setScrollbarsShown(false);
    patternDisplayEditor.setCaretVisible(false);
    patternDisplayEditor.setJustification(juce::Justification::centred);
    patternDisplayEditor.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    patternDisplayEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    patternDisplayEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    patternDisplayEditor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
    patternDisplayEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5568));
    addAndMakeVisible(patternDisplayEditor);
    
    // Analysis Label - improved readability
    analysisLabel.setJustificationType(juce::Justification::centred);
    analysisLabel.setFont(juce::Font(13.0f));
    analysisLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(analysisLabel);
    
    // Version Label with build number
    versionLabel.setText("v0.02b.240703", juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::centredLeft);
    versionLabel.setFont(juce::Font(10.0f));
    versionLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(versionLabel);
    
    // Connect sliders to parameters - COMMENTED OUT for clean interface
    /*
    bpmSlider.onValueChange = [this]()
    {
        audioProcessor.getBpmParameter()->setValueNotifyingHost(
            audioProcessor.getBpmParameter()->convertTo0to1(static_cast<float>(bpmSlider.getValue()))
        );
    };
    
    patternTypeComboBox.onChange = [this]()
    {
        int selectedIndex = patternTypeComboBox.getSelectedId() - 1;
        audioProcessor.getPatternTypeParameter()->setValueNotifyingHost(
            static_cast<float>(selectedIndex) / (audioProcessor.getPatternTypeParameter()->choices.size() - 1)
        );
        
        // Show/hide generate button for random patterns
        generateButton.setVisible(selectedIndex == 2);
    };
    
    onsetsSlider.onValueChange = [this]()
    {
        audioProcessor.getOnsetsParameter()->setValueNotifyingHost(
            audioProcessor.getOnsetsParameter()->convertTo0to1(static_cast<int>(onsetsSlider.getValue()))
        );
    };
    
    stepsSlider.onValueChange = [this]()
    {
        audioProcessor.getStepsParameter()->setValueNotifyingHost(
            audioProcessor.getStepsParameter()->convertTo0to1(static_cast<int>(stepsSlider.getValue()))
        );
    };
    */
    
    // Initial display update
    updatePatternDisplay();
    updateAnalysisDisplay();
    
    // Start timer for regular UI updates
    startTimer(30); // Update every 30ms for even smoother animation
    
    DBG("RhythmPatternExplorer: Editor initialized");
}

RhythmPatternExplorerAudioProcessorEditor::~RhythmPatternExplorerAudioProcessorEditor()
{
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Background
    g.fillAll (juce::Colour(0xff2d3748));
    
    // Title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font(18.0f, juce::Font::bold));
    g.drawText ("Rhythm Pattern Explorer", 0, 10, getWidth(), 30, 
                juce::Justification::centred);
    
    // Pattern circle area - now uses dynamic sizing from resized()
    if (!circleArea.isEmpty())
        drawPatternCircle(g, circleArea);
}

void RhythmPatternExplorerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // Title area
    auto titleArea = area.removeFromTop(50);
    
    // Control area - minimal layout with just UPI input
    auto controlArea = area.removeFromTop(60);
    controlArea.reduce(20, 10);
    
    // UPI Pattern Input row - only visible control
    auto upiRow = controlArea.removeFromTop(40);
    upiLabel.setBounds(upiRow.removeFromLeft(100));
    parseUPIButton.setBounds(upiRow.removeFromRight(80).reduced(5));
    upiTextEditor.setBounds(upiRow.reduced(5));
    
    // All other controls commented out for clean interface
    /*
    controlArea.removeFromTop(10); // spacing
    
    // BPM row
    auto bpmRow = controlArea.removeFromTop(30);
    bpmLabel.setBounds(bpmRow.removeFromLeft(50));
    bpmSlider.setBounds(bpmRow);
    
    controlArea.removeFromTop(5); // spacing
    
    // Pattern Type row
    auto patternRow = controlArea.removeFromTop(30);
    patternTypeLabel.setBounds(patternRow.removeFromLeft(100));
    patternTypeComboBox.setBounds(patternRow);
    
    // Additional controls in a compact layout
    auto compactArea = area.removeFromTop(80);
    compactArea.reduce(20, 5);
    
    // Onsets and Steps in one row
    auto paramRow = compactArea.removeFromTop(30);
    onsetsLabel.setBounds(paramRow.removeFromLeft(60));
    onsetsSlider.setBounds(paramRow.removeFromLeft(120));
    paramRow.removeFromLeft(10); // spacing
    stepsLabel.setBounds(paramRow.removeFromLeft(50));
    stepsSlider.setBounds(paramRow);
    
    compactArea.removeFromTop(5); // spacing
    
    // Play and Generate buttons row
    auto buttonRow = compactArea.removeFromTop(30);
    playButton.setBounds(buttonRow.removeFromLeft(80));
    buttonRow.removeFromLeft(10); // spacing
    generateButton.setBounds(buttonRow.removeFromLeft(100));
    */
    
    // Pattern display area (text results) - readable size
    auto displayArea = area.removeFromTop(60);
    patternDisplayEditor.setBounds(displayArea.reduced(10));
    
    // Analysis area - hidden
    auto analysisArea = area.removeFromTop(0);
    analysisLabel.setBounds(analysisArea.reduced(10));
    
    // Version label (bottom left corner)
    versionLabel.setBounds(getLocalBounds().removeFromBottom(20).removeFromLeft(50));
    
    // Remaining area is for the circle - MAXIMIZED for clean interface
    circleArea = area.expanded(100);
}

void RhythmPatternExplorerAudioProcessorEditor::timerCallback()
{
    // Update UI from parameters - minimal updates for clean interface
    /*
    float bpm = audioProcessor.getBpmParameter()->get();
    if (std::abs(bpmSlider.getValue() - bpm) > 0.1)
    {
        bpmSlider.setValue(bpm, juce::dontSendNotification);
    }
    
    int patternType = audioProcessor.getPatternTypeParameter()->getIndex();
    if (patternTypeComboBox.getSelectedId() - 1 != patternType)
    {
        patternTypeComboBox.setSelectedId(patternType + 1, juce::dontSendNotification);
        generateButton.setVisible(patternType == 2); // Random type
    }
    
    int onsets = audioProcessor.getOnsetsParameter()->get();
    if (static_cast<int>(onsetsSlider.getValue()) != onsets)
    {
        onsetsSlider.setValue(onsets, juce::dontSendNotification);
    }
    
    int steps = audioProcessor.getStepsParameter()->get();
    if (static_cast<int>(stepsSlider.getValue()) != steps)
    {
        stepsSlider.setValue(steps, juce::dontSendNotification);
    }
    
    bool isPlaying = audioProcessor.getPlayingParameter()->get();
    if (playButton.getToggleState() != isPlaying)
    {
        playButton.setToggleState(isPlaying, juce::dontSendNotification);
        playButton.setButtonText(isPlaying ? "Stop" : "Play");
    }
    */
    
    // Update pattern display
    static int lastUpdateHash = 0;
    static int lastCurrentStep = -1;
    static bool lastPlayingState = false;
    
    int currentHash = std::hash<std::string>{}(audioProcessor.getPatternEngine().getBinaryString().toStdString());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Repaint if pattern changed, step changed during playback, or play state changed
    // Be more aggressive about detecting step changes for better animation
    if (currentHash != lastUpdateHash || 
        currentStep != lastCurrentStep ||
        isPlaying != lastPlayingState)
    {
        if (currentHash != lastUpdateHash)
        {
            updatePatternDisplay();
            updateAnalysisDisplay();
        }
        repaint(); // Repaint for visual circle animation
        
        lastUpdateHash = currentHash;
        lastCurrentStep = currentStep;
        lastPlayingState = isPlaying;
    }
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    if (pattern.empty())
        return;
    
    auto center = bounds.getCentre();
    float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.3f;  // Keep markers inside bounds
    float innerRadius = maxRadius * 0.15f;  // Small hole to avoid moiré
    float outerRadius = maxRadius;
    float markerRadius = maxRadius * 0.85f;  // Step markers INSIDE the pie boundary
    
    int numSteps = static_cast<int>(pattern.size());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Debug current step (removed for production)
    // DBG("Current step: " + juce::String(currentStep) + ", Playing: " + juce::String(isPlaying ? "true" : "false"));
    
    // Draw the background circle first
    g.setColour(juce::Colour(0xff2d3748));
    g.fillEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2);
    
    // Draw each slice with FIXED positioning and calculations
    for (int i = 0; i < numSteps; ++i)
    {
        // Calculate slice angles correctly
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;  // Angle per slice
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;  // Start at top, step i
        float endAngle = startAngle + sliceAngle;  // End of step i
        
        // Only color onset slices differently - rest slices stay background color
        if (pattern[i])
        {
            juce::Path slice;
            // Use proper pie segment bounds
            float x = center.x - outerRadius;
            float y = center.y - outerRadius;
            float width = outerRadius * 2;
            float height = outerRadius * 2;
            float thickness = (outerRadius - innerRadius) / outerRadius;
            
            slice.addPieSegment(x, y, width, height, startAngle, endAngle, thickness);
            
            g.setColour(juce::Colour(0xff48bb78));  // Green for onsets
            g.fillPath(slice);
            
            // Debug slice info (removed for production)
            // DBG("Slice " + juce::String(i) + ": onset=" + juce::String(pattern[i] ? "true" : "false"));
        }
    }
    
    // Draw playback highlighting OVER the slices
    if (isPlaying && currentStep >= 0 && currentStep < numSteps)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float startAngle = (currentStep * sliceAngle) - juce::MathConstants<float>::halfPi;
        float endAngle = startAngle + sliceAngle;
        
        juce::Path highlightSlice;
        float x = center.x - outerRadius;
        float y = center.y - outerRadius;
        float width = outerRadius * 2;
        float height = outerRadius * 2;
        float thickness = (outerRadius - innerRadius) / outerRadius;
        
        highlightSlice.addPieSegment(x, y, width, height, startAngle, endAngle, thickness);
        
        g.setColour(juce::Colour(0xffff6b35));  // Bright orange highlight
        g.fillPath(highlightSlice);
        
        // DBG("Highlighting step " + juce::String(currentStep) + " at angles " + juce::String(startAngle) + " to " + juce::String(endAngle));
    }
    
    // Draw slice separator lines AFTER filling
    g.setColour(juce::Colour(0xff4a5568));
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;  // Line at start of step i
        
        float lineStartX = center.x + innerRadius * std::cos(angle);
        float lineStartY = center.y + innerRadius * std::sin(angle);
        float lineEndX = center.x + outerRadius * std::cos(angle);
        float lineEndY = center.y + outerRadius * std::sin(angle);
        g.drawLine(lineStartX, lineStartY, lineEndX, lineEndY, 1.5f);
    }
    
    // Draw outer and inner circle outlines
    g.setColour(juce::Colour(0xff4a5568));
    g.drawEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2, 2.0f);
    if (innerRadius > 0)
        g.drawEllipse(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2, 2.0f);
    
    // Draw step markers INSIDE the available space
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;  // Angle for step i marker
        
        // Position marker at the CENTER of each slice
        float centerAngle = angle + (sliceAngle * 0.5f);  // Middle of the slice
        float x = center.x + markerRadius * std::cos(centerAngle);
        float y = center.y + markerRadius * std::sin(centerAngle);
        
        // Draw step marker circles
        g.setColour(juce::Colour(0xff4a5568));
        g.fillEllipse(x - 8, y - 8, 16, 16);
        
        // Draw step numbers
        juce::String stepNumber = juce::String(i);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        
        // Center the text in the marker
        juce::Rectangle<float> textBounds(x - 8, y - 6, 16, 12);
        g.drawText(stepNumber, textBounds, juce::Justification::centred);
    }
}

void RhythmPatternExplorerAudioProcessorEditor::updatePatternDisplay()
{
    juce::String binary = audioProcessor.getPatternEngine().getBinaryString();
    juce::String hex = audioProcessor.getPatternEngine().getHexString();
    juce::String octal = audioProcessor.getPatternEngine().getOctalString();
    juce::String decimal = audioProcessor.getPatternEngine().getDecimalString();
    juce::String description = audioProcessor.getPatternEngine().getPatternDescription();
    
    // Display pattern in multiple notations: binary with description, then hex/octal/decimal
    juce::String displayText = binary + " | " + description + "\n" + 
                              hex + " | " + octal + " | " + decimal;
    
    patternDisplayEditor.setText(displayText, juce::dontSendNotification);
}

void RhythmPatternExplorerAudioProcessorEditor::updateAnalysisDisplay()
{
    // Balance and center of gravity analysis - COMMENTED OUT per user request for pre-AUv3 state
//    auto balance = audioProcessor.getPatternEngine().analyzeBalance();
//    double cogAngle = audioProcessor.getPatternEngine().calculateCenterOfGravityAngle();
//    
//    juce::String analysis = "Balance: " + balance.balanceRating + 
//                           juce::String::formatted(" (%.3f)", balance.magnitude) +
//                           "\nCenter of Gravity: " + juce::String::formatted("%.1f°", cogAngle);
//    
//    analysisLabel.setText(analysis, juce::dontSendNotification);
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
    
    // Clear the text editor for next input (optional)
    // upiTextEditor.clear();
}

void RhythmPatternExplorerAudioProcessorEditor::onParseButtonClicked()
{
    parseUPIPattern();
}
