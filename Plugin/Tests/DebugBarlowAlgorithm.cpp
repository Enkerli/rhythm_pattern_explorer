/*
  ==============================================================================
    Debug Barlow Algorithm Test
    Check what's wrong with B(3,8) pattern generation
  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <algorithm>

// Extracted from PatternUtils.cpp
double calculateBarlowIndispensabilityCurrent(int position, int stepCount)
{
    if (stepCount <= 1) return 1.0;
    
    double sum = 0.0;
    
    // Calculate indispensability based on Barlow's theory
    // Sum of (q-1)/q for all prime factors q where q divides (stepCount-position)
    for (int q = 2; q <= stepCount; ++q)
    {
        if (stepCount % q == 0 && (stepCount - position) % q == 0)
        {
            sum += static_cast<double>(q - 1) / q;
        }
    }
    
    return sum;
}

// Correct Barlow implementation based on CLAUDE.md documentation
double calculateBarlowIndispensabilityCorrect(int position, int stepCount)
{
    if (stepCount <= 1) return 1.0;
    
    // Use metric strength calculation for common step counts
    if (stepCount <= 17) {
        double positionRatio = static_cast<double>(position) / stepCount;
        
        // Calculate metric strength based on position in the measure
        if (positionRatio == 0.0) {
            return 1.0; // Downbeat
        } else if (fmod(positionRatio * 2, 1.0) == 0.0) {
            return 0.75; // Half-note positions
        } else if (fmod(positionRatio * 4, 1.0) == 0.0) {
            return 0.5; // Quarter-note positions  
        } else if (fmod(positionRatio * 8, 1.0) == 0.0) {
            return 0.25; // Eighth-note positions
        } else {
            return 0.1; // Syncopated positions
        }
    } else {
        // For larger step counts, use mathematical indispensability
        return 1.0 / (position + 1); // Simple inverse relationship
    }
}

std::vector<bool> bjorklundAlgorithm(int k, int n)
{
    if (k >= n) {
        return std::vector<bool>(n, true);
    }
    if (k <= 0) {
        return std::vector<bool>(n, false);
    }
    
    std::vector<std::vector<bool>> levels;
    levels.push_back(std::vector<bool>(1, true));
    levels.push_back(std::vector<bool>(1, false));
    
    std::vector<int> counts = {k, n - k};
    
    while (counts[1] > 1) {
        int minCount = std::min(counts[0], counts[1]);
        counts[0] -= minCount;
        counts[1] -= minCount;
        
        std::vector<bool> newLevel = levels[0];
        newLevel.insert(newLevel.end(), levels[1].begin(), levels[1].end());
        levels.insert(levels.begin(), newLevel);
        
        counts.insert(counts.begin(), minCount);
        
        if (counts[0] == 0) {
            counts.erase(counts.begin());
            levels.erase(levels.begin());
        }
        if (counts.size() > 1 && counts[1] == 0) {
            counts.erase(counts.begin() + 1);
            levels.erase(levels.begin() + 1);
        }
    }
    
    std::vector<bool> pattern;
    for (int i = 0; i < counts.size(); ++i) {
        for (int j = 0; j < counts[i]; ++j) {
            pattern.insert(pattern.end(), levels[i].begin(), levels[i].end());
        }
    }
    
    return pattern;
}

std::vector<bool> generateCorrectBarlow(int onsets, int steps)
{
    // Start with empty pattern
    std::vector<bool> pattern(steps, false);
    
    // Calculate indispensability for all positions
    std::vector<std::pair<int, double>> positions;
    for (int i = 0; i < steps; ++i) {
        double indispensability = calculateBarlowIndispensabilityCorrect(i, steps);
        positions.push_back({i, indispensability});
    }
    
    // Sort by indispensability (highest first)
    std::sort(positions.begin(), positions.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            return a.second > b.second;
        });
    
    // Add onsets at the most indispensable positions
    for (int i = 0; i < onsets && i < static_cast<int>(positions.size()); ++i) {
        pattern[positions[i].first] = true;
    }
    
    return pattern;
}

std::string patternToString(const std::vector<bool>& pattern) {
    std::string result;
    for (bool bit : pattern) {
        result += bit ? "1" : "0";
    }
    return result;
}

int main() {
    std::cout << "=== Barlow Algorithm Debug ===" << std::endl;
    std::cout << "Testing B(3,8) - Expected: 10001001, Current: ?" << std::endl << std::endl;
    
    // Test indispensability calculations
    std::cout << "Current indispensability calculation for B(3,8):" << std::endl;
    for (int i = 0; i < 8; ++i) {
        double current = calculateBarlowIndispensabilityCurrent(i, 8);
        std::cout << "Position " << i << ": " << current << std::endl;
    }
    
    std::cout << std::endl << "Correct indispensability calculation for B(3,8):" << std::endl;
    for (int i = 0; i < 8; ++i) {
        double correct = calculateBarlowIndispensabilityCorrect(i, 8);
        std::cout << "Position " << i << ": " << correct << std::endl;
    }
    
    // Generate patterns
    auto correctPattern = generateCorrectBarlow(3, 8);
    std::cout << std::endl << "Correct B(3,8): " << patternToString(correctPattern) << std::endl;
    
    // Compare with Euclidean for reference
    auto euclideanPattern = bjorklundAlgorithm(3, 8);
    std::cout << "E(3,8) reference: " << patternToString(euclideanPattern) << std::endl;
    
    return 0;
}