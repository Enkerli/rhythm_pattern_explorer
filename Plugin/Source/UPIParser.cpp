/*
  ==============================================================================

    Universal Pattern Input (UPI) Parser Implementation

  ==============================================================================
*/

#include "UPIParser.h"
#include "PatternEngine.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <functional>

//==============================================================================
// Progressive offset engine support - static members
bool UPIParser::hasProgressiveOffsetEngine = false;
PatternEngine* UPIParser::progressiveOffsetEngine = nullptr;

//==============================================================================
UPIParser::ParseResult UPIParser::parse(const juce::String& input)
{
    if (input.trim().isEmpty())
        return createError("Empty input");
    
    juce::String cleaned = cleanInput(input);
    
    // Check for accent patterns first (before other processing)
    ParseResult accentResult;
    juce::String basePattern = cleaned;
    
    if (hasAccentPattern(cleaned))
    {
        juce::String accentStr = extractAccentPattern(cleaned);
        basePattern = removeAccentPattern(cleaned);
        
        // Parse the accent pattern
        accentResult = parseAccentPattern(accentStr);
        if (!accentResult.isValid())
        {
            return createError("Invalid accent pattern: " + accentStr);
        }
        
    }
    
    // Check for combinations (+ or -) in the base pattern (without accents)
    if (basePattern.contains("+") || basePattern.contains("-"))
    {
        // Handle pattern combinations - support multiple additions
        auto parts = tokenize(basePattern, "+");
        if (parts.size() >= 2)
        {
            // Special handling for polygon combinations - calculate LCM first
            bool allPolygons = true;
            std::vector<int> polygonSizes;
            
            for (const auto& part : parts)
            {
                juce::String trimmed = part.trim();
                if (isPolygonPattern(trimmed))
                {
                    // Extract the polygon size for LCM calculation
                    std::regex polygonRegex(R"([Pp]\((\d+),(\d+)(?:,(\d+))?\))");
                    std::smatch match;
                    std::string inputStr = trimmed.toStdString();
                    
                    if (std::regex_search(inputStr, match, polygonRegex))
                    {
                        int sides = std::stoi(match[1].str());
                        int multiplier = match[3].matched ? std::stoi(match[3].str()) : 1;
                        int steps = sides * multiplier;
                        polygonSizes.push_back(steps);
                    }
                    else
                    {
                        allPolygons = false;
                        break;
                    }
                }
                else
                {
                    allPolygons = false;
                    break;
                }
            }
            
            if (allPolygons && polygonSizes.size() >= 2)
            {
                // Calculate LCM of all polygon sizes
                int targetLCM = polygonSizes[0];
                for (int i = 1; i < polygonSizes.size(); ++i)
                {
                    targetLCM = lcm(targetLCM, polygonSizes[i]);
                }
                
                
                // Parse each polygon projected onto the LCM space
                auto result = parsePolygonForCombination(parts[0].trim(), targetLCM);
                if (!result.isValid()) return result;
                
                for (int i = 1; i < parts.size(); ++i)
                {
                    auto nextResult = parsePolygonForCombination(parts[i].trim(), targetLCM);
                    if (!nextResult.isValid()) return nextResult;
                    
                    // Simple OR combination since they're already in the same space
                    for (int j = 0; j < targetLCM; ++j)
                    {
                        result.pattern[j] = result.pattern[j] || nextResult.pattern[j];
                    }
                }
                
                result.patternName = "Combined: " + cleaned;
                result.stepCount = targetLCM;
                return result;
            }
            else
            {
                // Regular combination for non-polygon patterns
                auto result = parsePattern(parts[0].trim());
                if (!result.isValid()) return result;
                
                for (int i = 1; i < parts.size(); ++i)
                {
                    auto nextResult = parsePattern(parts[i].trim());
                    if (!nextResult.isValid()) return nextResult;
                    
                    result.pattern = combinePatterns(result.pattern, nextResult.pattern, true);
                }
                
                result.patternName = "Combined: " + cleaned;
                result.stepCount = static_cast<int>(result.pattern.size());
                return result;
            }
        }
    }
    
    // Check for space-separated stringing
    if (cleaned.contains(" ") && !hasTransformationPrefix(cleaned))
    {
        auto parts = tokenize(cleaned, " ");
        if (parts.size() > 1)
        {
            std::vector<bool> stringed;
            for (const auto& part : parts)
            {
                auto result = parsePattern(part.trim());
                if (result.isValid())
                {
                    stringed.insert(stringed.end(), result.pattern.begin(), result.pattern.end());
                }
            }
            if (!stringed.empty())
            {
                auto result = createSuccess(stringed, "Stringed Pattern");
                result.type = ParseResult::Stringed;
                return result;
            }
        }
    }
    
    // Parse as single pattern (using basePattern without accents)
    auto result = parsePattern(basePattern);
    if (result.isValid())
    {
        result.type = ParseResult::Single;
        
        // Add accent pattern information if present
        if (accentResult.isValid())
        {
            result.hasAccentPattern = true;
            result.accentPattern = accentResult.pattern;
            result.accentPatternName = accentResult.patternName;
        }
    }
    return result;
}

