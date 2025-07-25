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
#define DBG(textToWrite) do { } while (false)
#endif

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
    addAndMakeVisible(upiTextEditor);
    
    
    // Instance Name Editor - no label to save space
    instanceNameEditor.setMultiLine(false);
    instanceNameEditor.setReturnKeyStartsNewLine(false);
    instanceNameEditor.setReadOnly(false);
    instanceNameEditor.setScrollbarsShown(false);
    instanceNameEditor.setCaretVisible(true);
    instanceNameEditor.setPopupMenuEnabled(true);
    instanceNameEditor.setText("Rhythm", juce::dontSendNotification);
    instanceNameEditor.setFont(juce::FontOptions(12.0f));
    instanceNameEditor.setJustification(juce::Justification::centredLeft);
    addAndMakeVisible(instanceNameEditor);
    
    // MIDI Note Slider (spinner style) - wider text box for number visibility
    midiNoteLabel.setText("Note:", juce::dontSendNotification);
    addAndMakeVisible(midiNoteLabel);
    
    midiNoteSlider.setSliderStyle(juce::Slider::IncDecButtons);
    midiNoteSlider.setRange(0, 127, 1);
    midiNoteSlider.setValue(36); // C2 - kick drum
    midiNoteSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    midiNoteSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsNotDraggable);
    addAndMakeVisible(midiNoteSlider);
    
    
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
    versionEditor.setText("v1.3.5-" + buildTimestamp.substring(0, 6) + "-" + buildTimestamp.substring(12, 17), juce::dontSendNotification);
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
    docsToggleButton.setButtonText("Docs");
    docsToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    docsToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    docsToggleButton.onClick = [this]() { toggleDocumentation(); };
    addAndMakeVisible(docsToggleButton);
    
    // Initialize WebView documentation (initially hidden)
#if JUCE_WEB_BROWSER
    docsBrowser = std::make_unique<juce::WebBrowserComponent>();
    docsBrowser->setVisible(false); // Explicitly hidden initially
    addAndMakeVisible(*docsBrowser);
    createDocumentationHTML();
