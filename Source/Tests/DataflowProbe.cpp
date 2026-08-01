/*
    Headless dataflow probe — drive the real processor, capture what moved.

    Modelled on Vane's tools/RenderProbe: instantiate the actual processor, run a
    scripted session offline, and write something measurable. That workflow turned
    "sub-bass / no sustained tone" into numbers; this turns "the display is stuck"
    into a trace an analyser can rule on.

    Produces two artifacts under scratch/dataflow/ (gitignored):

      <name>.jsonl   the dataflow trace, for tools/dataflow/audit.mjs
      <name>.mid     the MIDI the run actually produced

    The MIDI matters. A verdict of "delivered" is not a claim about the music:
    E(3,8)%2|E(3,8)*3/E(3,7) advanced correctly for hours while sounding wrong to
    a person. The file is what lets that be checked later, by ear or by analysis.

    No UI, no host, no audio device — so it runs in CI and in a sandbox, and it
    exercises the within-binary channels where two of this session's bugs lived.
*/
#include "../Platform/PluginProcessor.h"
#include "../Platform/DataflowTrace.h"
#include <cstdio>

/**
 * The probe's own editor factory: none. The plugin's real one lives in
 * Source/WebUI/EditorFactory.cpp and is not compiled here, which is precisely
 * what keeps juce_gui_extra — and on Linux, GTK — out of this binary.
 */
juce::AudioProcessorEditor* SerpeAudioProcessor::createEditor() { return nullptr; }

namespace
{
constexpr double kSampleRate = 48000.0;
constexpr int    kBlock      = 512;

/** One scripted session: a pattern, then N triggers delivered as MIDI notes. */
struct Session
{
    const char* name;
    const char* upi;
    int triggers;

    // Blocks to run AFTER the triggers with no MIDI in — just the transport
    // running. A trigger is one 512-sample block (~11ms), so a handful of them
    // covers a fraction of one step: enough to watch progressive state advance,
    // nowhere near enough to hear a pattern. Anything about what the pattern
    // SOUNDS like (accents, most obviously) needs the clock to actually run,
    // and 750 blocks is ~8s, two cycles of the 8-beat default at 120bpm.
    int idleBlocks = 0;

    // How many distinct (note number, velocity) pairs the session must emit.
    // 0 = don't check. This is the accent assertion: an accent is a LOUDER,
    // TRANSPOSED note (INTENT §D8 / F3), so a pattern with accents that lands
    // on only one pair is playing flat — exactly the poly bug of 2026-08-01.
    int expectPairs = 0;
};

/**
 * Drive `triggers` MIDI notes through the processor, collecting everything it
 * emits. Each note is a separate block, which is how a player actually plays —
 * and it is the path that failed for poly until 2026-07-30, when MIDI-in did not
 * advance a lane's scene chain at all.
 */
/** Bit string for a pattern, leftmost = first step = LSB (INTENT D1). */
juce::String bits (const std::vector<bool>& v)
{
    juce::String s;
    for (bool b : v) s << (b ? '1' : '0');
    return s;
}

juce::MidiMessageSequence run (SerpeAudioProcessor& proc, const Session& s,
                               juce::StringArray* perTrigger = nullptr)
{
    juce::MidiMessageSequence captured;
    juce::AudioBuffer<float> audio (2, kBlock);
    double seconds = 0.0;

    proc.setUPIInput (juce::String (s.upi));
    proc.setInternalPlaying (true);

    for (int i = 0; i < s.triggers; ++i)
    {
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 36, 0.8f), 0);
        midi.addEvent (juce::MidiMessage::noteOff (1, 36), 32);

        audio.clear();
        proc.processBlock (audio, midi);

        for (const auto meta : midi)
        {
            auto m = meta.getMessage();
            m.setTimeStamp (seconds + meta.samplePosition / kSampleRate);
            captured.addEvent (m);
        }
        seconds += kBlock / kSampleRate;

        // One settle block before sampling.
        //
        // processBlock drains the pattern queue at its TOP (processPatternUpdates,
        // before checkMidiInputForTriggers), so a '%N' rotation enqueued by this
        // trigger does not reach the engine until the NEXT block --- deliberate,
        // it is what stops the queued base from overwriting the rotation. '*N'
        // by contrast sets the engine directly and lands at once.
        //
        // Sampling immediately after the trigger therefore read '%N' one step
        // behind '*N' and made a correct engine look inconsistent. Draining
        // first reports what both operators have actually settled on.
        {
            juce::MidiBuffer settle;
            audio.clear();
            proc.processBlock (audio, settle);
            for (const auto meta : settle)
            {
                auto m = meta.getMessage();
                m.setTimeStamp (seconds + meta.samplePosition / kSampleRate);
                captured.addEvent (m);
            }
            seconds += kBlock / kSampleRate;
        }

