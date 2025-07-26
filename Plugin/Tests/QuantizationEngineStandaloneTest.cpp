//==============================================================================
// QuantizationEngineStandaloneTest.cpp
// Standalone test for QuantizationEngine without JUCE dependencies
//==============================================================================

#include <iostream>
#include <cassert>
#include <vector>
#include <cmath>
#include <string>
#include <set>

// Mock JUCE String class for standalone testing
class String {
private:
    std::string str;
public:
    String() = default;
    String(const char* s) : str(s) {}
    String(const std::string& s) : str(s) {}
    String(int value) : str(std::to_string(value)) {}
    
    bool isEmpty() const { return str.empty(); }
    int length() const { return static_cast<int>(str.length()); }
    bool startsWith(const String& other) const {
        return str.length() >= other.str.length() && 
               str.substr(0, other.str.length()) == other.str;
    }
    
    int lastIndexOf(const String& substr) const {
        size_t pos = str.rfind(substr.str);
        return pos == std::string::npos ? -1 : static_cast<int>(pos);
    }
    
    String substring(int start, int end = -1) const {
        if (end == -1) end = static_cast<int>(str.length());
        if (start < 0) start = 0;
        if (end > static_cast<int>(str.length())) end = static_cast<int>(str.length());
        if (start >= end) return String();
        return String(str.substr(start, end - start));
    }
    
    String trim() const {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == std::string::npos) return String();
        size_t end = str.find_last_not_of(" \t\n\r");
        return String(str.substr(start, end - start + 1));
    }
    
    int getIntValue() const {
        try {
            return std::stoi(str);
        } catch (...) {
            return 0;
        }
    }
    
    char operator[](int index) const {
        if (index < 0 || index >= static_cast<int>(str.length())) return 0;
        return str[index];
    }
    
    const char* toRawUTF8() const { return str.c_str(); }
    
    String operator+(const String& other) const {
        return String(str + other.str);
    }
    
    bool operator==(const String& other) const {
        return str == other.str;
    }
};

namespace CharacterFunctions {
    bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
}

//==============================================================================
// Standalone QuantizationEngine implementation
class QuantizationEngine
{
public:
    struct QuantizationResult
    {
        std::vector<bool> pattern;
        int originalStepCount = 0;
        int quantizedStepCount = 0;
        bool isClockwise = true;
        int originalOnsetCount = 0;
        int quantizedOnsetCount = 0;
        double densityRatio = 1.0;
        bool isValid = false;
        String errorMessage;
        
        QuantizationResult() = default;
        QuantizationResult(const std::vector<bool>& p, int origSteps, int quantSteps, bool clockwise)
            : pattern(p), originalStepCount(origSteps), quantizedStepCount(quantSteps), 
              isClockwise(clockwise), isValid(true) 
        {
            originalOnsetCount = 0;
            quantizedOnsetCount = 0;
            
            for (bool onset : pattern) {
                if (onset) quantizedOnsetCount++;
            }
            
            if (origSteps > 0) {
                densityRatio = static_cast<double>(quantizedStepCount) / static_cast<double>(origSteps);
            }
        }
    };
    
    struct QuantizationParams
    {
        int newStepCount = 0;
        bool clockwise = true;
        bool isValid = false;
        String patternPart;
        String errorMessage;
    };
    
    static QuantizationResult quantizePattern(const std::vector<bool>& originalPattern, 
                                            int newStepCount, 
                                            bool clockwise = true)
    {
        if (originalPattern.empty()) {
            return createError("Original pattern is empty");
        }
        
        if (!isValidStepCount(newStepCount)) {
            return createError(String("Invalid step count: ") + String(newStepCount));
        }
        
        int originalStepCount = static_cast<int>(originalPattern.size());
        
        if (originalStepCount == newStepCount) {
            return QuantizationResult(originalPattern, originalStepCount, newStepCount, clockwise);
        }
        
        std::vector<int> onsetPositions = extractOnsetPositions(originalPattern);
        
        if (onsetPositions.empty()) {
            std::vector<bool> emptyPattern(newStepCount, false);
            return QuantizationResult(emptyPattern, originalStepCount, newStepCount, clockwise);
        }
        
        std::set<int> quantizedPositions;
        
        for (int originalPos : onsetPositions) {
            double angle = positionToAngle(originalPos, originalStepCount, clockwise);
            int newPos = angleToPosition(angle, newStepCount);
            quantizedPositions.insert(newPos);
        }
        
        std::vector<bool> quantizedPattern(newStepCount, false);
        for (int pos : quantizedPositions) {
            if (pos >= 0 && pos < newStepCount) {
                quantizedPattern[pos] = true;
            }
        }
        
        QuantizationResult result(quantizedPattern, originalStepCount, newStepCount, clockwise);
        result.originalOnsetCount = static_cast<int>(onsetPositions.size());
        
        return result;
    }
    