UPIParser::ParseResult UPIParser::parsePattern(const juce::String& input)
{
    juce::String cleaned = cleanInput(input);
    
    
    // Pattern recognition processing
    
    // Handle transformations first
    if (cleaned.startsWith("~") || cleaned.startsWith("inv "))
    {
        juce::String basePattern = cleaned.startsWith("~") ? 
            cleaned.substring(1).trim() : 
            cleaned.substring(4).trim();
        
        auto baseResult = parsePattern(basePattern);
        if (baseResult.isValid())
        {
            auto inverted = invertPattern(baseResult.pattern);
            return createSuccess(inverted, "Inverted " + baseResult.patternName);
        }
    }
    
    if (cleaned.startsWith("rev "))
    {
        auto baseResult = parsePattern(cleaned.substring(4).trim());
        if (baseResult.isValid())
        {
            auto reversed = reversePattern(baseResult.pattern);
            return createSuccess(reversed, "Reversed " + baseResult.patternName);
        }
    }
    
    if (cleaned.startsWith("comp "))
    {
        auto baseResult = parsePattern(cleaned.substring(5).trim());
        if (baseResult.isValid())
        {
            auto complement = complementPattern(baseResult.pattern);
            return createSuccess(complement, "Complement " + baseResult.patternName);
        }
    }
    
    // Handle rotation: pattern@steps
    if (cleaned.contains("@"))
    {
        auto parts = tokenize(cleaned, "@");
        if (parts.size() == 2)
        {
            auto baseResult = parsePattern(parts[0].trim());
            int rotationSteps = parts[1].trim().getIntValue();
            if (baseResult.isValid())
            {
                // Negate rotation to make positive rotations go clockwise (webapp standard)
                auto rotated = rotatePattern(baseResult.pattern, -rotationSteps);
                return createSuccess(rotated, baseResult.patternName + "@" + juce::String(rotationSteps));
            }
        }
    }
    
    // Handle progressive transformations: patternB>target, patternW>target, etc.
    if (cleaned.contains(">"))
    {
        auto parts = tokenize(cleaned, ">");
        if (parts.size() == 2)
        {
            juce::String basePattern = parts[0].trim();
            int targetOnsets = parts[1].trim().getIntValue();
            
            
            // Progressive pattern processing
            
            // Extract transformer type (B, W, E, D) and remove it from base pattern
            char transformerType = 'b'; // Default to Barlow
            if (basePattern.length() > 1)
            {
                char lastChar = basePattern[basePattern.length() - 1];
                if (lastChar == 'b' || lastChar == 'B') transformerType = 'b';
                else if (lastChar == 'w' || lastChar == 'W') transformerType = 'w';
                else if (lastChar == 'e' || lastChar == 'E') transformerType = 'e';
                else if (lastChar == 'd' || lastChar == 'D') transformerType = 'd';
                
                // Remove transformer letter from base pattern IMMEDIATELY
                if (lastChar == 'b' || lastChar == 'B' || lastChar == 'w' || lastChar == 'W' ||
                    lastChar == 'e' || lastChar == 'E' || lastChar == 'd' || lastChar == 'D')
                {
                    basePattern = basePattern.dropLastCharacters(1);
                }
            }
            
            
            // Progressive transformation setup
            
            auto baseResult = parsePattern(basePattern);
            if (baseResult.isValid())
            {
                
                // Apply progressive transformation with target onset count
                auto transformed = applyProgressiveTransformation(baseResult.pattern, transformerType, targetOnsets);
                
                auto result = createSuccess(transformed, "Progressive: " + cleaned);
                
                // Mark as progressive transformation so the UI knows to track progression steps
                result.hasProgressiveOffset = true;
                result.initialOffset = 0;  // Progressive transformations don't use offset
                result.progressiveOffset = 1;  // Each trigger advances by 1 step
                
                // Store the pattern key for step tracking
                result.progressivePatternKey = patternToBinary(baseResult.pattern) + juce::String(transformerType) + juce::String(targetOnsets);
                
                return result;
            }
        }
    }
    
    // Parse core pattern types
    if (isEuclideanPattern(cleaned))
    {
        // E(onsets,steps) or E(onsets,steps,offset) or E(onsets,steps)@initial#progressive - case insensitive
        std::regex euclideanRegex(R"([Ee]\((\d+),(\d+)(?:,(-?\d+))?\)(?:@(-?\d+)#(-?\d+))?)");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, euclideanRegex))
        {
            int onsets = std::stoi(match[1].str());
            int steps = std::stoi(match[2].str());
            int offset = match[3].matched ? std::stoi(match[3].str()) : 0;
            
            // Check for progressive offset syntax @initial>progressive
            bool hasProgressiveOffset = match[4].matched && match[5].matched;
            int initialOffset = hasProgressiveOffset ? std::stoi(match[4].str()) : offset;
            int progressiveOffset = hasProgressiveOffset ? std::stoi(match[5].str()) : 0;
            
            // Use current progressive offset if this is a progressive pattern and engine is tracking state
            int effectiveOffset = initialOffset;
            if (hasProgressiveOffset && hasProgressiveOffsetEngine)
            {
                effectiveOffset = getCurrentProgressiveOffset();
            }
            else
            {
                    // Progressive offset calculated
            }
            
            auto pattern = parseEuclidean(onsets, steps, effectiveOffset);
            auto result = createSuccess(pattern, "E(" + juce::String(onsets) + "," + juce::String(steps) + 
                               (initialOffset != 0 || hasProgressiveOffset ? 
                                (hasProgressiveOffset ? "@" + juce::String(initialOffset) + "#" + juce::String(progressiveOffset) :
                                 "," + juce::String(offset)) : "") + ")");
            
            // Set progressive offset information
            result.hasProgressiveOffset = hasProgressiveOffset;
            result.initialOffset = initialOffset;
            result.progressiveOffset = progressiveOffset;
            
            return result;
        }
    }
    
    if (isPolygonPattern(cleaned))
    {
        // P(sides,offset) or P(sides,offset,steps) - case insensitive
        std::regex polygonRegex(R"([Pp]\((\d+),(\d+)(?:,(\d+))?\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, polygonRegex))
        {
            int sides = std::stoi(match[1].str());
            int offset = std::stoi(match[2].str());
            int multiplier = match[3].matched ? std::stoi(match[3].str()) : 1;
            int steps = sides * multiplier;
            
            auto pattern = parsePolygon(sides, offset, steps);
            return createSuccess(pattern, "P(" + juce::String(sides) + "," + juce::String(offset) + 
                               (match[3].matched ? "," + juce::String(multiplier) : "") + ")");
        }
    }
    
    if (isBinaryPattern(cleaned))
    {
        // Remove 'b' prefix if present
        juce::String binaryStr = cleaned.startsWith("b") ? cleaned.substring(1) : cleaned;
        
        // Check for explicit step count: pattern:steps
        if (binaryStr.contains(":"))
        {
            auto parts = tokenize(binaryStr, ":");
            if (parts.size() == 2)
            {
                auto pattern = parseBinary(parts[0].trim(), parts[1].trim().getIntValue());
                return createSuccess(pattern, "Binary: " + parts[0].trim());
            }
        }
        else
        {
            auto pattern = parseBinary(binaryStr);
            return createSuccess(pattern, "Binary: " + binaryStr);
        }
    }
    
    // Check for numeric patterns using generic handler
    static const NumericPatternInfo numericPatterns[] = {
        {"0x", NumericBase::Hexadecimal, "0123456789ABCDEFabcdef"},
        {"d", NumericBase::Decimal, "0123456789"},
        {"o", NumericBase::Octal, "01234567"}
    };
    
    for (const auto& info : numericPatterns)
    {
        if (isNumericPattern(cleaned, info))
        {
            return parseNumericPattern(cleaned, info, 0); // 0 = auto-calculate steps
        }
    }
    
    if (isArrayPattern(cleaned))
    {
        // [0,3,6]:8 format
        auto pattern = parseArray(cleaned);
        return createSuccess(pattern, "Array: " + cleaned);
    }
    
    if (isMorsePattern(cleaned))
    {
        // M:SOS or direct morse like -.-- 
        juce::String morseCode = cleaned;
        if (cleaned.startsWith("m:"))
        {
            morseCode = cleaned.substring(2).trim();
        }
        auto pattern = parseMorse(morseCode);
        return createSuccess(pattern, "Morse: " + morseCode);
    }
    
    if (isRandomPattern(cleaned))
    {
        // R(onsets,steps) or R(r,steps) - case insensitive
        std::regex randomRegex(R"([Rr]\(([r\d]+),(\d+)\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, randomRegex))
        {
            juce::String onsetsStr = juce::String(match[1].str());
            int steps = std::stoi(match[2].str());
            
            if (onsetsStr == "r")
            {
                // Bell curve random
                int onsets = steps / 3 + (rand() % (steps / 2)); // Rough bell curve
                auto pattern = parseRandom(onsets, steps);
                return createSuccess(pattern, "R(r," + juce::String(steps) + ")");
            }
            else
            {
                int onsets = onsetsStr.getIntValue();
                auto pattern = parseRandom(onsets, steps);
                return createSuccess(pattern, "R(" + juce::String(onsets) + "," + juce::String(steps) + ")");
            }
        }
    }
    
    if (isBarlowPattern(cleaned))
    {
        // B(onsets,steps) - Generate Barlow indispensability pattern
        std::regex barlowRegex(R"([Bb]\((\d+),(\d+)\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, barlowRegex))
        {
            int onsets = std::stoi(match[1].str());
            int steps = std::stoi(match[2].str());
            
            // Create base pattern with single onset and transform to target using Barlow
            std::vector<bool> basePattern(steps, false);
            basePattern[0] = true; // Start with downbeat
            
            auto pattern = generateBarlowTransformation(basePattern, onsets, false);
            return createSuccess(pattern, "B(" + juce::String(onsets) + "," + juce::String(steps) + ")");
        }
    }
    
    if (isWolrabPattern(cleaned))
    {
        // W(onsets,steps) - Generate Wolrab (anti-Barlow) pattern 
        std::regex wolrabRegex(R"([Ww]\((\d+),(\d+)\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, wolrabRegex))
        {
            int onsets = std::stoi(match[1].str());
            int steps = std::stoi(match[2].str());
            
            // Create base pattern with single onset and transform to target using Wolrab
            std::vector<bool> basePattern(steps, false);
            basePattern[0] = true; // Start with downbeat
            
            auto pattern = generateBarlowTransformation(basePattern, onsets, true); // true = Wolrab mode
            return createSuccess(pattern, "W(" + juce::String(onsets) + "," + juce::String(steps) + ")");
        }
    }
    
    if (isDilcuePattern(cleaned))
    {
        // D(onsets,steps) - Generate Dilcue (anti-Euclidean) pattern
        std::regex dilcueRegex(R"([Dd]\((\d+),(\d+)\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, dilcueRegex))
        {
            int onsets = std::stoi(match[1].str());
            int steps = std::stoi(match[2].str());
            
            auto pattern = generateEuclideanTransformation(std::vector<bool>(steps, false), onsets, true); // true = anti-Euclidean
            return createSuccess(pattern, "D(" + juce::String(onsets) + "," + juce::String(steps) + ")");
        }
    }
    
    // Handle shorthand polygon names
    if (cleaned == "tri") return parsePattern("P(3,0)");
    if (cleaned == "pent") return parsePattern("P(5,0)");
    if (cleaned == "hex") return parsePattern("P(6,0)");
    if (cleaned == "hept") return parsePattern("P(7,0)");
    if (cleaned == "oct") return parsePattern("P(8,0)");
    
    // Handle traditional rhythm names
    if (cleaned == "tresillo") return parsePattern("E(3,8)");
    if (cleaned == "cinquillo") return parsePattern("E(5,8)");
    
    // Try decimal parsing if it's just a number
    if (cleaned.containsOnly("0123456789"))
    {
        int decimal = cleaned.getIntValue();
        // Calculate minimum steps needed to represent this decimal
        int minSteps = decimal > 0 ? static_cast<int>(std::ceil(std::log2(decimal + 1))) : 1;
        int targetSteps = std::max(minSteps, 8); // At least 8 steps, or enough to represent the number
        auto pattern = parseDecimal(decimal, targetSteps);
        return createSuccess(pattern, "Decimal: " + cleaned);
    }
    
    return createError("Unrecognized pattern format: " + cleaned);
}

//==============================================================================
// Core pattern generators

std::vector<bool> UPIParser::parseEuclidean(int onsets, int steps, int offset)
{
    auto pattern = bjorklundAlgorithm(onsets, steps);
    if (offset != 0)
    {
        // Negate offset to make positive offsets go clockwise (webapp standard)
        pattern = rotatePattern(pattern, -offset);
    }
    return pattern;
}

std::vector<bool> UPIParser::parsePolygon(int sides, int offset, int totalSteps)
{
    if (totalSteps == 0) 
        totalSteps = sides;
    
    std::vector<bool> pattern(totalSteps, false);
    
    // Place polygon vertices with proper spacing
    for (int i = 0; i < sides; ++i)
    {
        // Calculate position with floating point for accuracy, then round
        double exactPos = (double)(i * totalSteps) / sides;
        int pos = ((int)std::round(exactPos) + offset) % totalSteps;
        if (pos < 0) pos += totalSteps; // Handle negative modulo
        
        if (pos >= 0 && pos < totalSteps)
            pattern[pos] = true;
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseBinary(const juce::String& binaryStr, int stepCount)
{
    std::vector<bool> pattern;
    int actualSteps = stepCount > 0 ? stepCount : binaryStr.length();
    pattern.reserve(actualSteps);
    
    for (int i = 0; i < actualSteps; ++i)
    {
        if (i < binaryStr.length())
        {
            pattern.push_back(binaryStr[i] == '1');
        }
        else
        {
            pattern.push_back(false);
        }
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseArray(const juce::String& arrayStr, int stepCount)
{
    // Parse [0,3,6]:8 format
    juce::String processed = arrayStr;
    int explicitSteps = stepCount; // Use provided stepCount as default
    
    // Look for explicit step count after ]:
    if (processed.contains("]:"))
    {
        int colonPos = processed.indexOf("]:");
        if (colonPos >= 0)
        {
            juce::String stepCountStr = processed.substring(colonPos + 2).trim();
            explicitSteps = stepCountStr.getIntValue();
            processed = processed.substring(0, colonPos + 1); // Keep the closing bracket
        }
    }
    
    // Remove brackets
    processed = processed.removeCharacters("[]");
    
    auto positions = tokenize(processed, ",");
    std::vector<int> onsetPositions;
    
    for (const auto& pos : positions)
    {
        onsetPositions.push_back(pos.trim().getIntValue());
    }
    
    if (onsetPositions.empty())
        return {};
    
    // If no explicit steps given, use max position + 1 or minimum of 8
    if (explicitSteps <= 0)
    {
        int maxPos = *std::max_element(onsetPositions.begin(), onsetPositions.end());
        explicitSteps = std::max(maxPos + 1, 8);
    }
    
    std::vector<bool> pattern(explicitSteps, false);
    for (int pos : onsetPositions)
    {
        if (pos >= 0 && pos < explicitSteps)
            pattern[pos] = true;
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseRandom(int onsets, int steps)
{
    std::vector<bool> pattern(steps, false);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Randomly place onsets
    std::vector<int> positions;
    for (int i = 0; i < steps; ++i)
        positions.push_back(i);
    
    std::shuffle(positions.begin(), positions.end(), gen);
    
    for (int i = 0; i < std::min(onsets, steps); ++i)
    {
        pattern[positions[i]] = true;
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseDecimal(int decimal, int stepCount)
{
    std::vector<bool> pattern;
    pattern.reserve(stepCount);
    
    // Convert decimal to binary, LEFT-TO-RIGHT: leftmost bit = LSB (least significant bit first)
    // This gives strict left-to-right hex notation where 1000 = 0x1, 0100 = 0x2, etc.
    for (int i = 0; i < stepCount; ++i)
    {
        pattern.push_back((decimal & (1 << i)) != 0);
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseMorse(const juce::String& morseStr)
{
    std::vector<bool> pattern;
    
    // Simple morse code mapping
    juce::String processed = morseStr.toLowerCase();
    
    // Handle common morse patterns
    if (processed == "sos")
        processed = "...---...";
    else if (processed == "cq")
        processed = "-.-.--.-";
    
    // Convert morse to pattern (. = short onset, - = long onset, space = rest)
    for (int i = 0; i < processed.length(); ++i)
    {
        char c = processed[i];
        if (c == '.')
        {
            pattern.push_back(true);   // Short onset
        }
        else if (c == '-')
        {
            pattern.push_back(true);   // Long onset 
            pattern.push_back(false);  // Add extra rest for length
        }
        else if (c == ' ')
        {
            pattern.push_back(false);  // Rest
        }
    }
    
    // Ensure minimum length
    if (pattern.size() < 4)
    {
        pattern.resize(8, false);
    }
    
    return pattern;
}

//==============================================================================
// Pattern transformations

std::vector<bool> UPIParser::invertPattern(const std::vector<bool>& pattern)
{
    std::vector<bool> inverted;
    inverted.reserve(pattern.size());
    
    for (bool step : pattern)
    {
        inverted.push_back(!step);
    }
    
    return inverted;
}

std::vector<bool> UPIParser::reversePattern(const std::vector<bool>& pattern)
{
    std::vector<bool> reversed(pattern.rbegin(), pattern.rend());
    return reversed;
}

std::vector<bool> UPIParser::complementPattern(const std::vector<bool>& pattern)
{
    return invertPattern(pattern); // Same as invert for now
}

std::vector<bool> UPIParser::rotatePattern(const std::vector<bool>& pattern, int steps)
{
    if (pattern.empty()) return pattern;
    
    int size = static_cast<int>(pattern.size());
    steps = ((steps % size) + size) % size; // Handle negative rotation
    
    std::vector<bool> rotated;
    rotated.reserve(size);
    
    for (int i = 0; i < size; ++i)
    {
        rotated.push_back(pattern[(i + steps) % size]);
    }
    
    return rotated;
}

std::vector<bool> UPIParser::combinePatterns(const std::vector<bool>& pattern1, 
                                           const std::vector<bool>& pattern2, 
                                           bool isAddition)
{
    if (pattern1.empty()) return pattern2;
    if (pattern2.empty()) return pattern1;
    
    // Calculate LCM size for proper pattern expansion
    int size1 = static_cast<int>(pattern1.size());
    int size2 = static_cast<int>(pattern2.size());
    int lcmSize = lcm(size1, size2);
    
    auto expanded1 = expandToLCM(pattern1, lcmSize);
    auto expanded2 = expandToLCM(pattern2, lcmSize);
    
    std::vector<bool> combined;
    combined.reserve(lcmSize);
    
    for (int i = 0; i < lcmSize; ++i)
    {
        if (isAddition)
        {
            combined.push_back(expanded1[i] || expanded2[i]); // Logical OR
        }
        else
        {
            combined.push_back(expanded1[i] && !expanded2[i]); // Subtraction
        }
    }
    
    return combined;
}

//==============================================================================
// Utility functions

std::vector<bool> UPIParser::bjorklundAlgorithm(int beats, int steps)
{
    if (beats > steps) beats = steps;
    if (beats <= 0) return std::vector<bool>(steps, false);
    if (beats == steps) return std::vector<bool>(steps, true);
    
    std::vector<bool> pattern;
    std::vector<int> counts;
    std::vector<int> remainders;
    
    int divisor = steps - beats;
    remainders.push_back(beats);
    int level = 0;
    
    do {
        counts.push_back(divisor / remainders[level]);
        remainders.push_back(divisor % remainders[level]);
        divisor = remainders[level];
        level++;
    } while (remainders[level] > 1);
    
    counts.push_back(divisor);
    
    std::function<void(int)> build = [&](int level) {
        if (level == -1) {
            pattern.push_back(false);
        } else if (level == -2) {
            pattern.push_back(true);
        } else {
            for (int i = 0; i < counts[level]; i++) {
                build(level - 1);
            }
            if (remainders[level] != 0) {
                build(level - 2);
            }
        }
    };
    
    build(level);
    
    // Ensure we have exactly 'steps' elements
    while (pattern.size() < static_cast<size_t>(steps)) {
        pattern.push_back(false);
    }
    pattern.resize(steps);
    
    // Rotate so first beat is at position 0
    auto firstBeatIndex = std::find(pattern.begin(), pattern.end(), true);
    if (firstBeatIndex != pattern.begin() && firstBeatIndex != pattern.end()) {
        size_t index = std::distance(pattern.begin(), firstBeatIndex);
        std::vector<bool> rotated;
        rotated.insert(rotated.end(), pattern.begin() + index, pattern.end());
        rotated.insert(rotated.end(), pattern.begin(), pattern.begin() + index);
        pattern = rotated;
    }
    
    return pattern;
}

int UPIParser::gcd(int a, int b)
{
    return b == 0 ? a : gcd(b, a % b);
}

int UPIParser::lcm(int a, int b)
{
    return (a * b) / gcd(a, b);
}

std::vector<bool> UPIParser::expandToLCM(const std::vector<bool>& pattern, int targetSize)
{
    if (pattern.empty()) return std::vector<bool>(targetSize, false);
    
    std::vector<bool> expanded;
    expanded.reserve(targetSize);
    
    int originalSize = static_cast<int>(pattern.size());
    for (int i = 0; i < targetSize; ++i)
    {
        expanded.push_back(pattern[i % originalSize]);
    }
    
    return expanded;
}

bool UPIParser::isValidPattern(const std::vector<bool>& pattern)
{
    return !pattern.empty();
}

int UPIParser::countOnsets(const std::vector<bool>& pattern)
{
    return static_cast<int>(std::count(pattern.begin(), pattern.end(), true));
}

juce::String UPIParser::patternToBinary(const std::vector<bool>& pattern)
{
    juce::String binary;
    for (bool step : pattern)
    {
        binary += step ? "1" : "0";
    }
    return binary;
}

//==============================================================================
// Table-driven pattern recognition

// Custom validators for complex patterns
static bool validateBinaryPattern(const juce::String& input)
{
    juce::String processed = input.startsWith("b") ? input.substring(1) : input;
    if (processed.contains(":"))
        processed = processed.upToFirstOccurrenceOf(":", false, false);
    return processed.containsOnly("01");
}

static bool validateMorsePattern(const juce::String& input)
{
    return input.startsWith("m:") || input.containsOnly(".-");
}

const std::map<UPIParser::PatternType, UPIParser::PatternRecognitionRule>& UPIParser::getPatternRules()
{
    static const std::map<PatternType, PatternRecognitionRule> rules = {
        {PatternType::Euclidean, {"E(", ")", "e("}},
        {PatternType::Polygon,   {"P(", ")", "p("}},
        {PatternType::Random,    {"R(", ")", "r("}},
        {PatternType::Barlow,    {"B(", ")", "b("}},
        {PatternType::Wolrab,    {"W(", ")", "w("}},
        {PatternType::Dilcue,    {"D(", ")", "d("}},
        {PatternType::Array,     {"[", "]"}},
        {PatternType::Binary,    {"", "", "", validateBinaryPattern}},
        {PatternType::Hex,       {"0x", ""}},
        {PatternType::Decimal,   {"d", ""}},
        {PatternType::Octal,     {"o", ""}},
        {PatternType::Morse,     {"", "", "", validateMorsePattern}}
    };
    return rules;
}

bool UPIParser::isPatternType(const juce::String& input, PatternType type)
{
    const auto& rules = getPatternRules();
    auto it = rules.find(type);
    if (it == rules.end()) return false;
    
    const auto& rule = it->second;
    
    // Use custom validator if provided
    if (rule.customValidator) {
        return rule.customValidator(input);
    }
    
    // Check primary prefix and suffix
    bool matchesStart = rule.startPrefix.isEmpty() || input.startsWith(rule.startPrefix);
    if (!matchesStart && !rule.alternateStart.isEmpty()) {
        matchesStart = input.startsWith(rule.alternateStart);
    }
    
    bool matchesEnd = rule.endSuffix.isEmpty() || input.endsWith(rule.endSuffix);
    
    // Special handling for Euclidean patterns (supports @# suffixes)
    if (type == PatternType::Euclidean && matchesStart) {
        return input.endsWith(")") || input.contains(")@") || input.contains(")#");
    }
    
    return matchesStart && matchesEnd;
}

//==============================================================================
// Legacy pattern recognition helpers (for backward compatibility)

bool UPIParser::isEuclideanPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Euclidean);
}

bool UPIParser::isPolygonPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Polygon);
}

bool UPIParser::isBinaryPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Binary);
}

bool UPIParser::isArrayPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Array);
}

bool UPIParser::isRandomPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Random);
}

bool UPIParser::isBarlowPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Barlow);
}

bool UPIParser::isWolrabPattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Wolrab);
}

