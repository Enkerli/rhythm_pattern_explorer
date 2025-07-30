/*
  ==============================================================================

    Universal Pattern Input (UPI) Parser Implementation

  ==============================================================================
*/

#include "UPIParser.h"
#include "PatternEngine.h"
#include "PatternUtils.h"
#include "QuantizationEngine.h"
#include "DebugConfig.h"
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
    
    // Check for accent pattern in curly braces
    ParseResult result;
    juce::String basePattern = cleaned;
    
    if (cleaned.contains("{") && cleaned.contains("}"))
    {
        int braceStart = cleaned.indexOf("{");
        int braceEnd = cleaned.indexOf("}");
        
        if (braceStart < braceEnd && braceStart >= 0 && braceEnd >= 0)
        {
            // Extract accent pattern
            juce::String accentSection = cleaned.substring(braceStart + 1, braceEnd);
            result.hasAccentPattern = true;
            result.accentPattern = parseAccentPattern(accentSection);
            result.accentPatternName = "{" + accentSection + "}";
            
            // Remove accent section from base pattern
            basePattern = cleaned.substring(0, braceStart) + cleaned.substring(braceEnd + 1);
            basePattern = basePattern.trim();
        }
    }
    
    // Check for combinations (+ or -) in the base pattern (without accents)
    // BUT first check if + represents a progressive offset (pattern+number)
    if (basePattern.contains("+") || basePattern.contains("-"))
    {
        // Check if this is a progressive offset pattern (pattern+N) rather than pattern combination
        if (basePattern.contains("+") && basePattern.lastIndexOf("+") > 0)
        {
            int lastPlusIndex = basePattern.lastIndexOf("+");
            juce::String afterPlus = basePattern.substring(lastPlusIndex + 1).trim();
            if (afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty())
            {
                // This is a progressive offset pattern - don't treat as combination
                // Extract base pattern and offset value
                juce::String basePatternOnly = basePattern.substring(0, lastPlusIndex).trim();
                int offsetValue = afterPlus.getIntValue();
                
                
                // Parse the base pattern
                auto baseResult = parsePattern(basePatternOnly);
                if (baseResult.isValid())
                {
                    // Don't apply the offset here - that's handled by the PatternEngine
                    // Just mark it as a progressive pattern
                    baseResult.hasProgressiveOffset = true;
                    baseResult.initialOffset = 0; // Progressive offsets start at 0
                    baseResult.progressiveOffset = offsetValue;
                    baseResult.patternName += "+" + juce::String(offsetValue);
                    
                    
                    return baseResult;
                }
            }
        }
    }
    
    // If we get here, it's not a progressive offset, so check for pattern combinations
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
                    targetLCM = PatternUtils::lcm(targetLCM, polygonSizes[i]);
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
                    
                    result.pattern = PatternUtils::combinePatterns(result.pattern, nextResult.pattern, true);
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
    auto patternResult = parsePattern(basePattern);
    if (patternResult.isValid())
    {
        patternResult.type = ParseResult::Single;
        
        // Merge accent information with pattern result
        if (result.hasAccentPattern)
        {
            patternResult.hasAccentPattern = result.hasAccentPattern;
            patternResult.accentPattern = result.accentPattern;
            patternResult.accentPatternName = result.accentPatternName;
        }
    }
    return patternResult;
}

