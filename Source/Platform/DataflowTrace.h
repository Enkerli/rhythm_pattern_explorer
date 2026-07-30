/*
    DataflowTrace — record what actually moves, in the format the audit reads.

    See music-suite docs/DATAFLOW_AUDIT.md. Writes JSONL lines that
    tools/dataflow/audit.mjs turns into verdicts:

      {"t":…,"side":"cpp","scope":"within-binary","dir":"out","ch":"queuedPatternUpdate",
       "seq":7,"hash":"1a2b3c4d","summary":"11 steps 10010010101"}

    A per-channel `seq` and a payload `hash` are what make delivery PROVABLE: a
    seq the receiving end never records is a dropped message, and a differing
    hash is corruption. Name-matching cannot establish either, which is why the
    grep-based bridge audit needed 46 false positives beaten out of it.

    OFF BY DEFAULT. Enabled only when ENKERLI_DATAFLOW_TRACE names a file, so a
    shipped plugin does no work and behaves identically. `enabled()` is a single
    relaxed atomic read on the hot path.

    THREADING. Recording is lock-free and allocation-free: events go into a fixed
    ring buffer, and only flush() — message thread, or a probe's own thread —
    touches the file. This is the SceneCompare/SceneTrace discipline, arrived at
    after a scene-transform race was found on the audio thread where file I/O
    would have caused dropouts.
*/
#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <array>

class DataflowTrace
{
public:
    static DataflowTrace& instance()
    {
        static DataflowTrace t;
        return t;
    }

    /** Point tracing at a file. Called once, before anything is recorded. */
    void start (const juce::File& file)
    {
        target = file;
        target.getParentDirectory().createDirectory();
        target.replaceWithText ({});
        cursor.store (0, std::memory_order_relaxed);
        flushed = 0;
        on.store (true, std::memory_order_release);
    }

    bool enabled() const { return on.load (std::memory_order_relaxed); }

    /**
        Record one event. `dir` is "out" for the producing end and "in" for the
        consuming end — BOTH must be recorded for delivery to be provable, and
        the audit says ONE_SIDED rather than guessing when only one is.
    */
    void record (const char* scope, const char* dir, const char* channel,
                 int seq, const juce::String& payload, const juce::String& summary = {})
    {
        if (! enabled()) return;
        const auto slot = cursor.fetch_add (1, std::memory_order_relaxed);
        auto& e = ring[slot % kCapacity];
        e.ms      = juce::Time::getMillisecondCounter();
        e.scope   = scope;
        e.dir     = dir;
        e.channel = channel;
        e.seq     = seq;
        e.bytes   = payload.getNumBytesAsUTF8();
        e.hash    = hash (payload);
        copyInto (e.summary, summary);
        e.valid   = true;
    }

    /**
        Next sequence number for a channel. The SENDER calls this once per event
        and the receiver echoes the same value back — that pairing is what lets
        the audit prove delivery instead of inferring it from a name.
    */
    int nextSeq (const juce::String& channel)
    {
        const juce::SpinLock::ScopedLockType lock (seqLock);
        const int next = seqs.contains (channel) ? seqs[channel] : 0;
        seqs.set (channel, next + 1);
        return next;
    }

    /** MESSAGE THREAD (or a probe). Appends everything recorded since last time. */
    void flush()
    {
        if (! enabled()) return;
        const auto total = cursor.load (std::memory_order_acquire);
        if (total == flushed) return;

        // A ring that has wrapped has lost the oldest events. Say so in the file
        // rather than letting the audit read a gap as a dropped message.
        const auto first = total > kCapacity && flushed < total - kCapacity
                             ? total - kCapacity : flushed;
        juce::String out;
        if (first > flushed)
            out << "# WARNING: trace ring overflowed, " << (int) (first - flushed)
                << " event(s) lost — raise kCapacity or flush more often\n";

        for (auto i = first; i < total; ++i)
        {
            const auto& e = ring[i % kCapacity];
            if (! e.valid) continue;
            out << "{\"t\":" << (juce::int64) e.ms
                << ",\"side\":\"cpp\",\"scope\":\"" << e.scope
                << "\",\"dir\":\"" << e.dir
                << "\",\"ch\":\"" << e.channel
                << "\",\"seq\":" << e.seq
                << ",\"bytes\":" << (int) e.bytes
                << ",\"hash\":\"" << e.hash << "\"";
            if (e.summary[0] != 0)
                out << ",\"summary\":" << juce::JSON::toString (juce::String (e.summary));
            out << "}\n";
        }
        target.appendText (out);
        flushed = total;
    }

    /** The 32-bit FNV-1a the JS side uses, so hashes are comparable across the bridge. */
    static juce::String hash (const juce::String& s)
    {
        juce::uint32 h = 0x811c9dc5u;
        for (auto c : s)
        {
            h ^= (juce::uint32) (juce::juce_wchar) c;
            h *= 0x01000193u;
        }
        return juce::String::toHexString ((int) h).paddedLeft ('0', 8);
    }

    /** Steps as a bit string — the suite's leftmost-first convention. */
    static juce::String bits (const std::vector<bool>& v)
    {
        juce::String s;
        s.preallocateBytes (v.size() + 1);
        for (bool b : v) s << (b ? '1' : '0');
        return s;
    }

private:
    static constexpr size_t kCapacity = 4096;
    static constexpr size_t kSummary = 48;

    struct Event
    {
        juce::uint32 ms = 0;
        const char* scope = "";
        const char* dir = "";
        const char* channel = "";
        int seq = 0;
        size_t bytes = 0;
        juce::String hash;
        char summary[kSummary + 1] = {};
        bool valid = false;
    };

    static void copyInto (char (&dst)[kSummary + 1], const juce::String& s)
    {
        const auto utf8 = s.toRawUTF8();
        size_t i = 0;
        for (; i < kSummary && utf8[i] != 0; ++i) dst[i] = utf8[i];
        dst[i] = 0;
    }

    std::array<Event, kCapacity> ring {};
    std::atomic<size_t> cursor { 0 };
    std::atomic<bool> on { false };
    size_t flushed = 0;
    juce::File target;
    juce::HashMap<juce::String, int> seqs;
    juce::SpinLock seqLock;
};