bool UPIParser::isDilcuePattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Dilcue);
}

bool UPIParser::isHexPattern(const juce::String& input)
{
    // For hex patterns, we still need the detailed validation
    static const NumericPatternInfo hexInfo = {"0x", NumericBase::Hexadecimal, "0123456789ABCDEFabcdef"};
    return isNumericPattern(input, hexInfo);
}

bool UPIParser::isDecimalPattern(const juce::String& input)
{
    // For decimal patterns, we still need the detailed validation
    static const NumericPatternInfo decimalInfo = {"d", NumericBase::Decimal, "0123456789"};
    return isNumericPattern(input, decimalInfo);
}

bool UPIParser::isOctalPattern(const juce::String& input)
{
    // For octal patterns, we still need the detailed validation
    static const NumericPatternInfo octalInfo = {"o", NumericBase::Octal, "01234567"};
    return isNumericPattern(input, octalInfo);
}

bool UPIParser::isMorsePattern(const juce::String& input)
{
    return isPatternType(input, PatternType::Morse);
}

//==============================================================================
// String processing utilities

juce::String UPIParser::cleanInput(const juce::String& input)
{
    return input.trim().toLowerCase();
}

juce::StringArray UPIParser::tokenize(const juce::String& input, const juce::String& delimiter)
{
    juce::StringArray tokens;
    tokens.addTokens(input, delimiter, "");
    return tokens;
}