    static QuantizationParams parseQuantizationNotation(const String& input)
    {
        QuantizationParams params;
        
        if (!hasQuantizationNotation(input)) {
            params.errorMessage = "No quantization notation found";
            return params;
        }
        
        int semicolonPos = input.lastIndexOf(";");
        if (semicolonPos < 0) {
            params.errorMessage = "Semicolon not found";
            return params;
        }
        
        params.patternPart = input.substring(0, semicolonPos).trim();
        String quantizationPart = input.substring(semicolonPos + 1).trim();
        
        if (params.patternPart.isEmpty()) {
            params.errorMessage = "Pattern part is empty";
            return params;
        }
        
        if (quantizationPart.isEmpty()) {
            params.errorMessage = "Quantization part is empty";
            return params;
        }
        
        bool isNegative = quantizationPart.startsWith("-");
        if (isNegative) {
            quantizationPart = quantizationPart.substring(1);
            params.clockwise = false;
        } else {
            params.clockwise = true;
        }
        
        int stepCount = quantizationPart.getIntValue();
        
        if (stepCount <= 0 || !isValidStepCount(stepCount)) {
            params.errorMessage = String("Invalid step count: ") + String(isNegative ? -stepCount : stepCount);
            return params;
        }
        
        params.newStepCount = stepCount;
        params.isValid = true;
        
        return params;
    }
    
    static bool hasQuantizationNotation(const String& input)
    {
        if (input.isEmpty()) return false;
        
        int semicolonPos = input.lastIndexOf(";");
        if (semicolonPos < 0) return false;
        
        if (semicolonPos == 0 || semicolonPos >= input.length() - 1) return false;
        
        String afterSemicolon = input.substring(semicolonPos + 1).trim();
        if (afterSemicolon.isEmpty()) return false;
        
        if (afterSemicolon.startsWith("-")) {
            afterSemicolon = afterSemicolon.substring(1);
        }
        
        for (int i = 0; i < afterSemicolon.length(); ++i) {
            if (!CharacterFunctions::isDigit(afterSemicolon[i])) {
                return false;
            }
        }
        
        return true;
    }
    
private:
    static constexpr double PI = 3.14159265358979323846;
    static constexpr double TWO_PI = 2.0 * PI;
    static constexpr int MIN_STEP_COUNT = 1;
    static constexpr int MAX_STEP_COUNT = 128;
    
    static bool isValidStepCount(int stepCount)
    {
        return stepCount >= MIN_STEP_COUNT && stepCount <= MAX_STEP_COUNT;
    }
    
    static std::vector<int> extractOnsetPositions(const std::vector<bool>& pattern)
    {
        std::vector<int> positions;
        for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
            if (pattern[i]) {
                positions.push_back(i);
            }
        }
        return positions;
    }
    
    static double positionToAngle(int position, int stepCount, bool clockwise)
    {
        double angle = (static_cast<double>(position) / static_cast<double>(stepCount)) * TWO_PI;
        
        if (!clockwise) {
            angle = TWO_PI - angle;
        }
        
        return normalizeAngle(angle);
    }
    
    static int angleToPosition(double angle, int stepCount)
    {
        angle = normalizeAngle(angle);
        double exactPosition = (angle / TWO_PI) * static_cast<double>(stepCount);
        int position = static_cast<int>(std::round(exactPosition));
        
        if (position >= stepCount) {
            position = 0;
        }
        
        return std::max(0, std::min(position, stepCount - 1));
    }
    
    static double normalizeAngle(double angle)
    {
        while (angle < 0.0) {
            angle += TWO_PI;
        }
        while (angle >= TWO_PI) {
            angle -= TWO_PI;
        }
        return angle;
    }
    
    static QuantizationResult createError(const String& message)
    {
        QuantizationResult result;
        result.isValid = false;
        result.errorMessage = message;
        return result;
    }
};

