# CRITICAL: Accent System Design Principles

## Problems to NEVER Repeat:

1. **Swirling**: Updating accent markers every step during playback
2. **Missing cycle updates**: Failing to update accent markers at cycle boundaries  
3. **Missing manual updates**: Failing to update accent markers when advancing scenes/progressive
4. **Timeline desync**: Resetting pattern structure at different times for MIDI vs visual
5. **Step indicator chaos**: Moving orange cursor on its own schedule vs actual playback position
6. **Transport start Off-by-One Error**: Calculating UI accent offset BEFORE MIDI accent position advances

## CORRECT Architecture:

### **Dual Offset System (PRESERVE THIS):**
```cpp
int globalAccentPosition = 0;  // For MIDI output (advances every onset)
int uiAccentOffset = 0;        // For UI display (stable within cycles)
```

### **Update Rules:**
1. **MIDI accent position**: Advances on every actual MIDI note trigger
2. **UI accent offset**: Updates ONLY at:
   - Cycle boundaries (step 0) - **AFTER** MIDI processing completes
   - Manual triggers (Enter, MIDI input, scenes, progressive)
   - Transport jumps/resets

### **Critical Timing Order:**
```cpp
// CORRECT order at cycle boundaries:
processStep(midiBuffer, sample, targetStep);  // 1. MIDI advances globalAccentPosition
uiAccentOffset = globalAccentPosition % size; // 2. UI syncs to new MIDI state

// WRONG order (causes Off-by-One Error):
uiAccentOffset = globalAccentPosition % size; // 1. UI uses old MIDI state  
processStep(midiBuffer, sample, targetStep);  // 2. MIDI advances, UI now outdated
```

### **Step Indicator:**
- Must reflect the ACTUAL current playback step
- Must update consistently with pattern triggers
- NO independent scheduling

### **Critical Timing:**
- Both accent systems and step indicator must use the SAME step calculation
- Both must update at the SAME moments
- NO separate timelines or scheduling

## Transport Sync Integration:

The transport sync code must:
1. Calculate the correct step position
2. Update step indicator to match
3. Update uiAccentOffset only at cycle boundaries or manual triggers
4. Keep globalAccentPosition in sync with actual MIDI triggers
5. Ensure all updates happen atomically

## Test Case:
`{10}E(3,8)` = pattern `10010010`, accent `10`
- Cycle 1: Accents at steps 0 (yes), 3 (no), 6 (yes)
- Cycle 2: Accents at steps 0 (no), 3 (yes), 6 (no)  
- Cycle 3: Accents at steps 0 (yes), 3 (no), 6 (yes)
- Step indicator: 0,1,2,3,4,5,6,7,0,1,2,3... (linear progression)

## Implementation Priority:
1. Fix step indicator to follow actual playback position
2. Fix uiAccentOffset calculation for proper cycling
3. Ensure both use the same step calculation source
4. Test with transport jumps and manual triggers