#endif
    
    // Step Counter Display - removed for clean production interface
    
    
    // Connect essential parameters
    midiNoteSlider.onValueChange = [this]()
    {
        if (audioProcessor.getMidiNoteParameter()) {
            audioProcessor.getMidiNoteParameter()->setValueNotifyingHost(
                audioProcessor.getMidiNoteParameter()->convertTo0to1(static_cast<int>(midiNoteSlider.getValue()))
            );
        }
    };
    
    
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
        instanceNameEditor.setVisible(!minimalMode);
        midiNoteLabel.setVisible(!minimalMode);
        midiNoteSlider.setVisible(!minimalMode);
        tickButton.setVisible(!minimalMode);
        patternDisplayEditor.setVisible(!minimalMode);
        docsToggleButton.setVisible(!minimalMode);
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
    int spacing = 50; // Total spacing between elements
    
    
    // Responsive layout: hide controls progressively when space is limited
    bool showTickButton = availableWidth >= (upiLabelWidth + minUpiWidth + tickButtonWidth + 30);
    bool showNameField = availableWidth >= (upiLabelWidth + minUpiWidth + (showTickButton ? tickButtonWidth : 0) + nameFieldWidth + 40);
    bool showNoteField = availableWidth >= (upiLabelWidth + minUpiWidth + (showTickButton ? tickButtonWidth : 0) + 
                                           (showNameField ? nameFieldWidth : 0) + noteFieldWidth + 50);
    
    // Set visibility based on available space
    tickButton.setVisible(showTickButton);
    instanceNameEditor.setVisible(showNameField);
    midiNoteLabel.setVisible(showNoteField);
    midiNoteSlider.setVisible(showNoteField);
    
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
        
        if (showNameField)
        {
            auto instanceField = rightControls.removeFromLeft(nameFieldWidth).reduced(2);
            instanceNameEditor.setBounds(instanceField);
            rightControls.removeFromLeft(10); // spacing
        }
        
        if (showNoteField)
        {
            midiNoteLabel.setBounds(rightControls.removeFromLeft(35));
            auto noteField = rightControls.removeFromLeft(75).reduced(2);
            midiNoteSlider.setBounds(noteField);
            rightControls.removeFromLeft(5); // spacing
        }
        
    }
    
    
    // UPI text field gets remaining space
    upiTextEditor.setBounds(upiRow.reduced(5));
    
    // Pattern display area (text results) - readable size
    auto displayArea = area.removeFromTop(60);
    patternDisplayEditor.setBounds(displayArea.reduced(10));
    
    // Docs button area - positioned right after pattern display, aligned right
    if (!showingDocs)
    {
        auto docsButtonArea = area.removeFromTop(30);
        docsToggleButton.setBounds(docsButtonArea.removeFromRight(80).reduced(5));
    }
    
    // Analysis area - hidden
    auto analysisArea = area.removeFromTop(0);
    analysisLabel.setBounds(analysisArea.reduced(10));
    
    // Version display at bottom left (only when not showing docs)
    if (!showingDocs)
    {
        auto bottomArea = getLocalBounds().removeFromBottom(25);
        versionEditor.setBounds(bottomArea.removeFromLeft(100));
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
            auto bottomControls = docsArea.removeFromBottom(30); // Leave space for toggle button
            docsBrowser->setBounds(docsArea);
            
            // Reposition docs toggle button to be visible
            docsToggleButton.setBounds(bottomControls.removeFromRight(80).reduced(2));
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
    
    // Sync UI sliders with parameter values (for host automation support)
    if (audioProcessor.getMidiNoteParameter()) {
        midiNoteSlider.setValue(audioProcessor.getMidiNoteParameter()->get(), juce::dontSendNotification);
    }
    
    // Update step/scene button text
    updateStepSceneButton();
    
    auto currentHash = std::hash<std::string>{}(audioProcessor.getPatternEngine().getBinaryString().toStdString());
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
                
                // Fill inner half with green
                g.setColour(juce::Colour(0xff48bb78));  // Green like unaccented
                g.fillPath(innerHalf);
                
                // Outer half: High contrast accent color (bright orange/red)
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
                
                // Fill outer half with bright accent color
                g.setColour(juce::Colour(0xffff4500));  // Bright orange-red for high contrast
                g.fillPath(outerHalf);
                
                // Bold outline for accented onsets - draw after filling
                g.setColour(juce::Colour(0xffffffff));  // White outline for maximum contrast
                g.strokePath(slice, juce::PathStrokeType(3.0f));  // Thick outline
                
            } else {
                // UNACCENTED ONSET: Solid green as before
                g.setColour(juce::Colour(0xff48bb78));  // Green for regular onsets
                g.fillPath(slice);
            }
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
        
        // Use different colors for playing vs recent playback
        juce::Colour highlightColor = isPlaying ? juce::Colour(0xffff6b35) : juce::Colour(0xff888888); // Orange when playing, gray when stopped
        g.setColour(highlightColor);
        g.fillPath(highlightSlice);
        
        // Clean production version - highlight displayed
        
        // Redraw inner circle on top of highlight
        if (innerRadius > 0)
        {
            g.setColour(getBackgroundColour());
            g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                         innerRadius * 2, innerRadius * 2);
        }
    }
    
    // Draw slice separator lines AFTER filling
    g.setColour(juce::Colour(0xff4a5568));
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
    g.setColour(juce::Colour(0xff4a5568));
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
            g.setColour(juce::Colour(0xff4a5568));
            g.fillEllipse(x - 8, y - 8, 16, 16);
            
            // Draw step numbers (show actual step index for larger patterns)
            juce::String stepNumber = juce::String(stepIndex);
            g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
            g.setColour(juce::Colours::white);
            
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
    
    // Display pattern in multiple notations: binary with description, then hex/octal/decimal
    juce::String displayText = binary + " | " + description + "\n" + 
                              hex + " | " + octal + " | " + decimal;
    
    patternDisplayEditor.setText(displayText, juce::dontSendNotification);
}

void RhythmPatternExplorerAudioProcessorEditor::updateAnalysisDisplay()
{
    // Analysis functionality removed - methods no longer available
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
    
    // Clear the text editor for next input (optional)
    // upiTextEditor.clear();
}

void RhythmPatternExplorerAudioProcessorEditor::onParseButtonClicked()
{
    parseUPIPattern();
}

