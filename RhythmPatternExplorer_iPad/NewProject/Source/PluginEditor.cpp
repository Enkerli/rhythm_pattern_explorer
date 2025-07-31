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
    // UPI Pattern Input (desktop-compatible)
    upiLabel.setText("UPI:", juce::dontSendNotification);
    upiLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(upiLabel);
    
    upiTextEditor.setMultiLine(false);
    upiTextEditor.setReturnKeyStartsNewLine(false);
    upiTextEditor.setTextToShowWhenEmpty("Enter pattern: E(3,8), P(5,0), etc.", juce::Colours::grey);
    upiTextEditor.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 14.0f, juce::Font::plain)));
    upiTextEditor.onReturnKey = [this]() { parseUPIPattern(); };
    upiTextEditor.setText(audioProcessor.getCurrentUPIInput(), juce::dontSendNotification);
    addAndMakeVisible(upiTextEditor);
    
    // Scene/Step Button (equivalent to Parse/Tick) - shows current step/scene
    tickButton.setButtonText("1"); // Will be updated in timer
    tickButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    tickButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    tickButton.setTooltip("Current step/scene number. Click to advance.");
    tickButton.onClick = [this]() { onParseButtonClicked(); };
    addAndMakeVisible(tickButton);
    
    
    // Pattern text display (smaller, supplementary to circle)
    patternDisplayEditor.setText("Ready", juce::dontSendNotification);
    patternDisplayEditor.setFont(juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::plain)));
    patternDisplayEditor.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(patternDisplayEditor);

    
    // Debug display removed - not needed with circle visualization
    
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
    // Background with adaptive color system
    g.fillAll(getBackgroundColour());
    
    // Draw pattern circle in available space
    if (!circleArea.isEmpty())
        drawPatternCircle(g, circleArea);
}

void RhythmPatternExplorerAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    area.reduce(20, 20);  // iPad-friendly padding
    
    // UPI input row at top
    auto upiRow = area.removeFromTop(40);
    upiLabel.setBounds(upiRow.removeFromLeft(50));
    
    // Tick button on right (iPad-friendly size)
    auto tickButtonWidth = 60;
    tickButton.setBounds(upiRow.removeFromRight(tickButtonWidth));
    upiRow.removeFromRight(10);  // Spacing
    
    // UPI text field takes remaining space
    upiTextEditor.setBounds(upiRow);
    
    area.removeFromTop(15);  // Spacing
    
    // Circle area - main focus (larger for iPad)
    auto circleHeight = juce::jmin(300, area.getHeight() - 60);
    circleArea = area.removeFromTop(circleHeight);
    
    area.removeFromTop(10);  // Spacing
    
    // Pattern text display (small, supplementary)
    patternDisplayEditor.setBounds(area.removeFromTop(30));
}

//==============================================================================


void RhythmPatternExplorerAudioProcessorEditor::timerCallback()
{
    // Update displays to show scene changes
    updatePatternDisplay();
    updateStepSceneButton();
    
    // Repaint to update circle visualization with current step
    repaint();
}

void RhythmPatternExplorerAudioProcessorEditor::drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto pattern = audioProcessor.getCurrentPattern();
    if (pattern.empty())
        return;
    
    auto center = bounds.getCentre();
    float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.35f;
    float innerRadius = maxRadius * 0.15f;  // Smaller inner hole like desktop
    float outerRadius = maxRadius;
    
    int numSteps = static_cast<int>(pattern.size());
    int currentStep = audioProcessor.getCurrentStep();
    
    // Draw background circle - adaptive color like desktop
    if (currentBackgroundColor == BackgroundColor::Green)
    {
        g.setColour(juce::Colours::white);
    }
    else
    {
        g.setColour(getBackgroundColour());
    }
    g.fillEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2);
    
    // Draw pattern steps
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        // Start at 12 o'clock like desktop
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi - (sliceAngle * 0.5f);
        
        // Draw onset slices
        if (pattern[i])
        {
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
            
            for (int seg = numSegments; seg >= 0; --seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + innerRadius * std::cos(angle);
                float y = center.y + innerRadius * std::sin(angle);
                slice.lineTo(x, y);
            }
            slice.closeSubPath();
            
            // Simple color scheme for iPad - current step vs regular
            if (i == currentStep)
            {
                g.setColour(juce::Colours::orange);  // Current step highlight
            }
            else
            {
                g.setColour(juce::Colours::lightblue);  // Regular onset
            }
            g.fillPath(slice);
        }
    }
    
    // Draw inner circle to create donut effect like desktop
    if (innerRadius > 0)
    {
        g.setColour(getBackgroundColour());
        g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                     innerRadius * 2, innerRadius * 2);
    }
    
    // Draw step markers around the circle
    for (int i = 0; i < numSteps; ++i)
    {
        float angle = (i * 2.0f * juce::MathConstants<float>::pi / numSteps) - juce::MathConstants<float>::halfPi;
        float markerRadius = maxRadius * 1.15f;
        
        float x = center.x + markerRadius * std::cos(angle);
        float y = center.y + markerRadius * std::sin(angle);
        
        // Step number with better visibility
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(juce::String(i + 1), x - 10, y - 10, 20, 20, juce::Justification::centred);
    }
}

