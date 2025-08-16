
# Claude Code Memory

## Project Configuration
- **Plugin Installation**: Install to user library (`~/Library/Audio/Plug-Ins/`) NOT system library (`/Library/Audio/Plug-Ins/`)
- **NO STANDALONE APP**: User explicitly does NOT want a standalone application - plugins only (AU/VST3)
  - Remove any `.app` files that get built accidentally
  - Focus on plugin formats only: AU Component and VST3
- **Licensing**: CC0 1.0 Universal (public domain)
- **JUCE Version**: 8.0.8 with WebBrowserComponent support

## Project Development Guidelines
- Before getting "carried away", make sure that was requested fully works as expected.

## MIDI Note Duration Debugging Guidelines
**CRITICAL: Always test on both macOS AND iPadOS - platforms can behave differently**

### Required Tools for MIDI Debugging:
1. **MIDI Monitor AUv3** (iPadOS) - Essential for seeing actual MIDI output
2. **Desktop MIDI Monitor** (macOS) - Console apps or DAW MIDI monitors
3. **Internal Debug Counters** - UI-based debugging for AUv3 (console debugging doesn't work)

### Note Duration Issue Diagnostic Sequence:
1. **Check MIDI Monitor Output First** - Shows actual MIDI messages reaching the host
   - Look for note-on (0x90) AND note-off (0x80) messages
   - WARNING FLAG: Note-offs appearing before note-ons indicates timing issue
   - WARNING FLAG: Only note-ons with no note-offs indicates note-offs not reaching output

2. **Check Internal Debug Counters** - Shows if logic is executing
   - Triggers: How many notes have been triggered
   - Active: Current notes in tracking system
   - Offs: How many note-offs have been sent internally
   - WARNING FLAG: High "Offs" count but no note-offs in MIDI monitor = timing issue

3. **Verify MIDI Processing Order** - Critical sequence in processBlock:
   ```cpp
   // CORRECT ORDER:
   // 1. Pattern processing (adds note-ons)
   triggerNote(midiBuffer, samplePosition);
   // 2. Note tracking (sends note-offs)  
   processActiveNotes(midiMessages, bufferSize);
   // 3. Update sample position
   absoluteSamplePosition += bufferSize;
   ```

4. **Cross-Platform Testing** - Desktop can mask timing issues that break iPadOS
   - Test identical patterns on both platforms
   - Compare MIDI monitor output between platforms
   - iPadOS is more sensitive to incorrect MIDI buffer timing

### Common Note Duration Issues:
- **Wrong processBlock order**: Note-offs processed before note-ons → note-offs sent in future buffers
- **Invalid buffer positions**: Note-offs at negative or out-of-bounds positions → messages dropped
- **Zero velocity note-offs**: Some hosts/instruments ignore zero velocity → use 0.5f
- **Platform differences**: Desktop may work with incorrect timing that breaks iPadOS

[Rest of the file remains unchanged...]
- User disapproves of premature celebration and self-praise. Emphasizes thoughtful work.