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
    
    // Pattern Display Label - larger and more readable
    patternDisplayLabel.setJustificationType(juce::Justification::centred);
    patternDisplayLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain));
    patternDisplayLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    patternDisplayLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff1a1a1a));
    addAndMakeVisible(patternDisplayLabel);
    
    // Analysis Label - improved readability
    analysisLabel.setJustificationType(juce::Justification::centred);
    analysisLabel.setFont(juce::Font(13.0f));
    analysisLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(analysisLabel);
    
    // Version Label
    versionLabel.setText("v0.01", juce::dontSendNotification);
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
    startTimer(100); // Update every 100ms
    
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
    patternDisplayLabel.setBounds(displayArea.reduced(10));
    
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
    int currentHash = std::hash<std::string>{}(audioProcessor.getPatternEngine().getBinaryString().toStdString());
    if (currentHash != lastUpdateHash)
    {
        updatePatternDisplay();
        updateAnalysisDisplay();
        repaint(); // Repaint for visual circle
        lastUpdateHash = currentHash;
    }
}

//==============================================================================
void RhythmPatternExplorerAudioProcessorEditor::drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    if (pattern.empty())
        return;
    
    auto center = bounds.getCentre();
    float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.3f;
    
    // Draw circle outline
    g.setColour(juce::Colour(0xff4a5568));
    g.drawEllipse(center.x - radius, center.y - radius, radius * 2, radius * 2, 2.0f);
    
    // Draw pattern steps
    for (int i = 0; i < pattern.size(); ++i)
    {
        float angle = 2.0f * juce::MathConstants<float>::pi * i / pattern.size() - juce::MathConstants<float>::halfPi;
        float x = center.x + radius * std::cos(angle);
        float y = center.y + radius * std::sin(angle);
        
        if (pattern[i])
        {
            // Onset - filled circle
            g.setColour(juce::Colour(0xff48bb78));
            g.fillEllipse(x - 6, y - 6, 12, 12);
        }
        else
        {
            // Rest - empty circle
            g.setColour(juce::Colour(0xff718096));
            g.drawEllipse(x - 4, y - 4, 8, 8, 1.0f);
        }
    }
    
    // Draw center of gravity - COMMENTED OUT per user request for pre-AUv3 state
//    auto balance = audioProcessor.getPatternEngine().analyzeBalance();
//    double cogAngle = audioProcessor.getPatternEngine().calculateCenterOfGravityAngle();
//    float cogAngleRad = static_cast<float>(cogAngle * juce::MathConstants<double>::pi / 180.0 - juce::MathConstants<double>::halfPi);
//    float cogRadius = radius * 0.3f * static_cast<float>(balance.normalizedMagnitude);
//    
//    float cogX = center.x + cogRadius * std::cos(cogAngleRad);
//    float cogY = center.y + cogRadius * std::sin(cogAngleRad);
//    
//    g.setColour(juce::Colour(0xfff56565));
//    g.fillEllipse(cogX - 3, cogY - 3, 6, 6);
//    g.drawLine(center.x, center.y, cogX, cogY, 1.0f);
}

void RhythmPatternExplorerAudioProcessorEditor::updatePatternDisplay()
{
    juce::String binary = audioProcessor.getPatternEngine().getBinaryString();
    juce::String description = audioProcessor.getPatternEngine().getPatternDescription();
    
    patternDisplayLabel.setText(binary + "\n" + description, juce::dontSendNotification);
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
