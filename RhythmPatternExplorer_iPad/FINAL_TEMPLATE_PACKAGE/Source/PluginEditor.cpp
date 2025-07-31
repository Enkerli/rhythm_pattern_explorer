/*
  ==============================================================================

    iPad AUv3 MIDI Effect Template
    Plugin Editor Implementation

    A minimal working template for creating iPad AUv3 MIDI effect plugins
    using JUCE framework.
    
    This template is in the public domain (CC0 1.0).
    Feel free to use it as a starting point for your own plugins.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TemplateAudioProcessorEditor::TemplateAudioProcessorEditor (TemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Set up title label
    titleLabel.setText("iPad AUv3 MIDI Template", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Set up send note button with iPad-optimized sizing
    sendNoteButton.setButtonText("Send MIDI Note (C4)");
    sendNoteButton.onClick = [this] { sendNoteButtonClicked(); };
    sendNoteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkblue);
    sendNoteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(sendNoteButton);
    
    // Set up status label
    statusLabel.setText("Tap button to send MIDI note", juce::dontSendNotification);
    statusLabel.setFont(juce::Font(14.0f));
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(statusLabel);

    // Set size suitable for iPad - responsive design
    setSize (400, 300);
    
    // Make resizable for different iPad orientations
    setResizable(true, true);
    setResizeLimits(300, 200, 800, 600);
}

TemplateAudioProcessorEditor::~TemplateAudioProcessorEditor()
{
}

//==============================================================================
void TemplateAudioProcessorEditor::paint (juce::Graphics& g)
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

void TemplateAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.reduce(20, 20);  // Add padding for touch-friendly spacing
    
    // Title at top
    titleLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(20);  // Spacing
    
    // Button in middle - ensure minimum 44pt touch target
    auto buttonBounds = bounds.removeFromTop(60);
    auto buttonWidth = juce::jmax(200, buttonBounds.getWidth() / 2);
    auto buttonHeight = juce::jmax(44, 50); // Minimum 44pt for touch
    buttonBounds = buttonBounds.withSizeKeepingCentre(buttonWidth, buttonHeight);
    sendNoteButton.setBounds(buttonBounds);
    
    bounds.removeFromTop(20);  // Spacing
    
    // Status at bottom
    statusLabel.setBounds(bounds.removeFromTop(30));
}

//==============================================================================
void TemplateAudioProcessorEditor::sendNoteButtonClicked()
{
    // Send a MIDI note through the processor
    audioProcessor.sendMidiNote(60, 100, 1);
    
    // Update status with confirmation
    statusLabel.setText("MIDI Note Sent! (C4, Velocity 100)", juce::dontSendNotification);
    
    // Reset status after a delay for better UX
    juce::Timer::callAfterDelay(1500, [this]() {
        if (statusLabel.isShowing()) // Check if component still exists
        {
            statusLabel.setText("Tap button to send MIDI note", juce::dontSendNotification);
        }
    });
}