void RhythmPatternExplorerAudioProcessorEditor::parseUPIPattern()
{
    juce::String upiInput = upiTextEditor.getText().trim();
    
    if (upiInput.isNotEmpty())
    {
        // Check if this is a different pattern than what's currently loaded
        if (upiInput != audioProcessor.getCurrentUPIInput())
        {
            // New pattern - send to processor for initialization
            audioProcessor.setUPIInput(upiInput);
        }
        
        // Always trigger pattern playback (handles scene advancement)
        auto* tickParam = audioProcessor.getTickParameter();
        if (tickParam != nullptr)
        {
            tickParam->setValueNotifyingHost(1.0f);
        }
        
        updatePatternDisplay();
    }
}

void RhythmPatternExplorerAudioProcessorEditor::onParseButtonClicked()
{
    parseUPIPattern();
}

void RhythmPatternExplorerAudioProcessorEditor::updatePatternDisplay()
{
    // Get current pattern from processor
    auto currentPattern = audioProcessor.getCurrentPattern();
    
    if (currentPattern.empty())
    {
        patternDisplayEditor.setText("No pattern", juce::dontSendNotification);
        return;
    }
    
    // Get onset positions for display (desktop-compatible)
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
    
    // Calculate hex, octal, and decimal representations
    int decimal = 0;
    for (int i = 0; i < static_cast<int>(currentPattern.size()); ++i) {
        if (currentPattern[i]) {
            decimal |= (1 << i);  // LSB-first bit ordering like desktop
        }
    }
    
    juce::String hex = "0x" + juce::String::toHexString(decimal).toUpperCase();
    juce::String octal = "0o" + juce::String::formatted("%o", decimal);  // Octal format
    juce::String decimalStr = juce::String(decimal);
    
    // Get accent information if present (placeholder for now)
    juce::String accentInfo = "";
    // TODO: Add accent pattern support
    
    // Combine all information like desktop version
    juce::String displayText = "Onsets: " + onsetPositions + accentInfo + " | " + hex + " | " + octal + " | " + decimalStr;
    
    patternDisplayEditor.setText(displayText, juce::dontSendNotification);
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
        // Progressive transformation: show current offset
        int offset = audioProcessor.getProgressiveOffset();
        buttonText = juce::String(offset);
        tooltip = "Progressive transformation offset: " + juce::String(offset) + ". Click to advance.";
    }
    else
    {
        // Regular pattern: show current step (1-based)
        int currentStep = audioProcessor.getCurrentStep() + 1;
        buttonText = juce::String(currentStep);
        tooltip = "Current step " + juce::String(currentStep) + ". Click to trigger pattern.";
    }
    
    tickButton.setButtonText(buttonText);
    tickButton.setTooltip(tooltip);
}

juce::Colour RhythmPatternExplorerAudioProcessorEditor::getBackgroundColour() const
{
    switch (currentBackgroundColor)
    {
        case BackgroundColor::White:  return juce::Colours::white;
        case BackgroundColor::Green:  return juce::Colour(0xff48bb78);
        case BackgroundColor::Orange: return juce::Colour(0xffff6b35);
        case BackgroundColor::Blue:   return juce::Colour(0xff4299e1);
        case BackgroundColor::Purple: return juce::Colour(0xff9f7aea);
        default:                      return juce::Colour(0xff2d3748); // Dark
    }
}