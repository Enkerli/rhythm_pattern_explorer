/*
  Test to verify E(5,8)+2 progressive offset behavior
*/

#include <iostream>
#include <vector>
#include <string>

// Mock JUCE String for testing
class MockString {
private:
    std::string str;
public:
    MockString(const std::string& s) : str(s) {}
    MockString(const char* s) : str(s) {}
    
    std::string toStdString() const { return str; }
    bool contains(const std::string& substr) const {
        return str.find(substr) != std::string::npos;
    }
    int lastIndexOf(const std::string& substr) const {
        size_t pos = str.rfind(substr);
        return (pos != std::string::npos) ? static_cast<int>(pos) : -1;
    }
    MockString substring(int start, int end = -1) const {
        if (end == -1) return MockString(str.substr(start));
        return MockString(str.substr(start, end - start));
    }
    MockString trim() const {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) return MockString("");
        size_t end = str.find_last_not_of(" \t\r\n");
        return MockString(str.substr(start, end - start + 1));
    }
    bool containsOnly(const std::string& chars) const {
        return str.find_first_not_of(chars) == std::string::npos;
    }
    bool isNotEmpty() const { return !str.empty(); }
};

// Euclidean algorithm implementation
std::vector<bool> euclideanRhythm(int onsets, int steps) {
    std::vector<bool> pattern(steps, false);
    
    if (onsets >= steps) {
        // More onsets than steps - fill all
        std::fill(pattern.begin(), pattern.end(), true);
        return pattern;
    }
    
    if (onsets <= 0) {
        // No onsets
        return pattern;
    }
    
    // Bjorklund's algorithm
    std::vector<std::vector<bool>> sequences;
    
    // Initialize with onsets (1s) and rests (0s)
    for (int i = 0; i < onsets; ++i) {
        sequences.push_back({true});
    }
    for (int i = 0; i < steps - onsets; ++i) {
        sequences.push_back({false});
    }
    
    // Bjorklund redistribution
    while (sequences.size() > 1) {
        int minCount = std::min(onsets, steps - onsets);
        int maxCount = std::max(onsets, steps - onsets);
        
        if (minCount <= 1) break;
        
        // Combine sequences
        std::vector<std::vector<bool>> newSequences;
        
        for (int i = 0; i < minCount; ++i) {
            std::vector<bool> combined = sequences[i];
            combined.insert(combined.end(), sequences[maxCount + i].begin(), sequences[maxCount + i].end());
            newSequences.push_back(combined);
        }
        
        // Add remaining sequences
        for (int i = minCount; i < maxCount; ++i) {
            newSequences.push_back(sequences[i]);
        }
        
        sequences = newSequences;
        onsets = minCount;
        steps = maxCount - minCount;
        
        if (steps <= 0) break;
    }
    
    // Flatten to final pattern
    int index = 0;
    for (const auto& seq : sequences) {
        for (bool bit : seq) {
            if (index < static_cast<int>(pattern.size())) {
                pattern[index++] = bit;
            }
        }
    }
    
    return pattern;
}

// Pattern rotation
std::vector<bool> rotatePattern(const std::vector<bool>& pattern, int offset) {
    if (pattern.empty()) return pattern;
    
    int size = static_cast<int>(pattern.size());
    offset = offset % size;
    if (offset < 0) offset += size;
    
    std::vector<bool> rotated(size);
    for (int i = 0; i < size; ++i) {
        rotated[i] = pattern[(i + offset) % size];
    }
    return rotated;
}

// Convert pattern to string
std::string patternToString(const std::vector<bool>& pattern) {
    std::string result;
    for (bool bit : pattern) {
        result += bit ? "1" : "0";
    }
    return result;
}

// Count onsets
int countOnsets(const std::vector<bool>& pattern) {
    return std::count(pattern.begin(), pattern.end(), true);
}

// Progressive offset detection (from PluginProcessor.cpp logic)
bool detectProgressiveOffset(const MockString& pattern) {
    if (!pattern.contains("+") || pattern.lastIndexOf("+") <= 0) return false;
    
    int lastPlusIndex = pattern.lastIndexOf("+");
    MockString afterPlus = pattern.substring(lastPlusIndex + 1).trim();
    return afterPlus.containsOnly("0123456789-") && afterPlus.isNotEmpty();
}

int main() {
    std::cout << "=== PROGRESSIVE OFFSET TEST ===" << std::endl;
    std::cout << "Testing E(5,8)+2 pattern behavior\n" << std::endl;
    
    // Test base pattern E(5,8)
    auto basePattern = euclideanRhythm(5, 8);
    std::cout << "E(5,8) base pattern: " << patternToString(basePattern) << std::endl;
    std::cout << "E(5,8) onset count: " << countOnsets(basePattern) << std::endl;
    std::cout << "E(5,8) expected: 10110110 (from documentation)" << std::endl;
    
    // Test progressive offset detection
    MockString testInput("E(5,8)+2");
    bool isProgressive = detectProgressiveOffset(testInput);
    std::cout << "\nProgressive offset detection for 'E(5,8)+2': " << (isProgressive ? "YES" : "NO") << std::endl;
    
    if (isProgressive) {
        int lastPlusIndex = testInput.lastIndexOf("+");
        MockString afterPlus = testInput.substring(lastPlusIndex + 1).trim();
        std::cout << "Offset value: " << afterPlus.toStdString() << std::endl;
    }
    
    // Test progressive sequence
    std::cout << "\nProgressive sequence for E(5,8)+2:" << std::endl;
    for (int step = 0; step < 6; ++step) {
        int totalOffset = step * 2; // +2 per trigger
        auto rotatedPattern = rotatePattern(basePattern, totalOffset);
        std::cout << "Step " << step << " (offset +" << totalOffset << "): " 
                  << patternToString(rotatedPattern) << " | Onsets: " 
                  << countOnsets(rotatedPattern) << std::endl;
    }
    
    // Check what pattern produces 11110110
    std::cout << "\nAnalyzing the reported pattern 11110110:" << std::endl;
    std::vector<bool> reportedPattern = {true, true, true, true, false, true, true, false};
    std::cout << "Pattern: " << patternToString(reportedPattern) << std::endl;
    std::cout << "Onsets: " << countOnsets(reportedPattern) << std::endl;
    std::cout << "This looks like E(6,8): " << patternToString(euclideanRhythm(6, 8)) << std::endl;
    
    return 0;
}