bool UPIParser::hasTransformationPrefix(const juce::String& input)
{
    return input.startsWith("~") || input.startsWith("inv ") || 
           input.startsWith("rev ") || input.startsWith("comp ");
}

//==============================================================================
// Accent pattern utilities

/**
 * Checks if input contains accent pattern notation.
 * 
 * Accent patterns use curly bracket notation: {accent}pattern or pattern{accent}
 * Examples: {100}E(3,8), {10010}E(5,8), {E(2,5)}B(3,13)
 * 
 * @param input The UPI string to check
 * @return true if contains { and } characters
 */
bool UPIParser::hasAccentPattern(const juce::String& input)
{
    return input.contains("{") && input.contains("}");
}

/**
 * Extracts the accent pattern from curly brackets.
 * 
 * From "{100}E(3,8)" extracts "100"
 * From "{E(2,5)}B(3,13)" extracts "E(2,5)"
 * 
 * @param input UPI string containing accent pattern
 * @return Content between { and } characters, or empty string if not found
 */
juce::String UPIParser::extractAccentPattern(const juce::String& input)
{
    int start = input.indexOfChar('{');
    int end = input.indexOfChar('}');
    
    if (start >= 0 && end > start)
    {
        return input.substring(start + 1, end);
    }
    
    return "";
}

