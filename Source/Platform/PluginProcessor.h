/*
  ==============================================================================

    Serpe - Sequence Editor, Rhythmic Pattern Explorer
    Unified Audio Processor Header
    (Adapted from RhythmPatternExplorer original implementation)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Core/PatternEngine.h"
#include "../Core/UPIParser.h"
#include "../Core/PolyParser.h"
#include "../Core/PolyClock.h"
#include "../Managers/SceneManager.h"
#include "../Managers/PresetManager.h"
#include "PlatformSpecific.h"
#include <atomic>
#include <array>
#include "../Core/Microtiming.h"

//==============================================================================
// PHASE 2: Lock-Free Pattern Update Queue

struct PatternUpdate {
    std::vector<bool> rhythmPattern;
    std::vector<bool> accentPattern;
    bool hasAccent = false;
    int accentPhaseOffset = 0;  // User-controlled offset
    
    PatternUpdate() = default;
    PatternUpdate(const std::vector<bool>& rhythm, const std::vector<bool>& accent, bool accent_enabled, int offset = 0)
        : rhythmPattern(rhythm), accentPattern(accent), hasAccent(accent_enabled), accentPhaseOffset(offset) {}
};

class PatternUpdateQueue {
    static constexpr size_t QUEUE_SIZE = 16;
    std::array<PatternUpdate, QUEUE_SIZE> queue;
    std::atomic<size_t> writeIndex{0};
    std::atomic<size_t> readIndex{0};
    
public:
    bool enqueue(const PatternUpdate& update);
    bool dequeue(PatternUpdate& update);
    bool isEmpty() const;
};

//==============================================================================
/**
 * Main audio processor for Rhythm Pattern Explorer AUv3 plugin
 * 
 * This class handles:
 * - MIDI input/output 
 * - MIDI effect processing (no audio synthesis)
 * - Parameter management
 * - Pattern generation and analysis
 * - DAW transport synchronization
 */
class SerpeAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SerpeAudioProcessor();
    ~SerpeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // Pattern Engine Access
    const PatternEngine& getPatternEngine() const { return patternEngine; }
    PatternEngine& getPatternEngine() { return patternEngine; }
    
    // Pattern display information for UI
    juce::String getCurrentPatternDisplay() const;
    
    
    // Parameter access for editor
    juce::AudioParameterBool* getUseHostTransportParameter() const { return useHostTransportParam; }
    juce::AudioParameterInt* getMidiNoteParameter() const { return midiNoteParam; }
    juce::AudioParameterBool* getTickParameter() const { return tickParam; }
    juce::AudioParameterChoice* getPatternLengthUnitParameter() const { return patternLengthUnitParam; }
    juce::AudioParameterChoice* getPatternLengthValueParameter() const { return patternLengthValueParam; }
    juce::AudioParameterChoice* getSubdivisionParameter() const { return subdivisionParam; }
    
    // Accent parameter access for editor
    juce::AudioParameterFloat* getAccentVelocityParameter() const { return accentVelocityParam; }
    juce::AudioParameterFloat* getUnaccentedVelocityParameter() const { return unaccentedVelocityParam; }
    juce::AudioParameterInt* getAccentPitchOffsetParameter() const { return accentPitchOffsetParam; }
    
    // Helper to convert pattern length choice to float value
    float getPatternLengthValue() const;
    
    // Helper to convert subdivision choice to beat fraction
    double getSubdivisionInBeats(int subdivisionIndex) const;
    
    // Phase 3: Advanced Host Sync - Automatic pattern length adjustment
    double calculateAutoPatternLength(const std::vector<bool>& pattern) const;
    
    // Internal state access for editor
    float getCurrentBPM() const { return currentBPM; }
    void setCurrentBPM(float bpm) { currentBPM = bpm; updateTiming(); }
    bool getInternalPlaying() const { return internalPlaying; }
    void setInternalPlaying(bool playing) { internalPlaying = playing; }
    void triggerPatternUpdate() { if (!currentUPIInput.isEmpty()) parseAndApplyUPI(currentUPIInput); }
    
    // Phase 3: Advanced Host Sync - Loop sync access
    bool getHostIsLooping() const { return hostIsLooping; }
    bool getEnableLoopSync() const { return enableLoopSync; }
    void setEnableLoopSync(bool enable) { enableLoopSync = enable; }
    
    // Playback state
    int getCurrentStep() const { 
        // PHASE 0.2: Test derived approach - keep legacy for validation
        return getDerivedRhythmStep(); 
    }
    bool isCurrentlyPlaying() const { 
        // REFINED: Check if we're getting recent processBlock calls AND transport is playing
        double currentTime = juce::Time::getMillisecondCounter();
        bool recentProcessBlock = (currentTime - lastProcessBlockTime) < 100.0; // Within last 100ms
        
        // First try host transport, fall back to recent processBlock activity
        if (useHostTransportParam && useHostTransportParam->get()) {
            return hostIsPlaying && recentProcessBlock;
        } else {
            return internalPlaying && recentProcessBlock;
        }
    }
    bool isPlaying() const { return isCurrentlyPlaying(); } // Alias for compatibility
    
    // Parameter access for the WebView editor (read params + relay automation).
    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }

    // Accent phase for the current cycle (onsets-played % accent length) — lets
    // the UI precess the displayed accents in step with the audio, exactly as
    // the old native editor did. Updates at cycle boundaries.
    int getUIAccentOffset() const { return uiAccentOffset; }

    // True while a manually-edited (suspension-mode) pattern is active — the
    // editor mirrors processStep's accent branch so the DISPLAY matches the
    // AUDIO: per-step accents in suspension mode, onset-based (precessing)
    // accents for normal UPI patterns.
    bool isPatternManuallyModified() const { return patternManuallyModified; }

    // Onset count at the start of the cycle now playing — the display feeds this
    // (+ the onset's within-cycle index) to shouldOnsetBeAccented() so the
    // highlighted accents match what the audio plays, through precession and
    // through progressive pattern changes. (Public for SerpeEditor.)
    uint32_t getCycleStartOnsetCount() const;

    // UPI pattern input methods
    void setUPIInput(const juce::String& upiPattern);
    juce::String getUPIInput() const { return currentUPIInput; }
    juce::String getCurrentUPIInput() const { return currentUPIInput; } // Alias for compatibility
    void parseAndApplyUPI(const juce::String& upiPattern, bool resetAccentPosition = true);
    void applyCurrentScenePattern();
    /** Scene rotation/lengthening — runs after the queued base pattern lands. */
    void applySceneProgressiveTransform();
    
    // Background color persistence
    int getCurrentBackgroundColor() const { return currentBackgroundColor; }
    void setCurrentBackgroundColor(int color) { currentBackgroundColor = color; }
    
    // UPI history (ticker tape feature)
    const juce::StringArray& getUPIHistory() const { return upiHistory; }
    void addToUPIHistory(const juce::String& upiPattern);
    void clearUPIHistory() { upiHistory.clear(); }
    
    // Preset management
    PresetManager& getPresetManager() { return presetManager; }
    const PresetManager& getPresetManager() const { return presetManager; }
    
    // Interactive pattern modification methods
    void togglePatternStep(int stepIndex);
    void toggleAccentAtStep(int stepIndex);
    bool isValidStepIndex(int stepIndex) const;
    void updateUPIFromCurrentPattern();
    
    // Progressive offset support (universal for all patterns)
    void resetProgressiveOffset() { progressiveOffset = 0; }
    void advanceProgressiveOffset() { progressiveOffset += progressiveStep; }
    int getProgressiveOffset() const { return progressiveOffset; }

    /** Advance a MONO `%N`/`+N`/`*N` for a re-trigger. Returns true if it did.
        Shared by the tick edge and the MIDI-note path so the two cannot drift
        apart again — that drift was the bug (see the definition). */
    bool advanceMonoProgressiveForTrigger(const juce::String& upiToProcess);
    
    // Scene information access for UI
    int getCurrentSceneIndex() const { return sceneManager->getCurrentSceneIndex(); }
    int getSceneCount() const { return sceneManager->getSceneCount(); }
    
    // Progressive transformation access for UI  
    int getProgressiveTriggerCount() const;
    bool hasProgressiveOffset() const { return patternEngine.hasProgressiveOffsetEnabled(); }
    
    // Progressive lengthening support (universal for all patterns)
    void resetProgressiveLengthening() { progressiveLengthening = 0; baseLengthPattern.clear(); }
    void advanceProgressiveLengthening();
    
    // Scene cycling support (universal for all patterns)
    void resetScenes() { sceneManager->resetScenes(); }
    void advanceScene();
    
    // Accent system access for UI and processing
    bool getHasAccentPattern() const { return hasAccentPattern; }
    const std::vector<bool>& getCurrentAccentPattern() const { return currentAccentPattern; }
    int getGlobalOnsetCounter() const { 
        // PHASE 3: Use derived onset count
        return static_cast<int>(getCurrentOnsetCount()); 
    }
    bool shouldOnsetBeAccented(int onsetNumber) const; // DEPRECATED: onset-based logic
    bool shouldStepBeAccented(int stepIndex) const;    // NEW: step-based logic for MIDI alignment
    std::vector<bool> getCurrentAccentMap() const;
    bool checkPatternChanged(); // Check and reset pattern changed flag
    void resetAccentSystem();
    void generatePreCalculatedAccentMap(); // Generate deterministic accent map for UI
    
    
    
    // Debug info for UI display
    int getDebugTriggerCount() const { return debugTriggerCount; }
    int getDebugActiveNotesCount() const { return static_cast<int>(activeNotes.size()); }
    int getDebugNoteOffsSent() const { return debugNoteOffsSent; }
    int getDebugAbsoluteSamplePos() const { return absoluteSamplePosition; }
    
    // Lascabettes quantization access for UI and processing
    bool getHasQuantization() const { return hasQuantization; }
    int getOriginalStepCount() const { return originalStepCount; }
    int getQuantizedStepCount() const { return quantizedStepCount; }
    bool getQuantizationClockwise() const { return quantizationClockwise; }
    int getOriginalOnsetCount() const { return originalOnsetCount; }
    int getQuantizedOnsetCount() const { return quantizedOnsetCount; }

    // Poly lanes: read-only access for the WebView editor (SerpeEditor) so
    // it can show which lane is active and animate a real per-lane playhead
    // instead of a frozen panel. -1 = inactive or nothing played yet.
    bool getIsPolyPattern() const { return isPolyPattern; }
    int getPolyLaneStep(int laneIndex) const
    {
        if (laneIndex < 0 || laneIndex >= kMaxPolyLanes) return -1;
        const auto& lane = polyLanes[static_cast<size_t>(laneIndex)];
        return lane.active ? lane.lastProcessedStep : -1;
    }

    /**
     * What this lane is ACTUALLY sounding, as a binary string — rotated by its
     * progressive offset, grown by its lengthening, and resolved to the scene
     * the lane is currently on. The editor used to draw lane rows from the
     * WebUI's own parse of the typed text, which meant a chain always displayed
     * its first scene while the engine cycled behind it (Alex, 2026-07-29).
     * Empty when the lane is inactive.
     */
    juce::String getPolyLanePattern(int laneIndex) const
    {
        if (laneIndex < 0 || laneIndex >= kMaxPolyLanes) return {};
        const auto& lane = polyLanes[static_cast<size_t>(laneIndex)];
        if (!lane.active) return {};
        return lane.engine.getBinaryString();
    }

    /** Which scene of its own chain this lane is on, and how many it has. */
    int getPolyLaneSceneIndex(int laneIndex) const
    {
        if (laneIndex < 0 || laneIndex >= kMaxPolyLanes) return 0;
        return polyLanes[static_cast<size_t>(laneIndex)].sceneIndex;
    }
    int getPolyLaneSceneCount(int laneIndex) const
    {
        if (laneIndex < 0 || laneIndex >= kMaxPolyLanes) return 0;
        return polyLanes[static_cast<size_t>(laneIndex)].sceneChain.size();
    }

    /** How many times this lane has been triggered on the scene it is
        currently sounding, 1-based — trigger 1 is the bare base (INTENT D6).

        This is the number the UI shows, and it is READ from the engine rather
        than counted alongside it: a display that keeps its own tally drifts
        from the thing it describes, which is how a rotating pattern once sat
        frozen for a week without anyone noticing (DESIGN_BRIEF §3.2).

        `sceneVisits` is the right source because it is already what progressive
        OFFSET is derived from (offset = step * (visits - 1)), so the readout
        and the sound cannot disagree by construction. */
    int getPolyLaneTriggerIndex(int laneIndex) const
    {
        if (laneIndex < 0 || laneIndex >= kMaxPolyLanes) return 0;
        const auto& lane = polyLanes[static_cast<size_t>(laneIndex)];
        if (!lane.active) return 0;
        const size_t sc = static_cast<size_t>(juce::jlimit(0, juce::jmax(0, (int) lane.sceneVisits.size() - 1),
                                                           lane.sceneIndex));
        return sc < lane.sceneVisits.size() ? lane.sceneVisits[sc] : 0;
    }