        // The pattern the engine is actually on, per trigger. This is what
        // makes progressive PHASE observable: trigger 1 must be the bare base
        // for every operator (INTENT D6, base-first since 2026-07-30), and a
        // MIDI file alone cannot show that.
        //
        // A POLY pattern's lanes each own their own PatternEngine, so the
        // processor's mono patternEngine says nothing about them --- it holds
        // whatever was last parsed and sits there looking static. Printing it
        // for a poly session invited exactly the wrong conclusion, so read the
        // lanes instead.
        if (perTrigger != nullptr)
        {
            if (proc.getIsPolyPattern())
            {
                juce::StringArray lanes;
                for (int i = 0; i < 4; ++i)
                {
                    const auto p = proc.getPolyLanePattern (i);
                    if (p.isEmpty()) break;
                    lanes.add (p);
                }
                perTrigger->add (lanes.isEmpty() ? juce::String ("(no lanes)")
                                                 : lanes.joinIntoString ("+"));
            }
            else
            {
                perTrigger->add (bits (proc.getPatternEngine().getCurrentPattern()));
            }
        }

        // Flush from this thread: the probe IS the message thread here, and
        // flushing per trigger keeps the ring far from overflowing.
        DataflowTrace::instance().flush();
    }

    // Drain blocks. The last trigger enqueues a pattern update that only the
    // NEXT processBlock consumes, and a DAW keeps calling — so stopping straight
    // after the final trigger left one update in flight and the audit dutifully
    // reported it as DROPPED. That was the harness, not the plugin: a tail
    // enqueue with no drain is not a lost message. Verified by this fix removing
    // the finding, which is the discipline the whole tool is for.
    //
    // Then s.idleBlocks of plain transport, captured like everything else: the
    // only way a session hears more than the first step of its own pattern.
    for (int i = 0; i < 2 + s.idleBlocks; ++i)
    {
        juce::MidiBuffer empty;
        audio.clear();
        proc.processBlock (audio, empty);
        for (const auto meta : empty)
        {
            auto m = meta.getMessage();
            m.setTimeStamp (seconds + meta.samplePosition / kSampleRate);
            captured.addEvent (m);
        }
        seconds += kBlock / kSampleRate;
        DataflowTrace::instance().flush();
    }

    proc.setInternalPlaying (false);
    captured.updateMatchedPairs();
    return captured;
}

/**
 * The distinct (note number, velocity) pairs among a session's note-ons, with
 * how many times each was heard, in first-heard order. What an accent actually
 * IS on the wire: an accented onset arrives as note+accentPitchOffset at
 * accentVelocity, so ONE pair means flat and two or more mean the accent layer
 * reached the output.
 *
 * The counts matter as much as the count of pairs: two pairs where one appears
 * once is a stuck note, not an accent pattern.
 */
juce::StringArray notePairs (const juce::MidiMessageSequence& seq)
{
    juce::StringArray pairs;
    std::vector<int> counts;
    for (int i = 0; i < seq.getNumEvents(); ++i)
    {
        const auto& m = seq.getEventPointer (i)->message;
        if (! m.isNoteOn()) continue;
        const auto pair = "note " + juce::String (m.getNoteNumber())
                        + " vel " + juce::String (m.getVelocity());
        const int at = pairs.indexOf (pair);
        if (at < 0) { pairs.add (pair); counts.push_back (1); }
        else ++counts[(size_t) at];
    }
    for (int i = 0; i < pairs.size(); ++i)
        pairs.set (i, pairs[i] + " x" + juce::String (counts[(size_t) i]));
    return pairs;
}

/**
    Write the captured sequence as a playable .mid.

    UNITS, and this was wrong from the probe's first commit until 2026-08-01:
    `run()` timestamps events in SECONDS, but MidiFile::writeTo interprets a
    sequence's timestamps as TICKS. With 960 ticks per quarter note, a whole
    second was being written as 1/960th of one — so every artifact collapsed
    into a few milliseconds, several notes deep, and the accent-poly file put
    all 34 note-ons inside 0.004 beats.

    Nothing downstream noticed, because the analyser reads the JSONL trace and
    the probe only ever counted MIDI events. The files were checked for
    existence and event count, never for timing. That is the whole point of
    these artifacts ("checked later, by ear or by analysis") quietly not
    working, which is L4 again: a step that reports success while producing
    nothing usable.

    Converted here rather than in `run()` so the capture stays in real time and
    only the file-writing boundary deals in ticks.
*/
void writeMidi (const juce::File& out, const juce::MidiMessageSequence& seq)
{
    constexpr int    kTicksPerQuarter = 960;
    constexpr double kBpm             = 120.0;   // the processor's default
    const double ticksPerSecond = kTicksPerQuarter * kBpm / 60.0;

    juce::MidiMessageSequence ticks;
    for (int i = 0; i < seq.getNumEvents(); ++i)
    {
        auto m = seq.getEventPointer (i)->message;
        m.setTimeStamp (m.getTimeStamp() * ticksPerSecond);
        ticks.addEvent (m);
    }
    ticks.updateMatchedPairs();

    // State the tempo in the file, so anything opening it reads the same clock
    // the conversion above assumed instead of guessing 120 itself.
    juce::MidiMessageSequence meta;
    meta.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60'000'000.0 / kBpm)));

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (kTicksPerQuarter);
    mf.addTrack (meta);
    mf.addTrack (ticks);
    out.deleteFile();
    if (auto stream = std::unique_ptr<juce::FileOutputStream> (out.createOutputStream()))
        mf.writeTo (*stream);
}
} // namespace