juce::String UPIParser::removeAccentPattern(const juce::String& input)
{
    int start = input.indexOfChar('{');
    int end = input.indexOfChar('}');
    
    if (start >= 0 && end > start)
    {
        // Remove the entire {accent} portion
        return input.substring(0, start) + input.substring(end + 1);
    }
    
    return input;
}

/**
 * Parses accent pattern string into binary pattern.
 * 
 * Accent patterns support the same notation as rhythm patterns:
 * - Binary: "100", "10010", "101"
 * - Algorithmic: "E(2,5)", "P(3,0)", "B(3,8)"
 * - Numeric: "0x5:8", "42:8", "[0,2]:8"
 * 
 * Design Decision: Accent patterns are independent of rhythm patterns
 * and cycle when different lengths (polyrhythmic accents).
 * 
 * @param accentStr The accent pattern string to parse
 * @return ParseResult with binary accent pattern
 */
UPIParser::ParseResult UPIParser::parseAccentPattern(const juce::String& accentStr)
{
    if (accentStr.isEmpty())
        return createError("Empty accent pattern");
    
    // Accent patterns can use any of the same notations as rhythm patterns
    // Binary patterns like "100", "10010", etc.
    if (accentStr.containsOnly("01"))
    {
        std::vector<bool> pattern;
        for (int i = 0; i < accentStr.length(); ++i)
        {
            pattern.push_back(accentStr[i] == '1');
        }
        
        auto result = createSuccess(pattern, "accent_" + accentStr);
        result.patternName = "accent_" + accentStr;
        return result;
    }
    
    // For other pattern types, recursively parse using the main pattern parser
    // This allows accent patterns like {E(3,8)}, {P(5,0)}, etc.
    auto result = parsePattern(accentStr);
    if (result.isValid())
    {
        result.patternName = "accent_" + accentStr;
        return result;
    }
    
    return createError("Invalid accent pattern format: " + accentStr);
}

