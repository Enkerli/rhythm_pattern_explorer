# iPad AUv3 MIDI Effect Template

**A minimal, working template for creating iPad AUv3 MIDI effect plugins using JUCE.**

This template provides everything you need to build, deploy, and distribute iPad AUv3 MIDI effects that work reliably in professional iOS DAWs like AUM, Logic Pro for iPad, and GarageBand.

## 🎯 What This Template Provides

- ✅ **Complete, working iPad AUv3 MIDI effect** - builds and runs out of the box
- ✅ **Proper iOS configuration** - passes all Apple validation requirements  
- ✅ **Professional code structure** - clean, documented, and maintainable
- ✅ **Touch-optimized UI foundation** - ready for iPad-specific interfaces
- ✅ **Thread-safe MIDI processing** - handles real-time audio thread correctly
- ✅ **Host integration** - works in AUM, Logic Pro, GarageBand, and other iOS DAWs
- ✅ **Zero compilation warnings** - production-ready code quality
- ✅ **Comprehensive documentation** - detailed guides and troubleshooting

## 🚀 Quick Start

### Prerequisites

- **Xcode 15+** with iOS development tools installed
- **JUCE 8.0+** framework (download from [juce.com](https://juce.com))
- **Apple Developer Account** for device deployment
- **iPad** running iOS 15.0+ for testing

### 1. Get the Template

```bash
git clone https://github.com/yourusername/ipad-auv3-midi-template.git
cd ipad-auv3-midi-template
```

### 2. Customize Your Plugin

1. **Open Projucer** and load `NewProject.jucer`
2. **Update project settings**:
   - **Project Name**: Your awesome plugin name
   - **Company Name**: Your company or artist name  
   - **Bundle Identifier**: `com.yourcompany.yourplugin` (must be unique)
   - **Plugin Manufacturer Code**: 4-character code (e.g., "YrCo")
   - **Plugin Code**: 4-character plugin ID (e.g., "AwsP")

### 3. Generate and Build

1. In Projucer, click **"Save and open in IDE"**
2. Xcode opens with your customized project
3. **Set signing**: Select both targets → Signing & Capabilities → choose your Apple Developer Team
4. **Update bundle IDs**: Make sure AUv3 extension has `.AUv3` suffix
5. **Connect iPad** and select as build target
6. **Build and run** (⌘+R)

### 4. Test Your Plugin

1. **App installs** on iPad and registers the AUv3 plugin
2. **Open AUM** → Add Module → MIDI Effects → Find your plugin
3. **Connect to a synth** and tap the demo button to send MIDI notes
4. **Success!** You have a working iPad AUv3 plugin

## 📁 Project Structure

```
iPad-AUv3-MIDI-Template/
├── NewProject.jucer              # JUCE project configuration
├── Source/                       # Plugin source code
│   ├── PluginProcessor.h         # Main audio processor (your MIDI logic goes here)
│   ├── PluginProcessor.cpp       # Audio processor implementation  
│   ├── PluginEditor.h           # UI editor interface
│   └── PluginEditor.cpp         # UI implementation (customize your interface)
├── Builds/iOS/                   # Generated Xcode project (auto-created)
├── docs/                         # Documentation and guides
│   ├── CUSTOMIZATION.md         # How to customize the template
│   ├── TROUBLESHOOTING.md       # Common issues and solutions
│   └── ADVANCED.md              # Advanced features and techniques
├── README.md                     # This file
└── LICENSE                       # CC0 Public Domain (use freely!)
```

## 🎛️ Key Features

### MIDI Processing
- **Thread-safe MIDI I/O** using JUCE's `MidiMessageCollector`
- **Real-time processing** in the audio thread
- **MIDI input handling** - process incoming notes, CCs, etc.
- **MIDI generation** - create and send MIDI messages
- **Example implementation** with clear extension points

### iPad-Optimized UI  
- **Touch-friendly controls** (44pt+ touch targets)
- **Responsive layout** adapting to iPad orientations
- **Clean visual design** using JUCE's modern LookAndFeel
- **Real-time feedback** for user interactions
- **AUv3 host integration** - proper embedding in DAW interfaces

### iOS Integration
- **Proper AUv3 configuration** - appears correctly in all major iOS DAWs
- **Background audio** capability for continuous operation
- **State persistence** - saves/loads settings between sessions
- **Memory management** - optimized for iOS constraints
- **App Store ready** - meets all Apple requirements

## 🛠️ Customization Guide

### Change Plugin Identity

1. **In Projucer**: Update project name, company, and bundle identifiers
2. **In source code**: Rename `TemplateAudioProcessor` class to match your plugin
3. **Regenerate project**: Click "Save and open in IDE"

### Add Your MIDI Logic

The main processing happens in `PluginProcessor.cpp`:

```cpp
void TemplateAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // Your MIDI processing logic here
    
    // Process incoming MIDI
    for (const auto message : midiMessages)
    {
        const auto midiMsg = message.getMessage();
        
        if (midiMsg.isNoteOn())
        {
            // Transform, generate, or respond to incoming notes
        }
    }
    
    // Generate new MIDI messages
    // Use midiCollector.addMessageToQueue() for thread safety
}
```

### Customize the Interface

Edit `PluginEditor.cpp` to create your UI:

```cpp
TemplateAudioProcessorEditor::TemplateAudioProcessorEditor(TemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Add your controls
    addAndMakeVisible(myButton);
    addAndMakeVisible(mySlider);
    
    // Set up interactions
    myButton.onClick = [this] { 
        // Handle button press
        audioProcessor.sendMidiNote(60, 127, 1);
    };
    
    setSize(400, 300); // iPad-friendly size
}
```

### Add Parameters

```cpp
// In constructor
addParameter(new AudioParameterFloat("gain", "Gain", 0.0f, 1.0f, 0.5f));

// In processBlock  
float gainValue = *parameters.getRawParameterValue("gain");
```

## 📱 Testing and Deployment

### Testing Workflow

1. **Simulator testing** - Limited but useful for UI work
2. **Device testing** - Essential for MIDI and performance testing
3. **Multiple host testing** - Test in AUM, Logic Pro, GarageBand
4. **Performance profiling** - Use Xcode Instruments for optimization

### Distribution Options

- **Direct installation** - Install via Xcode for personal use
- **TestFlight** - Beta testing with invited users
- **App Store** - Public distribution (requires App Store Review)
- **Enterprise distribution** - For corporate/educational use

## 🚨 Common Issues and Solutions

### "Plugin not found in host apps"
- ✅ Verify bundle IDs: App = `com.you.plugin`, Extension = `com.you.plugin.AUv3`
- ✅ Check that app installed successfully
- ✅ Restart host app to refresh AUv3 cache

### "Build failed with signing errors"  
- ✅ Set Apple Developer Team in both app and extension targets
- ✅ Ensure bundle IDs are unique (not used by other apps)
- ✅ Check device is registered in developer account

### "App crashes when loading plugin"
- ✅ Check Xcode console for error messages
- ✅ Verify all source files are added to correct targets
- ✅ Test with a minimal UI first

### "MIDI not working"
- ✅ Connect plugin output to a synth in AUM
- ✅ Check MIDI routing in host app
- ✅ Verify `producesMidi()` returns `true`

## 📚 Learn More

- **[JUCE Documentation](https://docs.juce.com/)** - Framework reference
- **[Apple Audio Unit Guide](https://developer.apple.com/documentation/audiotoolbox)** - iOS audio programming
- **[AUv3 Programming Guide](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/)** - Official Apple docs
- **[iOS App Extension Guide](https://developer.apple.com/app-extensions/)** - iOS extension development

## 🤝 Contributing

This template is public domain (CC0), but contributions are welcome!

- **Report issues** - Found a bug or improvement? Open an issue
- **Share examples** - Built something cool? Share your customizations
- **Improve docs** - Help make the template more accessible
- **Test compatibility** - Try with new iOS versions and DAWs

## 📄 License

**CC0 1.0 Universal (Public Domain)**

This template is dedicated to the public domain. You can copy, modify, distribute and perform the work, even for commercial purposes, all without asking permission. See the [LICENSE](LICENSE) file for full details.

## 🙏 Acknowledgments

This template was created to simplify iPad AUv3 development after navigating the complex world of iOS audio plugin creation. Special thanks to:

- **JUCE team** - For the excellent framework
- **iOS audio community** - For sharing knowledge and troubleshooting tips  
- **Apple Developer Forums** - For iOS-specific guidance
- **AUM and Logic Pro teams** - For creating great host environments

## 🎵 Happy Plugin Development!

Building iPad audio plugins should be about creativity, not configuration. This template handles the technical setup so you can focus on making great music tools.

**Questions? Issues? Ideas?** Open an issue or discussion - let's make iPad audio development more accessible for everyone! 

---

*Made with ❤️ for the iOS audio development community*