UPIParser::ParseResult UPIParser::parsePattern(const juce::String& input)
{
    juce::String cleaned = cleanInput(input);
    
    
    
    // Handle transformations first
    if (cleaned.startsWith("~") || cleaned.startsWith("inv "))
    {
        juce::String basePattern = cleaned.startsWith("~") ? 
            cleaned.substring(1).trim() : 
            cleaned.substring(4).trim();
        
        auto baseResult = parsePattern(basePattern);
        if (baseResult.isValid())
        {
            auto inverted = PatternUtils::invertPattern(baseResult.pattern);
            return createSuccess(inverted, "Inverted " + baseResult.patternName);
        }
    }
    
    if (cleaned.startsWith("rev "))
    {
        auto baseResult = parsePattern(cleaned.substring(4).trim());
        if (baseResult.isValid())
        {
            auto reversed = PatternUtils::reversePattern(baseResult.pattern);
            return createSuccess(reversed, "Reversed " + baseResult.patternName);
        }
    }
    
    if (cleaned.startsWith("comp "))
    {
        auto baseResult = parsePattern(cleaned.substring(5).trim());
        if (baseResult.isValid())
        {
            auto complement = PatternUtils::complementPattern(baseResult.pattern);
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
                auto rotated = PatternUtils::rotatePattern(baseResult.pattern, -rotationSteps);
                return createSuccess(rotated, baseResult.patternName + "@" + juce::String(rotationSteps));
            }
        }
    }
    
    // Handle quantization: pattern;steps or pattern;-steps (Lascabettes-style)
    if (QuantizationEngine::hasQuantizationNotation(cleaned))
    {
        
        auto quantParams = QuantizationEngine::parseQuantizationNotation(cleaned);
        if (quantParams.isValid)
        {
 
            
            // Parse the base pattern first
            auto baseResult = parsePattern(quantParams.patternPart);
            if (baseResult.isValid())
            {
                // Apply quantization to the parsed pattern
                auto quantResult = QuantizationEngine::quantizePattern(
                    baseResult.pattern, 
                    quantParams.newStepCount, 
                    quantParams.clockwise
                );
                
                if (quantResult.isValid)
                {
                    // Create success result with quantized pattern
                    juce::String directionSymbol = quantParams.clockwise ? "↻" : "↺";
                    juce::String quantizedName = baseResult.patternName + ";" + 
                                               juce::String(quantParams.clockwise ? "" : "-") + 
                                               juce::String(quantParams.newStepCount) + directionSymbol;
                    
                    auto result = createSuccess(quantResult.pattern, quantizedName);
                    
                    // Add quantization metadata
                    result.hasQuantization = true;
                    result.originalStepCount = quantResult.originalStepCount;
                    result.quantizedStepCount = quantResult.quantizedStepCount;
                    result.quantizationClockwise = quantResult.isClockwise;
                    result.originalOnsetCount = quantResult.originalOnsetCount;
                    result.quantizedOnsetCount = quantResult.quantizedOnsetCount;
                    
                    // Preserve any accent pattern from base result
                    if (baseResult.hasAccentPattern)
                    {
                        result.hasAccentPattern = baseResult.hasAccentPattern;
                        result.accentPattern = baseResult.accentPattern;
                        result.accentPatternName = baseResult.accentPatternName;
                    }
                    
                    
                    return result;
                }
                else
                {
                    return createError("Quantization failed: " + quantResult.errorMessage);
                }
            }
            else
            {
                return createError("Invalid base pattern for quantization: " + quantParams.patternPart);
            }
        }
        else
        {
            return createError("Invalid quantization notation: " + quantParams.errorMessage);
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
                result.progressivePatternKey = PatternUtils::patternToBinary(baseResult.pattern) + juce::String(transformerType) + juce::String(targetOnsets);
                
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
            if (parts.size() == 2 && !parts[1].trim().isEmpty())
            {
                int stepCount = parts[1].trim().getIntValue();
                if (stepCount > 0) // Validate that step count is positive
                {
                    auto pattern = parseBinary(parts[0].trim(), stepCount);
                    return createSuccess(pattern, "Binary: " + parts[0].trim());
                }
            }
            // If colon is present but step count is invalid, fall back to implicit length
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
        {"0o", NumericBase::Octal, "01234567"},
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
            
            auto pattern = PatternUtils::generateBarlowTransformation(basePattern, onsets, false);
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
            
            auto pattern = PatternUtils::generateBarlowTransformation(basePattern, onsets, true); // true = Wolrab mode
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
            
            auto pattern = PatternUtils::generateEuclideanTransformation(std::vector<bool>(steps, false), onsets, true); // true = anti-Euclidean
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
    auto pattern = PatternUtils::bjorklundAlgorithm(onsets, steps);
    if (offset != 0)
    {
        // Negate offset to make positive offsets go clockwise (webapp standard)
        pattern = PatternUtils::rotatePattern(pattern, -offset);
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
    
    // Handle common morse patterns and letter-to-morse conversion
    if (processed == "sos")
        processed = "...---...";
    else if (processed == "cq")
        processed = "-.-.--.-";
    else {
        // Multi-character letter sequence conversion
        juce::String morseCode = "";
        bool hasValidLetters = false;
        
        for (int i = 0; i < processed.length(); ++i) {
            char letter = processed[i];
            juce::String letterMorse = "";
            
            switch (letter) {
                case 'a': letterMorse = ".-"; break;
                case 'b': letterMorse = "-..."; break;
                case 'c': letterMorse = "-.-."; break;
                case 'd': letterMorse = "-.."; break;
                case 'e': letterMorse = "."; break;
                case 'f': letterMorse = "..-."; break;
                case 'g': letterMorse = "--."; break;
                case 'h': letterMorse = "...."; break;
                case 'i': letterMorse = ".."; break;
                case 'j': letterMorse = ".---"; break;
                case 'k': letterMorse = "-.-"; break;
                case 'l': letterMorse = ".-.."; break;
                case 'm': letterMorse = "--"; break;
                case 'n': letterMorse = "-."; break;
                case 'o': letterMorse = "---"; break;
                case 'p': letterMorse = ".--."; break;
                case 'q': letterMorse = "--.-"; break;
                case 'r': letterMorse = ".-."; break;
                case 's': letterMorse = "..."; break;
                case 't': letterMorse = "-"; break;
                case 'u': letterMorse = "..-"; break;
                case 'v': letterMorse = "...-"; break;
                case 'w': letterMorse = ".--"; break;
                case 'x': letterMorse = "-..-"; break;
                case 'y': letterMorse = "-.--"; break;
                case 'z': letterMorse = "--.."; break;
                default: 
                    // Keep non-letters as-is (for direct morse like '.-')
                    letterMorse = juce::String::charToString(letter);
                    break;
            }
            
            if (letterMorse != juce::String::charToString(letter)) {
                hasValidLetters = true;
            }
            
            morseCode += letterMorse;
            
            // Note: No spaces between letters for direct concatenation
            // This allows patterns like "AL" -> ".-" + ".-.." = "110" + "11011" = "11011011"
        }
        
        // Only use converted morse if we found valid letters
        if (hasValidLetters) {
            processed = morseCode;
        }
    }
    
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
    
    // Allow natural Morse code length without forced padding
    
    return pattern;
}

//==============================================================================
// Pattern transformations and utilities moved to PatternUtils namespace

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
    
    // Special handling for Array patterns (supports explicit step count with colon)
    if (type == PatternType::Array && matchesStart) {
        return input.endsWith("]") || input.contains("]:");
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
    // Check for both "0o" and "o" prefixes for octal patterns
    static const NumericPatternInfo octalInfo1 = {"0o", NumericBase::Octal, "01234567"};
    static const NumericPatternInfo octalInfo2 = {"o", NumericBase::Octal, "01234567"};
    return isNumericPattern(input, octalInfo1) || isNumericPattern(input, octalInfo2);
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
            // Parse octal with strict left-to-right notation where octal digits are reversed
            decimal = 0;
            
            // Process octal digits in reverse order for left-to-right bit mapping
            for (int i = content.length() - 1; i >= 0; --i)
            {
                int octalDigit = content[i] - '0';
                
                // Shift previous digits and add this one
                decimal = (decimal << 3) | octalDigit;
            }
            break;
            
        case NumericBase::Decimal:
            decimal = content.getIntValue();
            break;
            
        case NumericBase::Hexadecimal:
            // Parse hex with strict left-to-right notation where hex digits are reversed
            // So 0x94 becomes 0x49 to produce left-to-right bit interpretation 
            decimal = 0;
            
            // Process hex digits in reverse order for left-to-right bit mapping
            for (int i = content.length() - 1; i >= 0; --i)
            {
                int hexDigit;
                if (content[i] >= '0' && content[i] <= '9')
                    hexDigit = content[i] - '0';
                else if (content[i] >= 'A' && content[i] <= 'F')
                    hexDigit = content[i] - 'A' + 10;
                else if (content[i] >= 'a' && content[i] <= 'f')
                    hexDigit = content[i] - 'a' + 10;
                else
                    continue;
                
                // Shift previous digits and add this one
                decimal = (decimal << 4) | hexDigit;
            }
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
    int toRemove = static_cast<int>(progressivePatterns.size()) - (MAX_PROGRESSIVE_STATES / 2);
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
    juce::String patternKey = PatternUtils::patternToBinary(basePattern) + juce::String(transformerType) + juce::String(targetOnsets);
    
    
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
    
    int currentOnsets = PatternUtils::countOnsets(currentPattern);
    
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
            result = PatternUtils::generateBarlowTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'w': // Wolrab (anti-Barlow) transformation
        {
            // Inverts Barlow logic: lower indispensability = more likely to have onsets
            // Creates anti-metrical, groove-oriented patterns
            result = PatternUtils::generateBarlowTransformation(currentPattern, nextOnsets, true);
            break;
        }
        case 'e': // Euclidean transformation
        {
            // Uses Euclidean algorithm for even distribution
            // Maintains rhythmic regularity during transformation
            result = PatternUtils::generateEuclideanTransformation(currentPattern, nextOnsets, false);
            break;
        }
        case 'd': // Dilcue (anti-Euclidean) transformation
        {
            // Inverts Euclidean logic for more irregular patterns
            // Creates syncopated, unexpected rhythmic structures
            result = PatternUtils::generateEuclideanTransformation(currentPattern, nextOnsets, true);
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

//==============================================================================
std::vector<bool> UPIParser::parseAccentPattern(const juce::String& accentStr)
{
    juce::String trimmed = accentStr.trim();
    
    if (trimmed.isEmpty())
        return {};
    
    // Handle Euclidean accent patterns: E(3,8)
    if (trimmed.startsWith("E(") && trimmed.endsWith(")"))
    {
        auto content = trimmed.substring(2, trimmed.length() - 1);
        auto parts = juce::StringArray::fromTokens(content, ",", "");
        
        if (parts.size() >= 2)
        {
            int onsets = parts[0].trim().getIntValue();
            int steps = parts[1].trim().getIntValue();
            return parseEuclidean(onsets, steps);
        }
    }
    
    // Handle Barlow accent patterns: B(3,8)
    if (trimmed.startsWith("B(") && trimmed.endsWith(")"))
    {
        auto content = trimmed.substring(2, trimmed.length() - 1);
        auto parts = juce::StringArray::fromTokens(content, ",", "");
        
        if (parts.size() >= 2)
        {
            int onsets = parts[0].trim().getIntValue();
            int steps = parts[1].trim().getIntValue();
            
            // Create base pattern with single onset and transform to target using Barlow
            std::vector<bool> basePattern(steps, false);
            basePattern[0] = true; // Start with downbeat
            
            return PatternUtils::generateBarlowTransformation(basePattern, onsets, false);
        }
    }
    
    // Handle Morse code: .-- or .-.
    if (trimmed.containsAnyOf(".-"))
    {
        return parseMorse(trimmed);
    }
    
    // Handle hex patterns: 0x1A
    if (trimmed.startsWith("0x") || trimmed.startsWith("0X"))
    {
        auto hexPart = trimmed.substring(2);
        if (hexPart.containsOnly("0123456789ABCDEFabcdef"))
        {
            int decimal = hexPart.getHexValue32();
            int bits = hexPart.length() * 4; // Each hex digit = 4 bits
            return parseDecimal(decimal, bits);
        }
    }
    
    // Handle binary string: 101010
    if (trimmed.containsOnly("01"))
    {
        return parseBinary(trimmed);
    }
    
    // Default: treat as binary string, convert invalid chars to 0
    std::vector<bool> pattern;
    for (int i = 0; i < trimmed.length(); ++i)
    {
        pattern.push_back(trimmed[i] == '1');
    }
    
    return pattern;
}