//==============================================================================
// Generic numeric pattern handler

bool UPIParser::isNumericPattern(const juce::String& input, const NumericPatternInfo& info)
{
    if (!input.startsWith(info.prefix))
        return false;
    
    juce::String content = input.substring(info.prefix.length());
    
    // Handle step count specification (e.g., "0xFF:16")
    if (content.contains(":"))
        content = content.upToFirstOccurrenceOf(":", false, false);
    
    return content.containsOnly(info.validChars);
}

UPIParser::ParseResult UPIParser::parseNumericPattern(const juce::String& input, const NumericPatternInfo& info, int stepCount)
{
    juce::String content = input.substring(info.prefix.length());
    int explicitSteps = stepCount;
    
    // Extract step count if specified
    if (content.contains(":"))
    {
        juce::String stepStr = content.fromFirstOccurrenceOf(":", false, false);
        explicitSteps = stepStr.getIntValue();
        content = content.upToFirstOccurrenceOf(":", false, false);
    }
    
    // Convert to decimal based on base
    int decimal = 0;
    switch (info.base)
    {
        case NumericBase::Binary:
            for (int i = 0; i < content.length(); ++i)
                if (content[i] == '1')
                    decimal |= (1 << (content.length() - 1 - i));
            break;
            
        case NumericBase::Octal:
            for (int i = 0; i < content.length(); ++i)
            {
                decimal *= 8;
                decimal += (content[i] - '0');
            }
            break;
            
        case NumericBase::Decimal:
            decimal = content.getIntValue();
            break;
            
        case NumericBase::Hexadecimal:
            decimal = content.getHexValue32();
            break;
    }
    
    // Calculate appropriate step count
    if (explicitSteps <= 0)
    {
        if (info.base == NumericBase::Binary)
            explicitSteps = content.length();
        else
            explicitSteps = decimal > 0 ? static_cast<int>(std::ceil(std::log2(decimal + 1))) : 1;
        explicitSteps = std::max(explicitSteps, 8);
    }
    
    auto pattern = parseDecimal(decimal, explicitSteps);
    return createSuccess(pattern, info.prefix + content + ":" + juce::String(explicitSteps));
}

//==============================================================================
// Polygon combination helper

UPIParser::ParseResult UPIParser::parsePolygonForCombination(const juce::String& polygonStr, int targetSteps)
{
    // Parse P(sides,offset) or P(sides,offset,steps) but project onto targetSteps
    std::regex polygonRegex(R"([Pp]\((\d+),(\d+)(?:,(\d+))?\))");
    std::smatch match;
    std::string inputStr = polygonStr.toStdString();
    
    if (std::regex_search(inputStr, match, polygonRegex))
    {
        int sides = std::stoi(match[1].str());
        int offset = std::stoi(match[2].str());
        // Ignore the optional third parameter - we're projecting onto targetSteps
        
        auto pattern = parsePolygon(sides, offset, targetSteps);
        return createSuccess(pattern, "P(" + juce::String(sides) + "," + juce::String(offset) + "->" + juce::String(targetSteps) + ")");
    }
    
    return createError("Invalid polygon pattern: " + polygonStr);
}

// Progressive transformation state - stores current pattern for each key
static std::map<juce::String, std::vector<bool>> progressivePatterns;
static std::map<juce::String, int> progressiveAccessCount; // Track access frequency for cleanup
static std::map<juce::String, int> progressiveStepCount; // Track current step number for each pattern
static const int MAX_PROGRESSIVE_STATES = 100; // Limit to prevent unbounded growth

// Cleanup function to prevent unbounded growth
static void cleanupProgressiveStates()
{
    if (progressivePatterns.size() <= MAX_PROGRESSIVE_STATES) return;
    
    
    // Find least frequently used patterns
    std::vector<std::pair<int, juce::String>> accessCounts;
    for (const auto& pair : progressiveAccessCount)
    {
        accessCounts.push_back({pair.second, pair.first});
    }
    
    // Sort by access count (ascending - least used first)
    std::sort(accessCounts.begin(), accessCounts.end());
    
    // Remove the least used patterns (keep only the most recent half)
    int toRemove = progressivePatterns.size() - (MAX_PROGRESSIVE_STATES / 2);
    for (int i = 0; i < toRemove && i < accessCounts.size(); ++i)
    {
        const juce::String& keyToRemove = accessCounts[i].second;
        progressivePatterns.erase(keyToRemove);
        progressiveAccessCount.erase(keyToRemove);
        progressiveStepCount.erase(keyToRemove);
    }
    
}

/**
 * Applies progressive transformation to a base pattern, stepping through onset counts.
 * 
 * This function implements the core progressive transformation logic for the ">" syntax.
 * It maintains state between calls to create smooth transitions from base pattern to target.
 * 
 * Design Decisions:
 * - Uses pattern + transformer + target as unique key for state management
 * - Loops back to base pattern when target is reached (continuous cycling)
 * - Increments step counter AFTER pattern generation for UI consistency
 * - Implements LRU cleanup to prevent unbounded memory growth
 * 
 * @param basePattern   The starting pattern for transformation
 * @param transformerType  'e'=Euclidean, 'b'=Barlow, 'w'=Wolrab, 'd'=Dilcue
 * @param targetOnsets  Target number of onsets to progress toward
 * @return Next pattern in the progressive sequence
 */
