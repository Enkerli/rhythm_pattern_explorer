/*
    SceneTrace — temporary instrumentation for the scene-advance trigger bug.

    Symptom (Alex, 2026-07-28): with a scene chain like E(3,8)%2|E(3,8)*3 one
    trigger behaves like several. Reproduces on Enter and on MIDI note in.
    "There's a trigger problem in scene advance. Not for the first time."

    Static reading has produced two wrong guesses already. There are three
    advanceScene() call sites and it is not obvious which fire per event, so
    this records the actual order of events instead of arguing about it.

    Unlike SceneCompare (which counted), this keeps a ring buffer of the LAST
    events in sequence — the question here is "what happened, in what order,
    for ONE key press", and only a trace answers that.

    Audio-thread safe: processBlock reaches most of these sites, so an event is
    a POD struct written into a fixed ring buffer with an atomic cursor. No
    allocation, no locks, no file I/O on that path. The file is written from
    the message thread only.

        ~/Library/Serpe/scene-trace.log

    DELETE this file and its call sites once the bug is fixed.
*/
#pragma once

#include <JuceHeader.h>
#include <atomic>

class SceneTrace
{
public:
    // Every point that can move, apply, or re-enter a scene.
    enum Site : juce::uint8
    {
        TickEdge = 0,      // processBlock: tick parameter rising edge
        MidiNote,          // processBlock: MIDI note in
        UpiSubmit,         // setUPIInput received a chain (Enter, preset, restore)
        UpiSameSequence,   // parseAndApplyUPI saw an UNCHANGED chain -> advances
        UpiNewSequence,    // parseAndApplyUPI saw a NEW chain -> initialises
        AdvanceScene,      // SerpeAudioProcessor::advanceScene actually ran
        ApplyScene,        // applyCurrentScenePattern actually ran
        NumSites
    };

    static const char* siteName (juce::uint8 s)
    {
        switch (s)
        {
            case TickEdge:        return "tick-edge";
            case MidiNote:        return "midi-note";
            case UpiSubmit:       return "upi-submit";
            case UpiSameSequence: return "upi-same-sequence -> advance";
            case UpiNewSequence:  return "upi-new-sequence  -> initialise";
            case AdvanceScene:    return "  ADVANCE";
            case ApplyScene:      return "  apply";
            default:              return "?";
        }
    }

    /**
        Audio-thread safe. `index`/`offset`/`lengthening` are the state AFTER the
        step, and `bits` is what the ENGINE actually holds at that moment — the
        one field that separates "the scene never advanced" from "it advanced and
        the display never followed".
    */
    void record (Site site, int index, int offset, int lengthening, const juce::String& bits)
    {
        const auto slot = cursor.fetch_add (1, std::memory_order_relaxed);
        auto& e = events[slot % kCapacity];
        e.ms          = juce::Time::getMillisecondCounter();
        e.site        = static_cast<juce::uint8> (site);
        e.index       = static_cast<juce::int16> (index);
        e.offset      = static_cast<juce::int16> (offset);
        e.lengthening = static_cast<juce::int16> (lengthening);
        e.steps       = static_cast<juce::int16> (bits.length());
        // Fixed buffer, no allocation on the audio thread. Long patterns are
        // truncated; the step count above still reports the true length.
        const int n = juce::jmin (static_cast<int> (kBitsShown), bits.length());
        for (int i = 0; i < n; ++i) e.bits[i] = bits[i] == '1' ? '1' : '0';
        e.bits[n] = 0;
        e.valid       = true;
    }

    /** MESSAGE THREAD ONLY. Rewrites the file whole, so it is always readable. */
    void writeTrace() const
    {
        auto file = traceFile();
        file.getParentDirectory().createDirectory();

        const auto total = cursor.load (std::memory_order_relaxed);
        const auto shown = juce::jmin (total, static_cast<juce::uint32> (kCapacity));
        const auto first = total - shown;

        juce::String out;
        out << "Serpe - scene advance trace\n"
            << "written " << juce::Time::getCurrentTime().toISO8601 (true) << "\n"
            << static_cast<int> (total) << " events, showing the last "
            << static_cast<int> (shown) << "\n\n"
            << "A blank line separates events more than 250 ms apart, so one key\n"
            << "press or MIDI note is one block. Each block should contain exactly\n"
            << "ONE ADVANCE. More than one is the bug.\n\n"
            << "    +ms  site                          scene  offset  length  steps  engine pattern\n";

        juce::uint32 previousMs = 0;
        for (juce::uint32 i = first; i < total; ++i)
        {
            const auto& e = events[i % kCapacity];
            if (! e.valid) continue;

            if (previousMs != 0 && e.ms - previousMs > 250) out << "\n";
            const auto delta = previousMs == 0 ? 0 : (e.ms - previousMs);
            previousMs = e.ms;

            out << juce::String (static_cast<int> (delta)).paddedLeft (' ', 7) << "  "
                << juce::String (siteName (e.site)).paddedRight (' ', 30)
                << juce::String (e.index).paddedLeft (' ', 5)
                << juce::String (e.offset).paddedLeft (' ', 8)
                << juce::String (e.lengthening).paddedLeft (' ', 8)
                << juce::String (e.steps).paddedLeft (' ', 7) << "  "
                << juce::String (e.bits) << "\n";
        }

        if (total == 0)
            out << "\n(nothing recorded - play a scene chain, then read this again)\n";

        file.replaceWithText (out);
    }

    /** MESSAGE THREAD ONLY. Writes only when new events have arrived. */
    void writeTraceIfChanged()
    {
        const auto total = cursor.load (std::memory_order_relaxed);
        if (total == lastWrittenTotal) return;

        const auto now = juce::Time::getMillisecondCounter();
        if (lastWriteMs != 0 && now - lastWriteMs < 1000) return;

        lastWrittenTotal = total;
        lastWriteMs = now;
        writeTrace();
    }

    static juce::File traceFile()
    {
        // NOTE: userApplicationDataDirectory is ~/Library on macOS, not
        // ~/Library/Application Support.
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Serpe")
                   .getChildFile ("scene-trace.log");
    }

private:
    static constexpr size_t kCapacity = 512;
    static constexpr size_t kBitsShown = 32;

    struct Event
    {
        juce::uint32 ms = 0;
        juce::uint8 site = 0;
        juce::int16 index = 0;
        juce::int16 offset = 0;
        juce::int16 lengthening = 0;
        juce::int16 steps = 0;
        char bits[kBitsShown + 1] = {};
        bool valid = false;
    };

    Event events[kCapacity];
    std::atomic<juce::uint32> cursor { 0 };
    juce::uint32 lastWrittenTotal = 0;   // message thread only
    juce::uint32 lastWriteMs = 0;        // message thread only
};
