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
    setResizeLimits(150, 150, 1200, 1000); // min width, min height, max width, max height - small min for Easter egg
    
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
    
    
    // Instance Name Editor - no label to save space
    instanceNameEditor.setMultiLine(false);
    instanceNameEditor.setReturnKeyStartsNewLine(false);
    instanceNameEditor.setReadOnly(false);
    instanceNameEditor.setScrollbarsShown(false);
    instanceNameEditor.setCaretVisible(true);
    instanceNameEditor.setPopupMenuEnabled(true);
    instanceNameEditor.setText("Rhythm", juce::dontSendNotification);
    instanceNameEditor.setFont(juce::Font(12.0f));
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
    
    // Accent Pitch Offset Slider
    accentPitchOffsetLabel.setText("Accent +/-:", juce::dontSendNotification);
    addAndMakeVisible(accentPitchOffsetLabel);
    
    accentPitchOffsetSlider.setSliderStyle(juce::Slider::IncDecButtons);
    accentPitchOffsetSlider.setRange(-12, 12, 1);
    accentPitchOffsetSlider.setValue(5); // Default +5 semitones
    accentPitchOffsetSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    accentPitchOffsetSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsNotDraggable);
    addAndMakeVisible(accentPitchOffsetSlider);
    
    // Accent Velocity Slider
    accentVelocityLabel.setText("Accent Vol:", juce::dontSendNotification);
    addAndMakeVisible(accentVelocityLabel);
    
    accentVelocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    accentVelocitySlider.setRange(0.0, 1.0, 0.01);
    accentVelocitySlider.setValue(1.0); // Default maximum velocity
    accentVelocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(accentVelocitySlider);
    
    // Unaccented Velocity Slider
    unaccentedVelocityLabel.setText("Normal Vol:", juce::dontSendNotification);
    addAndMakeVisible(unaccentedVelocityLabel);
    
    unaccentedVelocitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    unaccentedVelocitySlider.setRange(0.0, 1.0, 0.01);
    unaccentedVelocitySlider.setValue(0.69); // Default 69%
    unaccentedVelocitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 20);
    addAndMakeVisible(unaccentedVelocitySlider);
    
    // Tick Button (equivalent to Parse)
    tickButton.setButtonText("Tick");
    tickButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a5568));
    tickButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(tickButton);
    
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
    versionEditor.setText("v0.03c", juce::dontSendNotification);
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
    
    // Connect essential parameters
    midiNoteSlider.onValueChange = [this]()
    {
        if (audioProcessor.midiNoteParam) {
            audioProcessor.midiNoteParam->setValueNotifyingHost(
                audioProcessor.midiNoteParam->convertTo0to1(static_cast<int>(midiNoteSlider.getValue()))
            );
        }
    };
    
    // Connect accent parameters
    accentPitchOffsetSlider.onValueChange = [this]()
    {
        if (audioProcessor.accentPitchOffsetParam) {
            audioProcessor.accentPitchOffsetParam->setValueNotifyingHost(
                audioProcessor.accentPitchOffsetParam->convertTo0to1(static_cast<int>(accentPitchOffsetSlider.getValue()))
            );
        }
    };
    
    accentVelocitySlider.onValueChange = [this]()
    {
        if (audioProcessor.accentVelocityParam) {
            audioProcessor.accentVelocityParam->setValueNotifyingHost(
                static_cast<float>(accentVelocitySlider.getValue())
            );
        }
    };
    
    unaccentedVelocitySlider.onValueChange = [this]()
    {
        if (audioProcessor.unaccentedVelocityParam) {
            audioProcessor.unaccentedVelocityParam->setValueNotifyingHost(
                static_cast<float>(unaccentedVelocitySlider.getValue())
            );
        }
    };
    
    tickButton.onClick = [this]()
    {
        // Trigger both the parameter (for host automation) and immediate parsing
        if (audioProcessor.tickParam) {
            audioProcessor.tickParam->setValueNotifyingHost(1.0f); // Trigger tick parameter
        }
        parseUPIPattern(); // Execute parsing immediately
    };
    
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
    // Background with color option
    g.fillAll(getBackgroundColour());
    
    // Title (only in normal mode)
    if (!minimalMode)
    {
        g.setColour (juce::Colours::white);
        g.setFont (juce::Font(18.0f, juce::Font::bold));
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
        
        DBG("Minimal mode: " + juce::String(minimalMode ? "ON" : "OFF"));
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
    auto titleArea = area.removeFromTop(50);
    
    // Control area - compact layout with UPI input and controls
    auto controlArea = area.removeFromTop(70);
    controlArea.reduce(20, 10);
    
    // UPI Pattern Input row - primary control with compact extras
    auto upiRow = controlArea.removeFromTop(40);
    upiLabel.setBounds(upiRow.removeFromLeft(100));
    
    // Compact controls to the right
    auto rightControls = upiRow.removeFromRight(290); // Space for Tick + Name + Note spinner
    tickButton.setBounds(rightControls.removeFromLeft(70).reduced(5));
    rightControls.removeFromLeft(10); // spacing
    
    // Instance Name (no label - more space for name)
    auto instanceField = rightControls.removeFromLeft(90).reduced(2);
    instanceNameEditor.setBounds(instanceField);
    rightControls.removeFromLeft(10); // spacing
    
    // MIDI Note (spinner with wider text box)
    midiNoteLabel.setBounds(rightControls.removeFromLeft(35));
    auto noteField = rightControls.removeFromLeft(75).reduced(2);
    midiNoteSlider.setBounds(noteField);
    rightControls.removeFromLeft(5); // spacing
    
    // Accent Pitch Offset (spinner)
    accentPitchOffsetLabel.setBounds(rightControls.removeFromLeft(65));
    auto accentPitchField = rightControls.removeFromLeft(65).reduced(2);
    accentPitchOffsetSlider.setBounds(accentPitchField);
    rightControls.removeFromLeft(5); // spacing
    
    // Accent Velocity (linear slider)
    accentVelocityLabel.setBounds(rightControls.removeFromLeft(65));
    auto accentVelField = rightControls.removeFromLeft(75).reduced(2);
    accentVelocitySlider.setBounds(accentVelField);
    
    // Unaccented Velocity (linear slider)
    unaccentedVelocityLabel.setBounds(rightControls.removeFromLeft(65));
    auto unaccentedVelField = rightControls.removeFromLeft(75).reduced(2);
    unaccentedVelocitySlider.setBounds(unaccentedVelField);
    
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
    static int lastUpdateHash = 0;
    static int lastCurrentStep = -1;
    static bool lastPlayingState = false;
    static int frameCount = 0;
    static int testStepCounter = 0;
    
    frameCount++;
    
    // Sync UI sliders with parameter values (for host automation support)
    if (audioProcessor.midiNoteParam) {
        midiNoteSlider.setValue(audioProcessor.midiNoteParam->get(), juce::dontSendNotification);
    }
    if (audioProcessor.accentPitchOffsetParam) {
        accentPitchOffsetSlider.setValue(audioProcessor.accentPitchOffsetParam->get(), juce::dontSendNotification);
    }
    if (audioProcessor.accentVelocityParam) {
        accentVelocitySlider.setValue(audioProcessor.accentVelocityParam->get(), juce::dontSendNotification);
    }
    
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
    
    // Draw each slice using the most basic approach possible
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        // Start at 12 o'clock (north) and center slice 0 there: offset by half slice angle
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi - (sliceAngle * 0.5f);
        float endAngle = startAngle + sliceAngle;
        
        // Only draw onset slices
        if (pattern[i])
        {
            // Check if this step should be accented - stable per cycle display
            bool isAccented = false;
            if (audioProcessor.getHasAccentPattern() && !audioProcessor.getCurrentAccentPattern().empty())
            {
                // Find which onset number this step is (count onsets up to current step)
                int onsetIndex = 0;
                for (int j = 0; j < i; ++j)
                {
                    if (j < pattern.size() && pattern[j])
                    {
                        onsetIndex++;
                    }
                }
                
                // SIMPLIFIED: Use global accent position directly for stable cycle-based display
                // This bypasses cycle detection issues and uses the processor's global accent state
                int globalAccentPosition = audioProcessor.getGlobalAccentPosition();
                
                // Count total onsets that have occurred before this pattern position
                int onsetsInPattern = 0;
                for (bool onset : pattern) {
                    if (onset) onsetsInPattern++;
                }
                
                // Calculate which cycle we're in based on global accent position
                int currentAccentCycle = 0;
                if (onsetsInPattern > 0) {
                    currentAccentCycle = globalAccentPosition / onsetsInPattern;
                }
                
                // Calculate the accent offset for this cycle
                int displayAccentOffset = (currentAccentCycle * onsetsInPattern) % audioProcessor.getCurrentAccentPattern().size();
                
                // Check if this onset should be accented
                int accentStep = (displayAccentOffset + onsetIndex) % audioProcessor.getCurrentAccentPattern().size();
                isAccented = audioProcessor.getCurrentAccentPattern()[accentStep];
                
                // Debug output for first few onsets to see what's happening
                if (onsetIndex < 3) {
                    DBG("ACCENT CHECK: OnsetIndex=" << onsetIndex << 
                        ", AccentStep=" << accentStep << 
                        ", IsAccented=" << (isAccented ? "YES" : "NO"));
                }
            }
            
            if (isAccented)
            {
                // For accented onsets: create split slice with inner part (regular) and outer part (inverted)
                
                // Inner part: from innerRadius to markerRadius (regular color)
                juce::Path innerSlice;
                innerSlice.startNewSubPath(center.x + innerRadius * std::cos(startAngle), 
                                          center.y + innerRadius * std::sin(startAngle));
                
                int numSegments = std::max(8, int(sliceAngle * 20));
                for (int seg = 0; seg <= numSegments; ++seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + markerRadius * std::cos(angle);
                    float y = center.y + markerRadius * std::sin(angle);
                    innerSlice.lineTo(x, y);
                }
                
                // Back along inner arc
                for (int seg = numSegments; seg >= 0; --seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + innerRadius * std::cos(angle);
                    float y = center.y + innerRadius * std::sin(angle);
                    innerSlice.lineTo(x, y);
                }
                innerSlice.closeSubPath();
                
                g.setColour(juce::Colour(0xff48bb78));  // Green for inner part
                g.fillPath(innerSlice);
                
                // Outer part: from markerRadius to outerRadius (inverted background color)
                juce::Path outerSlice;
                outerSlice.startNewSubPath(center.x + markerRadius * std::cos(startAngle), 
                                          center.y + markerRadius * std::sin(startAngle));
                
                for (int seg = 0; seg <= numSegments; ++seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + outerRadius * std::cos(angle);
                    float y = center.y + outerRadius * std::sin(angle);
                    outerSlice.lineTo(x, y);
                }
                
                // Back along marker arc
                for (int seg = numSegments; seg >= 0; --seg)
                {
                    float angle = startAngle + (sliceAngle * seg / numSegments);
                    float x = center.x + markerRadius * std::cos(angle);
                    float y = center.y + markerRadius * std::sin(angle);
                    outerSlice.lineTo(x, y);
                }
                outerSlice.closeSubPath();
                
                // Inverted background color for accent indication
                juce::Colour accentColor;
                if (currentBackgroundColor == BackgroundColor::Green || currentBackgroundColor == BackgroundColor::White)
                {
                    accentColor = juce::Colour(0xff2d3748);  // Dark for light backgrounds
                }
                else
                {
                    accentColor = juce::Colours::white;  // White for dark backgrounds
                }
                g.setColour(accentColor);
                g.fillPath(outerSlice);
            }
            else
            {
                // Regular onset: create normal full slice
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
                
                g.setColour(juce::Colour(0xff48bb78));  // Green for regular onsets
                g.fillPath(slice);
            }
            
            // Drew slice - clean production version
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
    if (!minimalMode)
    {
        for (int i = 0; i < numSteps; ++i)
        {
            float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
            // Position marker at the CENTER of each slice, aligned with 12 o'clock
            float centerAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
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
        DBG("WebBrowser not available!");
        return;
    }
    
    showingDocs = !showingDocs;
    DBG("Toggle documentation: showingDocs = " + juce::String(showingDocs));
    
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
        DBG("HTML content written to: " + htmlFile.getFullPathName());
        DBG("Debug file written to: " + debugFile.getFullPathName());
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