std::vector<bool> UPIParser::applyProgressiveTransformation(const std::vector<bool>& basePattern, char transformerType, int targetOnsets)
{
    // Create a unique key for this progressive pattern
    // Key format: "basePattern + transformerType + targetOnsets"
    // Example: "10000000e8" for E(1,8)E>8
    juce::String patternKey = patternToBinary(basePattern) + juce::String(transformerType) + juce::String(targetOnsets);
    
    
    // Check if cleanup is needed before proceeding
    // Prevents unbounded growth of progressive state maps
    cleanupProgressiveStates();
    
    // Track access for LRU cleanup
    // More frequently accessed patterns are kept longer
    progressiveAccessCount[patternKey]++;
    
    // Initialize or get the current pattern state
    std::vector<bool> currentPattern;
    
    if (progressivePatterns.find(patternKey) == progressivePatterns.end())
    {
        // First time - return base pattern directly without transformation
        // This ensures the user sees the starting pattern on initial trigger
        progressivePatterns[patternKey] = basePattern;
        progressiveStepCount[patternKey] = 1; // UI shows step 1 for base pattern
        return basePattern;
    }
    else
    {
        // Get the current state from previous transformation
        // This continues the progressive sequence from where it left off
        currentPattern = progressivePatterns[patternKey];
    }
    
    int currentOnsets = countOnsets(currentPattern);
    
    // Check if we've reached the target - if so, loop back to base pattern
    // This creates continuous cycling behavior for live performance
    if (currentOnsets == targetOnsets)
    {
        progressivePatterns[patternKey] = basePattern;
        progressiveStepCount[patternKey] = 1; // Reset UI to step 1
        return basePattern;
    }
    
    // Calculate the next step in the progression
    // Direction: +1 for concentration (adding onsets), -1 for dilution (removing onsets)
    int direction = (targetOnsets > currentOnsets) ? 1 : -1;
    int nextOnsets = currentOnsets + direction;
    
    
    // Clamp to valid range to prevent overshooting the target
    // This ensures we don't go beyond the target onset count
    if (direction > 0)
    {
        nextOnsets = std::min(nextOnsets, targetOnsets);
    }
    else
    {
        nextOnsets = std::max(nextOnsets, targetOnsets);
    }
    
    
    // Transform from current pattern to next step
    // Each transformer type uses different algorithms for onset placement/removal
    std::vector<bool> result;
    
    switch (transformerType)
    {
        case 'b': // Barlow transformation
        {
            // Uses indispensability theory for musically intelligent onset placement
            // Higher indispensability = more likely to have onsets
            result = generateBarlowTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'w': // Wolrab (anti-Barlow) transformation
        {
            // Inverts Barlow logic: lower indispensability = more likely to have onsets
            // Creates anti-metrical, groove-oriented patterns
            result = generateBarlowTransformation(currentPattern, nextOnsets, true);
            break;
        }
        case 'e': // Euclidean transformation
        {
            // Uses Euclidean algorithm for even distribution
            // Maintains rhythmic regularity during transformation
            result = generateEuclideanTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'd': // Dilcue (anti-Euclidean) transformation
        {
            // Inverts Euclidean logic for more irregular patterns
            // Creates syncopated, unexpected rhythmic structures
            result = generateEuclideanTransformation(currentPattern, nextOnsets, true);
            break;
        }
        default:
        {
            // Fallback: return current pattern unchanged
            result = currentPattern;
            break;
        }
    }
    
    // Store the result for the next step 
    // This becomes the current pattern for the next transformation call
    progressivePatterns[patternKey] = result;
    
    // Increment step counter to reflect what we're about to return
    // This ensures the UI shows the correct step number (1-based)
    // Step 1 = base pattern, Step 2 = first transformation, etc.
    progressiveStepCount[patternKey]++;
    
    
    return result;
}

std::vector<bool> UPIParser::dilatePattern(const std::vector<bool>& pattern, int newSteps)
{
    if (newSteps <= static_cast<int>(pattern.size()))
        return pattern;
    
    std::vector<bool> dilated(newSteps, false);
    int originalSteps = static_cast<int>(pattern.size());
    
    // Map each onset to the new time grid
    for (int i = 0; i < originalSteps; ++i)
    {
        if (pattern[i])
        {
            int newPos = (i * newSteps) / originalSteps;
            if (newPos < newSteps)
                dilated[newPos] = true;
        }
    }
    
    return dilated;
}

std::vector<bool> UPIParser::concentratePattern(const std::vector<bool>& pattern, int newSteps)
{
    if (newSteps >= static_cast<int>(pattern.size()))
        return pattern;
    
    std::vector<bool> concentrated(newSteps, false);
    int originalSteps = static_cast<int>(pattern.size());
    
    // Map onsets to the new compressed time grid
    for (int i = 0; i < originalSteps; ++i)
    {
        if (pattern[i])
        {
            int newPos = (i * newSteps) / originalSteps;
            if (newPos < newSteps)
                concentrated[newPos] = true;
        }
    }
    
    return concentrated;
}

std::vector<bool> UPIParser::generateBarlowTransformation(const std::vector<bool>& originalPattern, int targetOnsets, bool wolrabMode)
{
    int stepCount = static_cast<int>(originalPattern.size());
    int currentOnsets = countOnsets(originalPattern);
    
    if (currentOnsets == targetOnsets) return originalPattern;
    
    // Calculate Barlow indispensability for all positions
    std::vector<std::pair<int, double>> allPositions;
    for (int i = 0; i < stepCount; ++i)
    {
        double indispensability = calculateBarlowIndispensability(i, stepCount);
        allPositions.push_back({i, indispensability});
    }
    
    if (targetOnsets < currentOnsets)
    {
        // Dilution: remove onsets
        return diluteByBarlow(originalPattern, targetOnsets, allPositions, wolrabMode);
    }
    else
    {
        // Concentration: add onsets
        return concentrateByBarlow(originalPattern, targetOnsets, allPositions, wolrabMode);
    }
}

std::vector<bool> UPIParser::generateEuclideanTransformation(const std::vector<bool>& originalPattern, int targetOnsets, bool antiMode)
{
    int stepCount = static_cast<int>(originalPattern.size());
    
    if (targetOnsets == 0)
    {
        return std::vector<bool>(stepCount, false);
    }
    else if (targetOnsets == stepCount)
    {
        return std::vector<bool>(stepCount, true);
    }
    else if (antiMode)
    {
        // Dilcue (Anti-Euclidean): use complement pattern
        auto euclideanPattern = bjorklundAlgorithm(stepCount - targetOnsets, stepCount);
        return invertPattern(euclideanPattern);
    }
    else
    {
        // Normal Euclidean using Bjorklund algorithm
        return bjorklundAlgorithm(targetOnsets, stepCount);
    }
}

double UPIParser::calculateBarlowIndispensability(int position, int stepCount)
{
    // Authentic Barlow indispensability based on Clarence Barlow's theory
    // Uses algorithmic approach that works for ALL step counts including primes
    
    if (position == 0) {
        // Downbeat always has maximum indispensability
        return 10.0;
    }
    
    // Calculate indispensability using metric strength theory
    // This creates proper hierarchy even for prime step counts
    
    double indispensability = 0.0;
    
    // Method 1: GCD-based metric strength (works for composite numbers)
    int gcd_value = gcd(position, stepCount);
    if (gcd_value > 1) {
        // Position aligns with a metric subdivision
        indispensability = static_cast<double>(gcd_value) / stepCount * 10.0;
    }
    
    // Method 2: Fractional position strength (works for ALL numbers including primes)
    // Calculate how this position relates to common musical subdivisions
    double positionRatio = static_cast<double>(position) / stepCount;
    
    // Check alignment with common musical fractions
    double fractionStrengths[] = {
        1.0/2.0,  // Half (strongest secondary accent)
        1.0/4.0, 3.0/4.0,  // Quarters
        1.0/3.0, 2.0/3.0,  // Thirds
        1.0/8.0, 3.0/8.0, 5.0/8.0, 7.0/8.0,  // Eighths
        1.0/6.0, 5.0/6.0,  // Sixths
    };
    
    double fractionValues[] = {
        5.0,      // Half gets strong accent
        3.0, 3.0, // Quarters
        2.5, 2.5, // Thirds  
        1.5, 1.5, 1.5, 1.5, // Eighths
        1.0, 1.0  // Sixths
    };
    
    // Find closest musical fraction and assign its strength
    double closestDistance = 1.0;
    double fractionStrength = 0.0;
    
    for (int i = 0; i < 11; ++i) {
        double distance = std::abs(positionRatio - fractionStrengths[i]);
        if (distance < closestDistance) {
            closestDistance = distance;
            fractionStrength = fractionValues[i];
        }
    }
    
    // Apply fraction strength if it's close enough (tolerance for discrete positions)
    double tolerance = 0.5 / stepCount; // Half a step tolerance
    if (closestDistance <= tolerance) {
        indispensability = std::max(indispensability, fractionStrength);
    }
    
    // Method 3: Position-based weighting for remaining positions
    // Creates hierarchy based on distance from strong positions
    if (indispensability < 0.5) {
        // Distance from center (creates symmetrical hierarchy)
        double centerDistance = std::abs(position - stepCount / 2.0) / (stepCount / 2.0);
        
        // Distance from edges (pickup and anacrusis effects)
        double edgeDistance = std::min(position, stepCount - position) / (stepCount / 2.0);
        
        // Combine for unique values that avoid sequential filling
        indispensability = (1.0 - centerDistance * 0.3) + (edgeDistance * 0.2);
        
        // Add small position-dependent variation to break ties
        indispensability += (position % 3) * 0.01 + (position % 5) * 0.005;
    }
    
    // Special position bonuses
    if (position == stepCount - 1) {
        // Pickup beat (last position) gets high indispensability
        indispensability = std::max(indispensability, 7.0);
    }
    
    // Ensure all positions have unique values and avoid sequential patterns
    return std::max(indispensability, 0.1 + (position * 0.001));
}

std::vector<bool> UPIParser::diluteByBarlow(const std::vector<bool>& pattern, int targetOnsets, 
                                           const std::vector<std::pair<int, double>>& indispensabilityTable, 
                                           bool wolrabMode)
{
    int stepCount = static_cast<int>(pattern.size());
    int currentOnsets = countOnsets(pattern);
    int onsetsToRemove = currentOnsets - targetOnsets;
    
    // Get all current onset positions with their indispensability
    std::vector<std::pair<int, double>> onsetPositions;
    for (int i = 0; i < stepCount; ++i)
    {
        if (pattern[i])
        {
            onsetPositions.push_back({i, indispensabilityTable[i].second});
        }
    }
    
    // Sort by indispensability 
    std::sort(onsetPositions.begin(), onsetPositions.end(), 
        [wolrabMode](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            if (wolrabMode)
            {
                // Wolrab: remove most indispensable first (reverse Barlow)
                return a.second > b.second;
            }
            else
            {
                // Normal Barlow: remove least indispensable first
                return a.second < b.second;
            }
        });
    
    // Create new pattern and remove onsets
    std::vector<bool> result = pattern;
    for (int i = 0; i < std::min(onsetsToRemove, static_cast<int>(onsetPositions.size())); ++i)
    {
        result[onsetPositions[i].first] = false;
    }
    
    return result;
}

std::vector<bool> UPIParser::concentrateByBarlow(const std::vector<bool>& pattern, int targetOnsets,
                                                 const std::vector<std::pair<int, double>>& indispensabilityTable,
                                                 bool wolrabMode)
{
    int stepCount = static_cast<int>(pattern.size());
    int currentOnsets = countOnsets(pattern);
    int onsetsToAdd = targetOnsets - currentOnsets;
    
    // Get all empty positions with their indispensability
    std::vector<std::pair<int, double>> emptyPositions;
    for (int i = 0; i < stepCount; ++i)
    {
        if (!pattern[i])
        {
            emptyPositions.push_back({i, indispensabilityTable[i].second});
        }
    }
    
    // Sort by indispensability
    std::sort(emptyPositions.begin(), emptyPositions.end(),
        [wolrabMode](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            if (wolrabMode)
            {
                // Wolrab: add least indispensable first (reverse Barlow)
                return a.second < b.second;
            }
            else
            {
                // Normal Barlow: add most indispensable first
                return a.second > b.second;
            }
        });
    
    // Create new pattern and add onsets
    std::vector<bool> result = pattern;
    for (int i = 0; i < std::min(onsetsToAdd, static_cast<int>(emptyPositions.size())); ++i)
    {
        result[emptyPositions[i].first] = true;
    }
    
    return result;
}

void UPIParser::resetProgressiveState(const juce::String& patternKey)
{
    progressivePatterns.erase(patternKey);
    progressiveAccessCount.erase(patternKey);
    progressiveStepCount.erase(patternKey);
}

void UPIParser::resetAllProgressiveStates()
{
    progressivePatterns.clear();
    progressiveAccessCount.clear();
    progressiveStepCount.clear();
}

int UPIParser::getProgressiveStepCount(const juce::String& patternKey)
{
    auto it = progressiveStepCount.find(patternKey);
    if (it != progressiveStepCount.end())
    {
        // Return the current step count directly
        return it->second;
    }
    return 1; // Default to step 1 if not found
}

//==============================================================================
// Error handling

UPIParser::ParseResult UPIParser::createError(const juce::String& message)
{
    ParseResult result;
    result.type = ParseResult::Error;
    result.errorMessage = message;
    return result;
}

UPIParser::ParseResult UPIParser::createSuccess(const std::vector<bool>& pattern, const juce::String& name)
{
    ParseResult result;
    result.type = ParseResult::Single;
    result.pattern = pattern;
    result.patternName = name;
    result.stepCount = static_cast<int>(pattern.size());
    return result;
}

//==============================================================================
// Progressive offset engine support - implementation

void UPIParser::setProgressiveOffsetEngine(PatternEngine* engine)
{
    progressiveOffsetEngine = engine;
    hasProgressiveOffsetEngine = (engine != nullptr);
}

int UPIParser::getCurrentProgressiveOffset()
{
    if (hasProgressiveOffsetEngine && progressiveOffsetEngine != nullptr)
    {
        return progressiveOffsetEngine->getCurrentOffset();
    }
    return 0;
}