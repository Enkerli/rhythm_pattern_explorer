/*
  ==============================================================================

    Universal Pattern Input (UPI) Parser Implementation

  ==============================================================================
*/

#include "UPIParser.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <functional>

//==============================================================================
UPIParser::ParseResult UPIParser::parse(const juce::String& input)
{
    if (input.trim().isEmpty())
        return createError("Empty input");
    
    juce::String cleaned = cleanInput(input);
    
    // Check for combinations (+ or -)
    if (cleaned.contains("+") || cleaned.contains("-"))
    {
        // Handle pattern combinations - support multiple additions
        auto parts = tokenize(cleaned, "+");
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
                        int steps = match[3].matched ? std::stoi(match[3].str()) : sides;
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
    
    // Parse as single pattern
    auto result = parsePattern(cleaned);
    if (result.isValid())
    {
        result.type = ParseResult::Single;
    }
    return result;
}

UPIParser::ParseResult UPIParser::parsePattern(const juce::String& input)
{
    juce::String cleaned = cleanInput(input);
    
    // Debug logging for pattern recognition
    juce::File logFile("/tmp/rhythm_debug.log");
    logFile.appendText("=== parsePattern called ===\n");
    logFile.appendText("Input: '" + input + "'\n");
    logFile.appendText("Cleaned: '" + cleaned + "'\n");
    logFile.appendText("isEuclideanPattern: " + juce::String(isEuclideanPattern(cleaned) ? "true" : "false") + "\n");
    logFile.appendText("isBinaryPattern: " + juce::String(isBinaryPattern(cleaned) ? "true" : "false") + "\n");
    
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
            
            DBG("Progressive pattern detected: " + cleaned);
            DBG("   Base pattern (raw): " + basePattern);
            DBG("   Target onsets: " + juce::String(targetOnsets));
            
            // Also write to file for debugging
            juce::File logFile("/tmp/rhythm_debug.log");
            logFile.appendText("Progressive pattern detected: " + cleaned + "\n");
            logFile.appendText("Base pattern (raw): " + basePattern + "\n");
            logFile.appendText("Target onsets: " + juce::String(targetOnsets) + "\n");
            
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
            
            DBG("   Transformer type: " + juce::String(transformerType));
            DBG("   Final base pattern: " + basePattern);
            
            // Log to file
            logFile.appendText("Transformer type: " + juce::String(transformerType) + "\n");
            logFile.appendText("Final base pattern: " + basePattern + "\n");
            
            auto baseResult = parsePattern(basePattern);
            if (baseResult.isValid())
            {
                DBG("   Base result pattern: " + patternToBinary(baseResult.pattern));
                DBG("   Base result onsets: " + juce::String(countOnsets(baseResult.pattern)));
                
                // Log to file
                logFile.appendText("Base result pattern: " + patternToBinary(baseResult.pattern) + "\n");
                logFile.appendText("Base result onsets: " + juce::String(countOnsets(baseResult.pattern)) + "\n");
                
                // Apply progressive transformation with target onset count
                auto transformed = applyProgressiveTransformation(baseResult.pattern, transformerType, targetOnsets);
                DBG("   Transformed pattern: " + patternToBinary(transformed));
                DBG("   Transformed onsets: " + juce::String(countOnsets(transformed)));
                
                // Log to file
                logFile.appendText("Transformed pattern: " + patternToBinary(transformed) + "\n");
                logFile.appendText("Transformed onsets: " + juce::String(countOnsets(transformed)) + "\n");
                logFile.appendText("---\n");
                
                return createSuccess(transformed, "Progressive: " + cleaned);
            }
        }
    }
    
    // Parse core pattern types
    if (isEuclideanPattern(cleaned))
    {
        // E(onsets,steps) or E(onsets,steps,offset) - case insensitive
        std::regex euclideanRegex(R"([Ee]\((\d+),(\d+)(?:,(-?\d+))?\))");
        std::smatch match;
        std::string inputStr = cleaned.toStdString();
        
        if (std::regex_search(inputStr, match, euclideanRegex))
        {
            int onsets = std::stoi(match[1].str());
            int steps = std::stoi(match[2].str());
            int offset = match[3].matched ? std::stoi(match[3].str()) : 0;
            
            auto pattern = parseEuclidean(onsets, steps, offset);
            return createSuccess(pattern, "E(" + juce::String(onsets) + "," + juce::String(steps) + 
                               (offset != 0 ? "," + juce::String(offset) : "") + ")");
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
            int steps = match[3].matched ? std::stoi(match[3].str()) : sides;
            
            auto pattern = parsePolygon(sides, offset, steps);
            return createSuccess(pattern, "P(" + juce::String(sides) + "," + juce::String(offset) + 
                               (match[3].matched ? "," + juce::String(steps) : "") + ")");
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
    
    if (isHexPattern(cleaned))
    {
        // 0x92:8 format
        if (cleaned.contains(":"))
        {
            auto parts = tokenize(cleaned, ":");
            if (parts.size() == 2)
            {
                auto pattern = parseHex(parts[0].trim(), parts[1].trim().getIntValue());
                return createSuccess(pattern, "Hex: " + parts[0].trim());
            }
        }
        else
        {
            // Default to 8 steps if no explicit count
            auto pattern = parseHex(cleaned, 8);
            return createSuccess(pattern, "Hex: " + cleaned);
        }
    }
    
    // Check for decimal patterns (with or without step count)
    if (isDecimalPattern(cleaned) || (cleaned.contains(":") && isDecimalPattern(cleaned.upToFirstOccurrenceOf(":", false, false))))
    {
        // d146:8 format
        if (cleaned.contains(":"))
        {
            auto parts = tokenize(cleaned, ":");
            if (parts.size() == 2)
            {
                int decimal = parts[0].substring(1).getIntValue(); // Remove 'd' prefix
                auto pattern = parseDecimal(decimal, parts[1].trim().getIntValue());
                return createSuccess(pattern, "Decimal: " + parts[0].trim());
            }
        }
        else
        {
            // Default to appropriate steps if no explicit count
            juce::String decimalStr = cleaned.substring(1); // Remove 'd' prefix
            int decimal = decimalStr.getIntValue();
            int minSteps = decimal > 0 ? static_cast<int>(std::ceil(std::log2(decimal + 1))) : 1;
            int targetSteps = std::max(minSteps, 8);
            auto pattern = parseDecimal(decimal, targetSteps);
            return createSuccess(pattern, "Decimal: " + cleaned);
        }
    }
    
    // Check for octal patterns (with or without step count)
    if (isOctalPattern(cleaned) || (cleaned.contains(":") && isOctalPattern(cleaned.upToFirstOccurrenceOf(":", false, false))))
    {
        // o222:8 format
        if (cleaned.contains(":"))
        {
            auto parts = tokenize(cleaned, ":");
            if (parts.size() == 2)
            {
                auto pattern = parseOctal(parts[0].trim(), parts[1].trim().getIntValue());
                return createSuccess(pattern, "Octal: " + parts[0].trim());
            }
        }
        else
        {
            // Default to 8 steps if no explicit count
            auto pattern = parseOctal(cleaned, 8);
            return createSuccess(pattern, "Octal: " + cleaned);
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
        double exactPos = ((double)(i * totalSteps) / sides) + offset;
        int pos = ((int)std::round(exactPos)) % totalSteps;
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

std::vector<bool> UPIParser::parseHex(const juce::String& hexStr, int stepCount)
{
    // Remove 0x prefix
    juce::String hex = hexStr.startsWith("0x") ? hexStr.substring(2) : hexStr;
    
    int decimal = hex.getHexValue32();
    return parseDecimal(decimal, stepCount);
}

std::vector<bool> UPIParser::parseDecimal(int decimal, int stepCount)
{
    std::vector<bool> pattern;
    pattern.reserve(stepCount);
    
    // Convert decimal to binary, LEFT-TO-RIGHT: most significant bit first (leftmost bit = step 0)
    // This matches webapp standard where E(3,8) = 0x92 and E(5,8) = 0xB6
    for (int i = 0; i < stepCount; ++i)
    {
        pattern.push_back((decimal & (1 << (stepCount - 1 - i))) != 0);
    }
    
    return pattern;
}

std::vector<bool> UPIParser::parseOctal(const juce::String& octalStr, int stepCount)
{
    // Remove 'o' prefix if present
    juce::String octal = octalStr.startsWith("o") ? octalStr.substring(1) : octalStr;
    
    // Convert octal string to decimal
    int decimal = 0;
    int base = 1;
    
    // Process from right to left (least significant digit first)
    for (int i = octal.length() - 1; i >= 0; --i)
    {
        char digit = octal[i];
        if (digit >= '0' && digit <= '7')
        {
            decimal += (digit - '0') * base;
            base *= 8;
        }
        else
        {
            // Invalid octal digit
            return std::vector<bool>(stepCount, false);
        }
    }
    
    return parseDecimal(decimal, stepCount);
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
// Pattern recognition helpers

bool UPIParser::isEuclideanPattern(const juce::String& input)
{
    return (input.startsWith("E(") || input.startsWith("e(")) && input.endsWith(")");
}

bool UPIParser::isPolygonPattern(const juce::String& input)
{
    return (input.startsWith("P(") || input.startsWith("p(")) && input.endsWith(")");
}

bool UPIParser::isBinaryPattern(const juce::String& input)
{
    juce::String processed = input.startsWith("b") ? input.substring(1) : input;
    if (processed.contains(":"))
        processed = processed.upToFirstOccurrenceOf(":", false, false);
    
    return processed.containsOnly("01");
}

bool UPIParser::isArrayPattern(const juce::String& input)
{
    return input.startsWith("[") && input.contains("]");
}

bool UPIParser::isRandomPattern(const juce::String& input)
{
    return (input.startsWith("R(") || input.startsWith("r(")) && input.endsWith(")");
}

bool UPIParser::isHexPattern(const juce::String& input)
{
    return input.startsWith("0x");
}

bool UPIParser::isDecimalPattern(const juce::String& input)
{
    return input.startsWith("d") && input.substring(1).containsOnly("0123456789");
}

bool UPIParser::isOctalPattern(const juce::String& input)
{
    return input.startsWith("o") && input.substring(1).containsOnly("01234567");
}

bool UPIParser::isMorsePattern(const juce::String& input)
{
    return input.startsWith("m:") || input.containsOnly(".-");
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

std::vector<bool> UPIParser::applyProgressiveTransformation(const std::vector<bool>& basePattern, char transformerType, int targetOnsets)
{
    // Create a unique key for this progressive pattern
    juce::String patternKey = patternToBinary(basePattern) + juce::String(transformerType) + juce::String(targetOnsets);
    
    DBG("Progressive transformation called:");
    DBG("   Pattern key: " + patternKey);
    DBG("   Target onsets: " + juce::String(targetOnsets));
    
    // Initialize or get the current pattern state
    std::vector<bool> currentPattern;
    bool isFirstCall = false;
    
    if (progressivePatterns.find(patternKey) == progressivePatterns.end())
    {
        // First time - return base pattern directly without transformation
        progressivePatterns[patternKey] = basePattern;
        DBG("   First call - returning base pattern: " + patternToBinary(basePattern));
        return basePattern;
    }
    else
    {
        // Get the current state from previous transformation
        currentPattern = progressivePatterns[patternKey];
        DBG("   Continuing from stored pattern: " + patternToBinary(currentPattern));
    }
    
    int currentOnsets = countOnsets(currentPattern);
    DBG("   Current onsets: " + juce::String(currentOnsets));
    
    // Calculate the next step in the progression
    int direction = (targetOnsets > currentOnsets) ? 1 : -1;
    int nextOnsets = currentOnsets + direction;
    
    DBG("   Direction: " + juce::String(direction));
    DBG("   Next onsets target: " + juce::String(nextOnsets));
    
    // Clamp to valid range
    if (direction > 0)
    {
        nextOnsets = std::min(nextOnsets, targetOnsets);
    }
    else
    {
        nextOnsets = std::max(nextOnsets, targetOnsets);
    }
    
    DBG("   Clamped next onsets: " + juce::String(nextOnsets));
    
    // Transform from current pattern to next step
    std::vector<bool> result;
    
    switch (transformerType)
    {
        case 'b': // Barlow transformation
        {
            result = generateBarlowTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'w': // Wolrab (anti-Barlow) transformation
        {
            result = generateBarlowTransformation(currentPattern, nextOnsets, true);
            break;
        }
        case 'e': // Euclidean transformation
        {
            result = generateEuclideanTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'd': // Dilcue (anti-Euclidean) transformation
        {
            result = generateEuclideanTransformation(currentPattern, nextOnsets, true);
            break;
        }
        default:
        {
            result = currentPattern;
            break;
        }
    }
    
    // Store the result for the next step
    progressivePatterns[patternKey] = result;
    
    DBG("   Final result: " + patternToBinary(result));
    DBG("   Final onsets: " + juce::String(countOnsets(result)));
    
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
    // Calculate base indispensability using Barlow's method with prime factorization
    double indispensability = 0.0;
    int currentStepCount = stepCount;
    int currentPosition = position;
    
    // Check divisibility by powers of 2 first (most important in Western rhythm)
    while (currentStepCount % 2 == 0 && currentPosition % 2 == 0)
    {
        indispensability += 1.0 / 2.0; // Strong beats get higher values
        currentStepCount /= 2;
        currentPosition /= 2;
    }
    
    // Check divisibility by 3 (important for compound meters)
    while (currentStepCount % 3 == 0 && currentPosition % 3 == 0)
    {
        indispensability += 1.0 / 3.0;
        currentStepCount /= 3;
        currentPosition /= 3;
    }
    
    // Check other prime factors
    for (int prime = 5; prime <= currentStepCount; prime += 2)
    {
        if (currentStepCount % prime == 0 && currentPosition % prime == 0)
        {
            indispensability += 1.0 / prime;
            while (currentStepCount % prime == 0 && currentPosition % prime == 0)
            {
                currentStepCount /= prime;
                currentPosition /= prime;
            }
        }
    }
    
    // Base indispensability for positions that don't align with strong metric divisions
    if (indispensability == 0.0)
    {
        indispensability = 0.1;
    }
    
    // Special position bonuses to ensure correct ordering
    if (position == 0) 
    {
        // Downbeat gets small bonus to break ties with other strong beats
        indispensability += 0.01; 
    }
    else if (position == stepCount - 1) 
    {
        // Pickup beat has high indispensability but never higher than downbeat
        indispensability = std::max(indispensability, 0.75);
    }
    
    return indispensability;
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
}

void UPIParser::resetAllProgressiveStates()
{
    DBG("Resetting ALL progressive states");
    progressivePatterns.clear();
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