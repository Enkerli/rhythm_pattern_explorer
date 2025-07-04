/*
  ==============================================================================

    Rhythm Pattern Explorer - Plugin Editor Implementation

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>

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
    
    // Version Editor - copyable and selectable
    versionEditor.setText("v0.02b.240703.2245-CLEAN", juce::dontSendNotification);
    versionEditor.setMultiLine(false);
    versionEditor.setReadOnly(true);
    versionEditor.setScrollbarsShown(false);
    versionEditor.setCaretVisible(false);
    versionEditor.setJustification(juce::Justification::centredLeft);
    versionEditor.setFont(juce::Font(14.0f, juce::Font::bold));
    versionEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    versionEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1a1a1a));
    versionEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff4a5568));
    versionEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff4a5568));
    addAndMakeVisible(versionEditor);
    
    // Step Counter Display - removed for clean production interface
    
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
    startTimer(16); // Update every 16ms (~60fps) for even smoother animation
    
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
    
    // Version editor (bottom left corner) - larger bounds for better visibility
    versionEditor.setBounds(getLocalBounds().removeFromBottom(25).removeFromLeft(140));
    
    // Step counter display removed for clean production interface
    
    // Remaining area is for the circle - MAXIMIZED for clean interface
    circleArea = area.expanded(100);
}

void RhythmPatternExplorerAudioProcessorEditor::timerCallback()
{
    // Update pattern display and animation
    static int lastUpdateHash = 0;
    static int lastCurrentStep = -1;
    static bool lastPlayingState = false;
    static int frameCount = 0;
    static int testStepCounter = 0;
    
    frameCount++;
    
    int currentHash = std::hash<std::string>{}(audioProcessor.getPatternEngine().getBinaryString().toStdString());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Step counter display removed for clean production interface
    
    // Debug output removed for clean production version
    
    // Always repaint when playing to ensure smooth animation
    bool shouldRepaint = false;
    
    if (currentHash != lastUpdateHash)
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
        DBG("Playing state changed from " + juce::String(lastPlayingState ? "true" : "false") + " to " + juce::String(isPlaying ? "true" : "false"));
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
    float innerRadius = maxRadius * 0.3f;  // Larger inner hole for better donut effect
    float outerRadius = maxRadius;
    float markerRadius = maxRadius * 0.85f;
    
    int numSteps = static_cast<int>(pattern.size());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Clean production version - no debug output
    
    // Draw the background circle first
    g.setColour(juce::Colour(0xff2d3748));
    g.fillEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2);
    
    // Draw each slice using the most basic approach possible
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        float endAngle = startAngle + sliceAngle;
        
        // Only draw onset slices
        if (pattern[i])
        {
            // Create a proper filled pie sector using multiple triangles
            juce::Path slice;
            
            // Start from center
            slice.startNewSubPath(center.x, center.y);
            
            // Add multiple points around the arc to create a smooth sector
            int numSegments = std::max(8, int(sliceAngle * 20)); // More segments for larger slices
            for (int seg = 0; seg <= numSegments; ++seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + outerRadius * std::cos(angle);
                float y = center.y + outerRadius * std::sin(angle);
                slice.lineTo(x, y);
            }
            
            // Close back to center
            slice.closeSubPath();
            
            g.setColour(juce::Colour(0xff48bb78));  // Green for onsets
            g.fillPath(slice);
            
            // Drew slice - clean production version
        }
    }
    
    // Draw inner circle to create donut effect AFTER all slices
    if (innerRadius > 0)
    {
        g.setColour(juce::Colour(0xff2d3748)); // Same as background
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
        float startAngle = (highlightStep * sliceAngle) - juce::MathConstants<float>::halfPi;
        float endAngle = startAngle + sliceAngle;
        
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
        
        // Use different colors for playing vs recent playback
        juce::Colour highlightColor = isPlaying ? juce::Colour(0xffff6b35) : juce::Colour(0xff888888); // Orange when playing, gray when stopped
        g.setColour(highlightColor);
        g.fillPath(highlightSlice);
        
        // Clean production version - highlight displayed
        
        // Redraw inner circle on top of highlight
        if (innerRadius > 0)
        {
            g.setColour(juce::Colour(0xff2d3748));
            g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                         innerRadius * 2, innerRadius * 2);
        }
    }
    
    // Draw slice separator lines AFTER filling
    g.setColour(juce::Colour(0xff4a5568));
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        
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
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        
        // Position marker at the CENTER of each slice
        float centerAngle = angle + (sliceAngle * 0.5f);
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