private:
    //==============================================================================
    // Pattern Engine
    PatternEngine patternEngine;
    
    // PHASE 2: Pattern Update Queue for thread-safe pattern changes
    PatternUpdateQueue patternUpdateQueue;
    
    // MIDI effect mode - no audio synthesis components needed
    
    // Timing and sequencing
    double currentSampleRate = 44100.0;
    int samplesPerStep = 0;

    // ── Feel: microtiming (PD) + note length (LS) ────────────────────────────
    // Both come off the UPI string (see UPIParser::extract*). Microtiming
    // displaces WHERE a step fires; longShort sets how long the note lasts.
    // Recomputed on pattern/parameter change, never in the audio callback.
    std::vector<double> microtimingShift;   // per step, fractions of a step
    double microtimingDepth = 0.0;
    int microtimingSeed = 1;
    int microtimingCycle = 0;               // bumped per cycle so passes differ
    bool hasLongShort = false;
    double longShortMin = 1.0, longShortMax = 1.0, longShortDepth = 0.0;
    void rebuildMicrotiming();
    /** Step whose displaced position `pos` (continuous, in steps) has reached. */
    int displacedStep (double pos, int stepCount) const;
    int currentSample = 0;
    std::atomic<int> currentStep{0};  // Legacy - will be replaced by derived indices
    bool wasPlaying = false;
    
    /**
     * PHASE 1: MONOTONIC TRANSPORT TICK SYSTEM
     * 
     * This is the heart of the derived indices architecture that eliminates accent swirling.
     * All timing calculations derive from these atomic counters, ensuring consistency
     * between UI and MIDI systems.
     * 
     * transportTick: Monotonically increasing step counter, never decreases during playback
     * baseTickRhythm: Reference point for rhythm pattern position calculation  
     * baseTickAccent: Reference point for accent pattern position (can have phase offset)
     * 
     * Key insight: Instead of maintaining mutable step counters that can drift,
     * we calculate current positions mathematically from these base references.
     */
    std::atomic<uint64_t> transportTick{0};           // Monotonic step counter
    std::atomic<uint64_t> baseTickRhythm{0};          // Rhythm reference point  
    std::atomic<uint64_t> baseTickAccent{0};          // Accent reference point
    std::atomic<uint32_t> lastMidiOnsetCount{0};      // Track MIDI onset count for UI sync
    double samplesPerStepPrecise = 0.0;               // Precise step timing
    uint64_t sampleAccumulator = 0;                   // For precise step timing
    
    // Note tracking system for proper note duration management
    struct ActiveNote {
        int noteNumber;
        int endSample;  // Absolute sample position when note should end
        bool isActive;
        int channel;    // 1-16; poly lanes may use channels other than 1

        ActiveNote() : noteNumber(0), endSample(0), isActive(false), channel(1) {}
        ActiveNote(int note, int end, int ch = 1) : noteNumber(note), endSample(end), isActive(true), channel(ch) {}
    };

    std::vector<ActiveNote> activeNotes;
    int absoluteSamplePosition = 0;  // Track absolute sample position across buffers

    // ── Poly lanes (music-suite docs/SERPE_POLY.md §8 milestone 2) ──────────
    // Engaged only when currentUPIInput's top-level PolyParser::splitLanes
    // count is > 1. Deliberately kept structurally separate from the mono
    // members above (patternEngine, samplesPerStep, the transport-tick
    // system): a plain (non-'/') UPI string never touches anything below,
    // so mono behaviour is provably unaffected by any of this.
