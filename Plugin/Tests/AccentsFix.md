Certainly! Here’s a robust approach to ensure the **visual accent markers** in your circular sequencer always match the **actual MIDI accents**:

---

## **Design: Single Source of Truth for Accents**

**Key Principle:**  
The accent logic in the audio processor (PluginProcessor.cpp) is the source of truth. The UI (PluginEditor.cpp) should query the processor for accent state, not try to reconstruct it.

---

## **Step 1: Expose Per-Step Accent Information in the Processor**

Add a method to `RhythmPatternExplorerAudioProcessor` that, for the current pattern, returns a vector indicating which steps are accented **as they will be played** (i.e., only for onsets, in the correct accent cycle).

### **Implementation**

Add to `PluginProcessor.h`:
````cpp
// ...existing code...
public:
    /** Returns a vector of bools indicating which steps will be accented in the current pattern cycle. */
    std::vector<bool> getCurrentAccentMap() const;
// ...existing code...
````

Add to PluginProcessor.cpp:
````cpp
// ...existing code...
std::vector<bool> RhythmPatternExplorerAudioProcessor::getCurrentAccentMap() const
{
    // Get the current pattern and accent pattern
    auto pattern = patternEngine.getCurrentPattern();
    std::vector<bool> accentMap(pattern.size(), false);

    if (!hasAccentPattern || currentAccentPattern.empty())
        return accentMap;

    int accentIdx = globalAccentPosition;
    int accentLen = static_cast<int>(currentAccentPattern.size());
    int accentPos = accentIdx % accentLen;

    // Simulate accent assignment for the current pattern cycle
    int accentCounter = accentPos;
    for (size_t i = 0; i < pattern.size(); ++i)
    {
        if (pattern[i])
        {
            accentMap[i] = currentAccentPattern[accentCounter % accentLen];
            accentCounter++;
        }
    }
    return accentMap;
}
// ...existing code...
````

**Explanation:**  
- This method simulates the accent assignment for the current pattern cycle, starting from the current accent position, and only increments the accent index for steps that are onsets (matching the MIDI logic).

---

## **Step 2: Use This Map in the Editor for Drawing**

In your PluginEditor.cpp, when drawing the circular sequencer, **query the processor for the accent map** and use it to determine which steps are accented.

### **Implementation**

In PluginEditor.cpp, update your drawing code (e.g., in `drawPatternCircle` or equivalent):

````cpp
// ...existing code...
auto* processor = dynamic_cast<RhythmPatternExplorerAudioProcessor*>(&audioProcessor);
std::vector<bool> accentMap = processor->getCurrentAccentMap();

for (size_t i = 0; i < pattern.size(); ++i)
{
    bool isAccented = accentMap[i];
    // Use isAccented to set the color/marker for this step
    // ...existing drawing code...
}
// ...existing code...
````

---

## **Step 3: Reset Accent Position on Pattern Change**

Whenever a new pattern or accent structure is loaded (e.g., in `parseAndApplyUPI`), **reset `globalAccentPosition`** to zero to ensure the accent cycle starts in sync for both MIDI and UI.

You already do this in:
```cpp
if (resetAccentPosition) {
    globalAccentPosition = 0;
}
```
Just ensure that the UI is refreshed/redrawn after a pattern change.

---

## **Step 4: UI Refresh**

After any pattern/accent change, call your UI update method (e.g., `updatePatternDisplay()`) so the sequencer redraws using the new accent map.

---

## **Summary**

- **Processor**: Implements `getCurrentAccentMap()` as the single source of truth for accent assignment.
- **Editor**: Uses this map for drawing, never reconstructs accent logic.
- **Pattern Changes**: Reset accent position and refresh UI.

---

**This will guarantee that the visual accent markers always match the MIDI accents, regardless of pattern or accent structure changes.**  
Let me know if you want the exact code for your drawing function or further integration help!