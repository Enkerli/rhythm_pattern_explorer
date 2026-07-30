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
};

/**
 * Drive `triggers` MIDI notes through the processor, collecting everything it
 * emits. Each note is a separate block, which is how a player actually plays —
 * and it is the path that failed for poly until 2026-07-30, when MIDI-in did not
 * advance a lane's scene chain at all.
 */
juce::MidiMessageSequence run (SerpeAudioProcessor& proc, const Session& s)
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
    for (int i = 0; i < 2; ++i)
    {
        juce::MidiBuffer empty;
        audio.clear();
        proc.processBlock (audio, empty);
        DataflowTrace::instance().flush();
    }

    proc.setInternalPlaying (false);
    captured.updateMatchedPairs();
    return captured;
}

void writeMidi (const juce::File& out, const juce::MidiMessageSequence& seq)
{
    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (960);
    mf.addTrack (seq);
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
        // The chain that started all of this: a scene chain inside a poly lane,
        // with a progressive lengthening on the second scene.
        { "serpe-poly-scenes", "E(3,8)|E(3,8)*3/E(3,7)", 8 },
        // Mono progressive transform — the path that always worked, as a control.
        { "serpe-mono-transform", "E(1,8)>8", 8 },
        // Per-lane progressive offset, added 2026-07-29.
        { "serpe-lane-offset", "E(3,8)%2/E(3,7)", 6 },
    };

    int failures = 0;
    for (const auto& s : sessions)
    {
        const auto stem = outDir.getChildFile (s.name);
        auto& tr = DataflowTrace::instance();
        tr.start (stem.withFileExtension ("jsonl"));

        SerpeAudioProcessor proc;
        proc.prepareToPlay (kSampleRate, kBlock);
        const auto midi = run (proc, s);
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
        if (lines <= 0) { std::printf ("  FAIL: no trace events recorded\n"); ++failures; }
        if (midi.getNumEvents() == 0) std::printf ("  NOTE: no MIDI produced (pattern may not have fired in %d blocks)\n", s.triggers);
    }

    std::printf ("\n%s — artifacts in %s\n", failures ? "FAIL" : "OK",
                 outDir.getFullPathName().toRawUTF8());
    return failures == 0 ? 0 : 1;
}
