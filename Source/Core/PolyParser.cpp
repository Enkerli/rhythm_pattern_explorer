/*
  ==============================================================================

    PolyParser.cpp
    See PolyParser.h. Ported term-for-term from music-suite
    packages/upi/src/poly.js (splitLanes, parseOffset, parsePolyUPI) so the
    plugin and the webapp agree on poly notation — conformance-locked via
    Source/Tests/PolyConformanceVectors.h (Source/Tests/PolyConformanceTests.cpp).

  ==============================================================================
*/

#include "PolyParser.h"
#include "UPIParser.h"
#include <cmath>

namespace
{
    constexpr int kMaxMs = 50;
    // MAX_FRAC = 1/8, compared as num/den <= 1/8  <=>  num*8 <= den (den > 0).
}

//==============================================================================
int PolyParser::gcdInt(int a, int b)
{
    while (b != 0) { int t = b; b = a % b; a = t; }
    return a;
}

int PolyParser::lcmInt(int a, int b)
{
    if (a == 0 || b == 0) return 0;
    return (a / gcdInt(a, b)) * b;
}

//==============================================================================
juce::StringArray PolyParser::splitLanes(const juce::String& input)
{
    juce::StringArray rawLanes;
    juce::String cur;
    int depth = 0;
    const int len = input.length();
    int i = 0;

    while (i < len)
    {
        auto c = input[i];

        if (c == '(' || c == '[' || c == '{') depth++;
        else if (c == ')' || c == ']' || c == '}') depth--;

        if (c == '@' && depth == 0)
        {
            // Consume the whole offset token atomically: sign, digits, then
            // "ms" or "/digits" — so a fraction's slash never splits a lane.
            int j = i + 1;
            if (j < len && (input[j] == '+' || input[j] == '-')) j++;
            while (j < len && juce::CharacterFunctions::isDigit(input[j])) j++;
            if (j + 1 < len && input[j] == 'm' && input[j + 1] == 's') j += 2;
            else if (j < len && input[j] == '/' && j + 1 < len && juce::CharacterFunctions::isDigit(input[j + 1]))
            {
                j++;
                while (j < len && juce::CharacterFunctions::isDigit(input[j])) j++;
            }
            cur += input.substring(i, j);
            i = j;
            continue;
        }

        if (c == '/' && depth == 0)
        {
            rawLanes.add(cur);
            cur.clear();
            i++;
            continue;
        }

        cur += juce::String::charToString(c);
        i++;
    }
    rawLanes.add(cur);

    juce::StringArray lanes;
    for (auto& s : rawLanes)
    {
        auto trimmed = s.trim();
        if (trimmed.isNotEmpty()) lanes.add(trimmed);
    }
    return lanes;
}