int RhythmPatternExplorerAudioProcessorEditor::getMidiNoteNumber() const
{
    // Slider already handles range validation (0-127)
    return static_cast<int>(midiNoteSlider.getValue());
}

void RhythmPatternExplorerAudioProcessorEditor::toggleDocumentation()
{
#if JUCE_WEB_BROWSER
    if (!docsBrowser) 
    {
        return;
    }
    
    showingDocs = !showingDocs;
    
    // Update button text immediately
    docsToggleButton.setButtonText(showingDocs ? "Pattern" : "Docs");
    
    // Update visibility immediately
    docsBrowser->setVisible(showingDocs);
    
    // Force layout update
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
    htmlContent += "<p>UPI provides a mathematical language for describing rhythm patterns using algorithms like Euclidean, Polygon, and Binary sequences.</p>\n";
    htmlContent += "<p><strong>Download:</strong> <a href=\"https://github.com/Enkerli/rhythm_pattern_explorer/releases\" target=\"_blank\" style=\"color: #68d391; text-decoration: underline;\">Latest Releases</a> | <a href=\"https://github.com/Enkerli/rhythm_pattern_explorer\" target=\"_blank\" style=\"color: #68d391; text-decoration: underline;\">Source Code</a></p>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Basic Patterns</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Euclidean: 3 onsets distributed evenly across 8 steps</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">P(5,0)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Polygon: Pentagon rhythm (5 equally spaced onsets)</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">B(170,8)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Binary: Convert decimal 170 to 8-step binary pattern</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Progressive Patterns</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8)E.8</div>\n";
    htmlContent += "<div class=\"pattern-description\">Progressive Euclidean: Pattern advances each trigger</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<div class=\"warning\">\n";
    htmlContent += "<strong>Note:</strong> Progressive patterns change each time they are triggered via MIDI input.\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Pattern Combinations</h2>\n";
    htmlContent += "<div class=\"pattern-example\">\n";
    htmlContent += "<div class=\"pattern-code\">E(3,8) + E(2,5)</div>\n";
    htmlContent += "<div class=\"pattern-description\">Combine two Euclidean patterns using OR logic</div>\n";
    htmlContent += "</div>\n";
    htmlContent += "<h2>Syntax Reference</h2>\n";
    htmlContent += "<table>\n";
    htmlContent += "<tr><th>Pattern Type</th><th>Syntax</th><th>Example</th></tr>\n";
    htmlContent += "<tr><td>Euclidean</td><td>E(onsets,steps)</td><td>E(5,13)</td></tr>\n";
    htmlContent += "<tr><td>Polygon</td><td>P(sides,offset,steps?)</td><td>P(7,2,16)</td></tr>\n";
    htmlContent += "<tr><td>Binary</td><td>B(decimal,steps)</td><td>B(85,8)</td></tr>\n";
    htmlContent += "<tr><td>Progressive</td><td>Pattern.steps</td><td>E(3,8)E.8</td></tr>\n";
    htmlContent += "<tr><td>Combination</td><td>Pattern + Pattern</td><td>E(3,8) + P(5,0)</td></tr>\n";
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
    htmlContent += "<li><strong>MIDI Triggering:</strong> Play any MIDI note to advance progressive patterns</li>\n";
    htmlContent += "<li><strong>Mathematical Beauty:</strong> Try E(3,8), E(5,8), E(7,16) for musical results</li>\n";
    htmlContent += "<li><strong>Polygon Magic:</strong> P(3,0) through P(12,0) create interesting polyrhythms</li>\n";
    htmlContent += "<li><strong>Binary Exploration:</strong> Powers of 2 like B(85,8) create symmetric patterns</li>\n";
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
        // Also write to a debug file for inspection
        juce::File debugFile = tempDir.getChildFile("rhythm_pattern_docs_debug.html");
        debugFile.replaceWithText(htmlContent);
        
        juce::URL fileURL = juce::URL(htmlFile);
        docsBrowser->goToURL(fileURL.toString(false));
        
        // Log the file path for debugging
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
    repaint(); // Trigger redraw with new background color
}

void RhythmPatternExplorerAudioProcessorEditor::mouseDoubleClick(const juce::MouseEvent& event)
{
    // Double-click anywhere to cycle background colors
    cycleBackgroundColor();
}