int main (int argc, char** argv)
{
    // Artifacts land in the monorepo's scratch dir by default; override with an
    // argument so CI can point somewhere else.
    juce::File outDir = argc > 1
        ? juce::File (juce::String (argv[1]))
        : juce::File::getSpecialLocation (juce::File::userHomeDirectory)
              .getChildFile ("Documents/Coding/music-suite/scratch/dataflow");
    outDir.createDirectory();

    const std::vector<Session> sessions {
        // Mono progressive offset and lengthening — the two paths changed on
        // 2026-07-30 for base-first, and until then exercised by no probe at
        // all. Trigger 1 must print the bare base in both.
        { "serpe-mono-offset", "E(3,8)%2", 5 },
        { "serpe-mono-lengthen", "E(3,8)*3", 4 },
        // The chain that started all of this: a scene chain inside a poly lane,
        // with a progressive lengthening on the second scene.
        { "serpe-poly-scenes", "E(3,8)|E(3,8)*3/E(3,7)", 8 },
        // Mono progressive transform — the path that always worked, as a control.
        { "serpe-mono-transform", "E(1,8)>8", 8 },
        // Per-lane progressive offset, added 2026-07-29.
        { "serpe-lane-offset", "E(3,8)%2/E(3,7)", 6 },
        // Accents, both paths, added 2026-08-01. These run the transport
        // (idleBlocks) rather than only triggering: an accent is a property of
        // the SEQUENCE of onsets, so a session that never reaches its second
        // onset cannot tell an accent layer from a flat one.
        //
        // Mono is the reference — it always worked (F3): {10010} over E(5,8)'s
        // five onsets accents the 1st and 4th, so both a plain and an accented
        // pair must appear.
        { "serpe-accent-mono", "{10010}E(5,8)", 2, 750, 2 },
        // The poly case Alex hit in Logic (F2): flat until 2026-08-01, when the
        // lane's own `{…}` started reaching triggerPolyNote. The brace belongs
        // to LANE 1 ('/' binds loosest — INTENT §D4/§D8), so lane 1 alternates
        // accented and plain while lane 2 stays plain throughout.
        { "serpe-accent-poly", "{1001010}E(5,8)/E(1,17)>17", 2, 750, 2 },
    };

    int failures = 0;
    for (const auto& s : sessions)
    {
        const auto stem = outDir.getChildFile (s.name);
        auto& tr = DataflowTrace::instance();
        tr.start (stem.withFileExtension ("jsonl"));

        SerpeAudioProcessor proc;
        proc.prepareToPlay (kSampleRate, kBlock);
        juce::StringArray perTrigger;
        const auto midi = run (proc, s, &perTrigger);
        tr.flush();
        proc.releaseResources();

        writeMidi (stem.withFileExtension ("mid"), midi);

        const auto trace = stem.withFileExtension ("jsonl");
        int lines = 0;
        if (trace.existsAsFile())
            for (const auto& l : juce::StringArray::fromLines (trace.loadFileAsString()))
                if (l.trim().startsWith ("{")) ++lines;
        std::printf ("%-22s %-26s %3d trigger(s)  %4d trace event(s)  %3d MIDI event(s)\n",
                     s.name, s.upi, s.triggers, lines, midi.getNumEvents());

        // An empty trace means the probe recorded nothing — which would let the
        // audit report a clean bill of health from no evidence. Fail loudly.
        if (! perTrigger.isEmpty())
            std::printf ("  per-trigger: %s\n", perTrigger.joinIntoString (" ").toRawUTF8());

        if (s.expectPairs > 0)
        {
            const auto pairs = notePairs (midi);
            std::printf ("  note/velocity pairs (%d): %s\n", pairs.size(),
                         pairs.joinIntoString (", ").toRawUTF8());
            if (pairs.size() < s.expectPairs)
            {
                std::printf ("  FAIL: expected at least %d distinct pair(s) — accents are not reaching the output\n",
                             s.expectPairs);
                ++failures;
            }
        }

        if (lines <= 0) { std::printf ("  FAIL: no trace events recorded\n"); ++failures; }
        if (midi.getNumEvents() == 0) std::printf ("  NOTE: no MIDI produced (pattern may not have fired in %d blocks)\n", s.triggers);
    }

    std::printf ("\n%s — artifacts in %s\n", failures ? "FAIL" : "OK",
                 outDir.getFullPathName().toRawUTF8());
    return failures == 0 ? 0 : 1;
}