//==============================================================================
PolyParser::OffsetParse PolyParser::parseOffset(const juce::String& laneSrc)
{
    int at = laneSrc.lastIndexOfChar('@');
    if (at < 0) return { laneSrc, PolyOffset{}, false, {} };

    juce::String token = laneSrc.substring(at);
    const int len = token.length();
    int pos = 1; // skip '@'
    int sign = 1;
    if (pos < len && (token[pos] == '+' || token[pos] == '-'))
    {
        if (token[pos] == '-') sign = -1;
        pos++;
    }

    int digitsStart = pos;
    while (pos < len && juce::CharacterFunctions::isDigit(token[pos])) pos++;
    bool haveDigits = pos > digitsStart;
    int n = haveDigits ? token.substring(digitsStart, pos).getIntValue() : 0;

    PolyOffset offset;
    bool matched = false;

    if (haveDigits && pos == len)
    {
        // Bare "@±N" == ms.
        offset.kind = PolyOffset::Ms;
        offset.ms = sign * n;
        matched = true;
    }
    else if (haveDigits && pos + 2 == len && token[pos] == 'm' && token[pos + 1] == 's')
    {
        offset.kind = PolyOffset::Ms;
        offset.ms = sign * n;
        matched = true;
    }
    else if (haveDigits && pos < len && token[pos] == '/')
    {
        int denStart = pos + 1;
        int p2 = denStart;
        while (p2 < len && juce::CharacterFunctions::isDigit(token[p2])) p2++;
        if (p2 > denStart && p2 == len)
        {
            offset.kind = PolyOffset::Frac;
            offset.num = sign * n;
            offset.den = token.substring(denStart, p2).getIntValue();
            matched = true;
        }
    }

    if (!matched)
        return { laneSrc, PolyOffset{}, true,
                 "bad offset \"" + token + "\" -- try @+12ms or @-1/32" };

    if (offset.kind == PolyOffset::Frac)
    {
        if (offset.den == 0)
            return { laneSrc, PolyOffset{}, true, "bad offset \"" + token + "\" -- zero denominator" };
        // num/den > 1/8  <=>  num*8 > den (both taken as magnitudes; den > 0 required above).
        long long numMag = std::abs((long long) offset.num);
        if (numMag * 8 > offset.den)
            return { laneSrc, PolyOffset{}, true,
                     "offset " + token + " beyond +-1/8 -- that's a different rhythm, not a feel" };
    }
    else // Ms
    {
        if (std::abs(offset.ms) > kMaxMs)
            return { laneSrc, PolyOffset{}, true,
                     "offset " + token + " beyond +-" + juce::String(kMaxMs) + "ms -- that's a different rhythm, not a feel" };
    }

    return { laneSrc.substring(0, at).trim(), offset, false, {} };
}

//==============================================================================
PolyParseResult PolyParser::parse(const juce::String& input,
                                   const std::function<void(int)>& beforeLaneParse)
{
    PolyParseResult result;
    auto lanesSrc = splitLanes(input.trim());
    if (lanesSrc.isEmpty())
    {
        result.error = "empty poly pattern";
        return result;
    }

    for (int i = 0; i < lanesSrc.size(); ++i)
    {
        juce::String src = lanesSrc[i];
        juce::String label = "lane" + juce::String(i + 1);

        // Optional "name=" label: ^([A-Za-z][\w-]*)\s*=\s*(.+)$
        int eq = src.indexOfChar('=');
        if (eq > 0)
        {
            juce::String candidate = src.substring(0, eq).trimEnd();
            bool validLabel = candidate.isNotEmpty() && juce::CharacterFunctions::isLetter(candidate[0]) != 0;
            for (int k = 1; validLabel && k < candidate.length(); ++k)
            {
                auto ch = candidate[k];
                if (! (juce::CharacterFunctions::isLetterOrDigit(ch) || ch == '_' || ch == '-'))
                    validLabel = false;
            }
            juce::String after = src.substring(eq + 1).trimStart();
            if (validLabel && after.isNotEmpty())
            {
                label = candidate;
                src = after;
            }
        }

        auto off = parseOffset(src);
        if (off.hasError)
        {
            result.ok = false;
            result.lanes.clear();
            result.error = label + ": " + off.error;
            return result;
        }
        src = off.rest;

        if (beforeLaneParse) beforeLaneParse(i);
        auto parsed = UPIParser::parse(src);
        if (! parsed.isValid())
        {
            result.ok = false;
            result.lanes.clear();
            auto msg = parsed.errorMessage.isNotEmpty()
                ? parsed.errorMessage
                : ("unparsed \"" + src + "\"");
            result.error = label + ": " + msg;
            return result;
        }

        PolyLane lane;
        lane.label = label;
        lane.steps = parsed.pattern;
        lane.offset = off.offset;
        lane.source = src;
        lane.hasProgressiveOffset = parsed.hasProgressiveOffset;
        lane.progressiveInitialOffset = parsed.initialOffset;
        lane.progressiveOffsetStep = parsed.progressiveOffset;
        result.lanes.push_back(lane);
    }

    int lcm = 1;
    for (auto& lane : result.lanes)
        lcm = lcmInt(lcm, juce::jmax(1, (int) lane.steps.size()));
    result.lcm = lcm;
    result.ok = true;
    return result;
}