public:
    static constexpr int kMaxPolyLanes = 6;
private:
    struct PolyLaneRuntime
    {
        PatternEngine engine;
        bool active = false;
        juce::String source;             // this lane's own UPI body (re-parsed on progressive triggers)
        PolyOffset offset;                // Keil micro-timing offset (@ms / @frac), none = dead on the grid
        bool hasProgressiveOffset = false;
        int progressiveOffsetStep = 0;    // amount to advance per trigger (0 = not progressive)

        // Progressive LENGTHENING (`body*N`) for this lane. `grown` is the
        // pattern as it stands, extended by `step` fresh steps per trigger —
        // kept rather than recomputed so earlier growth STAYS put and the lane
        // actually grows, instead of re-randomising its whole tail each time.
        bool hasProgressiveLengthening = false;
        int progressiveLengtheningStep = 0;

        // Scenes are per LANE ('/' binds loosest), and every lane advances its
        // OWN chain on every trigger — so a 2-scene lane against a 3-scene one
        // takes 6 triggers to come back round. That independence is the whole
        // reason to put a chain on a lane rather than on the string.
        juce::StringArray sceneChain;     // this lane's scenes, as typed
        int sceneIndex = 0;
        // Progressive state is per (lane, scene): scene 2 keeps growing while
        // scene 1 keeps rotating, each remembering where it got to. Sized to
        // sceneChain, so index by sceneIndex.
        // How many times each scene has been entered. Progressive state is
        // DERIVED from this (offset = step * visits) rather than accumulated,
        // so it cannot drift out of step with the scene it belongs to.
        std::vector<int> sceneVisits;
        std::vector<std::vector<bool>> sceneGrown;  // grown pattern per scene
        int lastProcessedStep = -1;       // this lane's own step-boundary tracking, cycle-lock clock

        // This lane's OWN microtiming walk (PD on the lane body). Per-lane by
        // necessity: one shared walk would displace every lane identically,
        // which is not a groove, just a latency — and is exactly why
        // `E(3,8) PD(90%)/E(3,8) PD(10%)` sounded locked together. `shift` is
        // sized to the lane's step count and rebuilt off the audio thread, or
        // at a cycle boundary where it is already the right size and therefore
        // allocation-free.
        double microtimingDepth = 0.0;
        int microtimingSeed = 1;
        int microtimingCycle = 0;
        std::vector<double> microtimingShift;

        // This lane's own accent layer (the `{…}` written on THIS lane —
        // INTENT §D8, accents are per-lane because '/' binds loosest). Indexed
        // by cumulative onset like mono's, and derived from the lane clock at
        // trigger time rather than counted, so it cannot drift.
        bool hasAccentPattern = false;
        std::vector<bool> accentPattern;

        // This lane's OWN `>N` progressive bookkeeping, alongside its accent
        // layer and its microtiming walk for the same reason all three are here:
        // a lane's state is the LANE's. Every lane used to parse against one
        // process-wide map keyed by pattern text, so `E(1,8)>8/E(1,8)>8` had two
        // lanes advancing a single counter and they came apart on trigger 1
        // (SERPE_DAW_FINDINGS_2026-08 F1a, measured before the fix).
        ProgressiveTransformState progressive;
    };
    bool isPolyPattern = false;
    std::array<PolyLaneRuntime, kMaxPolyLanes> polyLanes;
    /** Redraw one lane's own microtiming walk (the poly twin of rebuildMicrotiming). */
    void rebuildLaneMicrotiming(PolyLaneRuntime& lane);

    juce::AudioParameterInt*    laneNoteParams[kMaxPolyLanes]    = {};
    juce::AudioParameterInt*    laneChannelParams[kMaxPolyLanes] = {};
    juce::AudioParameterBool*   laneMuteParams[kMaxPolyLanes]    = {};
    juce::AudioParameterFloat*  polyLagMsParam = nullptr;
    // Cycle lock (index 0, default) vs step lock (index 1) — music-suite
    // docs/SERPE_POLY.md §3b. Automatable/host-recallable like every other
    // poly param; previously the webapp's own toggle only, no C++ backing.
    juce::AudioParameterChoice* polyLockParam = nullptr;

    void parseAndApplyPolyUPI(const juce::String& upiPattern);
    void processPolyLanes(juce::MidiBuffer& midiBuffer, int numSamples, double ppqPosition);
    void triggerPolyNote(juce::MidiBuffer& midiBuffer, int samplePosition, int numSamples, int laneIndex, bool isAccented);
    void clearAllPolyActiveNotes(juce::MidiBuffer& midiBuffer);
    // Beats-per-cycle for the poly pattern as a whole (mirrors the mono
    // path's patternLengthInBeats calculation exactly, but as a standalone
    // helper so the mono processBlock code is never touched by this work).
    double computePolyCycleLengthInBeats(const std::vector<bool>& referencePattern) const;
    
    // DAW transport sync
    bool useHostTransport = true;
    double lastHostPosition = 0.0;
    bool hostIsPlaying = false;
    mutable double lastProcessBlockTime = 0.0;
    
    // Host loop sync (Phase 3: Advanced Host Sync)
    bool hostIsLooping = false;
    double hostLoopStart = 0.0;
    double hostLoopEnd = 0.0;
    bool enableLoopSync = true;
    
    // Internal state (not exposed as parameters)
    float currentBPM = 120.0f;
    bool internalPlaying = false;
    bool lastTickState = false;
    int tickResetCounter = 0;
    
    // UPI pattern input
    juce::String currentUPIInput;
    juce::String originalUPIInput; // Preserve original pattern with progressive/scene syntax
    juce::String lastProgressiveTransformUPI; // Last `>`-transform TYPED, so a fresh entry resets its step state (tick still advances)
    juce::String lastParsedUPI;
    juce::String currentProgressivePatternKey; // Track current progressive pattern for step counting

    /**
     * THIS INSTANCE's `>N` progressive bookkeeping, for the mono path. (Poly
     * lanes each own theirs — see PolyLaneRuntime::progressive.)
     *
     * It used to be file-scope statics in UPIParser.cpp, one map per process:
     * two tracks running the same pattern text fought over one counter, and a
     * newly opened project inherited whatever the previous one left behind.
     * `currentProgressivePatternKey` right above has always been saved with the
     * project — the KEY, into state the project file could not reach. This is
     * the other half, and it is saved next to it now (F1).
     */
    ProgressiveTransformState progressiveTransform;


    // Background color persistence
    int currentBackgroundColor = 0; // Default to Dark background
    
    // UPI history (ticker tape feature)
    juce::StringArray upiHistory;
    static constexpr int MAX_UPI_HISTORY = 20;
    
    // Preset management
    PresetManager presetManager;
    
    // Lascabettes quantization metadata
    bool hasQuantization = false;
    int originalStepCount = 0;
    int quantizedStepCount = 0;
    bool quantizationClockwise = true;
    int originalOnsetCount = 0;
    int quantizedOnsetCount = 0;
    
    
    // Progressive offset support (works for any pattern)
    int progressiveOffset = 0;      // Current accumulated offset
    int progressiveStep = 0;        // How much to advance each time
    juce::String basePattern;       // Pattern without progressive syntax
    
    // Progressive lengthening support (works for any pattern)
    int progressiveLengthening = 0; // How many steps to add each time
    std::vector<bool> baseLengthPattern; // Original pattern for lengthening
    std::mt19937 randomGenerator;   // For bell curve random step generation
    
    // Scene cycling support (works for any pattern). SceneManager owns all of
    // it — patterns, current index, per-scene progressive state, persistence.
    // Constructed in the constructor and never released, so it is always valid.
    std::unique_ptr<SceneManager> sceneManager;
    
    // Sequence of the most recently enqueued pattern update, so the audio-thread
    // drain can echo it into the dataflow trace. Diagnostic only: read and
    // written solely by the tracing calls, and never consulted by the engine.
    std::atomic<int> queuedSeq { 0 };
    // Per-lane seq for the traced `laneScenes` channel, same diagnostic-only role.
    std::array<int, kMaxPolyLanes> polyLaneSeq {};

    // Thread safety
    juce::CriticalSection processingLock;
    
    // Pattern change notification for UI updates
    std::atomic<bool> patternChanged{false};
    
    // Debug counters for UI display
    std::atomic<int> debugTriggerCount{0};
    std::atomic<int> debugNoteOffsSent{0};
    
    // Accent system - single source of truth
    bool hasAccentPattern = false;
    std::vector<bool> currentAccentPattern;
    int globalOnsetCounter = 0;           // Single source of truth: counts all onsets since pattern start
    int uiAccentOffset = 0;               // Stable accent offset for UI display (updates only at cycle boundaries)
    bool accentPatternManuallyModified = false; // Flag to prevent automatic accent cycling after manual edits
    bool patternManuallyModified = false;       // Flag to indicate pattern has been manually edited (suspension mode)
    std::vector<bool> suspendedRhythmPattern;   // Preserve manually modified rhythm pattern
    std::vector<bool> suspendedAccentPattern;   // Preserve manually modified accent pattern
    
    // Pre-calculated deterministic accent map for UI synchronization  
    std::vector<bool> preCalculatedAccentMap;   // Maps step index -> should be accented
    mutable std::atomic<bool> accentMapNeedsUpdate{true}; // Flag to regenerate map when pattern changes
    
    
    // Parameters - implementation details
    juce::AudioParameterBool* useHostTransportParam;
    juce::AudioParameterInt* midiNoteParam;
    juce::AudioParameterBool* tickParam;
    juce::AudioParameterChoice* patternLengthUnitParam;
    juce::AudioParameterChoice* patternLengthValueParam;
    juce::AudioParameterChoice* subdivisionParam;
    
    // Accent parameters
    juce::AudioParameterFloat* accentVelocityParam;
    juce::AudioParameterFloat* unaccentedVelocityParam;
    juce::AudioParameterInt* accentPitchOffsetParam;
    
    // AudioProcessorValueTreeState for robust state management
    juce::AudioProcessorValueTreeState parameters;
    
    // Helper methods
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateTiming();
    void processStep(juce::MidiBuffer& midiBuffer, int samplePosition, int stepToProcess);
    void triggerNote(juce::MidiBuffer& midiBuffer, int samplePosition, bool isAccented = false);
    void syncBPMWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void syncPositionWithHost(const juce::AudioPlayHead::CurrentPositionInfo& posInfo);
    void checkMidiInputForTriggers(juce::MidiBuffer& midiMessages);
    
    // PHASE 1: Monotonic Transport Tick helper methods
    void updateTransportTick(int bufferSize);
    uint32_t getCurrentRhythmStep() const;
    uint32_t getCurrentAccentStep() const;
    
    // PHASE 2: Pattern Update Queue methods
    void processPatternUpdates();
    void queuePatternUpdate(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern = {}, bool hasAccent = false, int accentPhaseOffset = 0);
    
    // PHASE 3: Derived onset counting
    uint32_t getCurrentOnsetCount() const;
    
    // PHASE 3: Emergency fix for direct pattern setting
    void setPatternWithPhaseSync(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern = {}, bool hasAccent = false, int accentPhaseOffset = 0);
    
    // PHASE 0: Derived index functions - proof of concept
    uint64_t getMonotonicTick() const;
    uint32_t getDerivedRhythmStep() const;
    uint32_t getDerivedAccentStep() const;
    void validateCounterConsistency() const; // Debug validation
    
    // Note tracking system methods
    void addActiveNote(int noteNumber, int duration, int channel = 1);
    void processActiveNotes(juce::MidiBuffer& midiBuffer, int bufferSize);
    void clearAllActiveNotes(juce::MidiBuffer& midiBuffer);
    
    
    // Pattern manipulation
    std::vector<bool> generateBellCurveRandomSteps(int numSteps);
    std::vector<bool> lengthenPattern(const std::vector<bool>& pattern, int additionalSteps);
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SerpeAudioProcessor)
};