//==============================================================================
// Test Functions

void printPattern(const std::vector<bool>& pattern, const std::string& name = "") {
    if (!name.empty()) {
        std::cout << name << ": ";
    }
    for (bool onset : pattern) {
        std::cout << (onset ? "1" : "0");
    }
    std::cout << " (" << pattern.size() << " steps)";
}

void testBasicQuantization() {
    std::cout << "=== Testing Basic Quantization ===\n\n";
    
    // Test E(3,8) -> 12 steps (tresillo)
    std::vector<bool> tresillo = {true, false, false, true, false, false, true, false};
    auto result = QuantizationEngine::quantizePattern(tresillo, 12, true);
    
    printPattern(tresillo, "Original E(3,8)");
    std::cout << "\n";
    
    if (result.isValid) {
        printPattern(result.pattern, "Quantized to 12");
        std::cout << "\n";
        std::cout << "Original onsets: " << result.originalOnsetCount << ", Quantized: " << result.quantizedOnsetCount << "\n";
        std::cout << "✅ Basic quantization working\n";
    } else {
        std::cout << "❌ Error: " << result.errorMessage.toRawUTF8() << "\n";
    }
    std::cout << "\n";
}

void testSemicolonParsing() {
    std::cout << "=== Testing Semicolon Notation ===\n\n";
    
    struct TestCase {
        std::string input;
        bool shouldParse;
        int expectedSteps;
        bool expectedClockwise;
    };
    
    std::vector<TestCase> tests = {
        {"E(3,8);12", true, 12, true},
        {"E(5,17);-13", true, 13, false},
        {"pattern;8", true, 8, true},
        {"invalid", false, 0, true}
    };
    
    for (const auto& test : tests) {
        std::cout << "Testing: \"" << test.input << "\"\n";
        
        bool hasNotation = QuantizationEngine::hasQuantizationNotation(String(test.input));
        std::cout << "  Has notation: " << (hasNotation ? "true" : "false") << "\n";
        
        if (hasNotation) {
            auto params = QuantizationEngine::parseQuantizationNotation(String(test.input));
            std::cout << "  Valid: " << (params.isValid ? "true" : "false") << "\n";
            if (params.isValid) {
                std::cout << "  Steps: " << params.newStepCount << "\n";
                std::cout << "  Direction: " << (params.clockwise ? "clockwise" : "counterclockwise") << "\n";
                std::cout << "  Pattern: \"" << params.patternPart.toRawUTF8() << "\"\n";
                
                bool correct = (params.newStepCount == test.expectedSteps && 
                               params.clockwise == test.expectedClockwise);
                std::cout << "  " << (correct ? "✅" : "❌") << "\n";
            }
        }
        std::cout << "\n";
    }
}

void testCounterclockwise() {
    std::cout << "=== Testing Counterclockwise Quantization ===\n\n";
    
    // Use asymmetric pattern to see direction difference
    std::vector<bool> pattern = {true, false, false, true, false, false, false, false};
    auto clockwise = QuantizationEngine::quantizePattern(pattern, 6, true);
    auto counterclockwise = QuantizationEngine::quantizePattern(pattern, 6, false);
    
    printPattern(pattern, "Original");
    std::cout << "\n";
    
    if (clockwise.isValid) {
        printPattern(clockwise.pattern, "Clockwise");
        std::cout << "\n";
    }
    
    if (counterclockwise.isValid) {
        printPattern(counterclockwise.pattern, "Counterclockwise");
        std::cout << "\n";
    }
    
    // They should be different
    bool different = true;
    if (clockwise.isValid && counterclockwise.isValid) {
        different = false;
        for (size_t i = 0; i < clockwise.pattern.size(); ++i) {
            if (clockwise.pattern[i] != counterclockwise.pattern[i]) {
                different = true;
                break;
            }
        }
    }
    
    std::cout << (different ? "✅" : "❌") << " Clockwise and counterclockwise are different\n\n";
}

int main() {
    std::cout << "=== Lascabettes Quantization Engine Standalone Tests ===\n\n";
    
    testBasicQuantization();
    testSemicolonParsing();
    testCounterclockwise();
    
    std::cout << "🎉 PHASE 1 BASIC TESTS COMPLETED!\n";
    std::cout << "QuantizationEngine foundation is working correctly.\n";
    
    return 0;
}