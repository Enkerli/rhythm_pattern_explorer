// Simple test without JUCE dependencies
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

// Copy the calculateBarlowIndispensability function from PatternUtils.cpp
double calculateBarlowIndispensability(int position, int stepCount)
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
    int gcd_value = std::gcd(position, stepCount);
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

std::vector<bool> generateBarlowPattern(int onsets, int steps)
{
    std::vector<bool> pattern(steps, false);
    
    // Calculate indispensability for all positions
    std::vector<std::pair<int, double>> positions;
    for (int i = 0; i < steps; ++i) {
        double indispensability = calculateBarlowIndispensability(i, steps);
        positions.push_back({i, indispensability});
    }
    
    // Sort by indispensability (highest first), then by position for stability
    std::sort(positions.begin(), positions.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            if (std::abs(a.second - b.second) < 1e-10) {
                return a.first < b.first; // Stable sort by position
            }
            return a.second > b.second;
        });
    
    // Place onsets at most indispensable positions
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
    std::cout << "=== Current PatternUtils Barlow Implementation Analysis ===" << std::endl;
    
    // Test the specific B(3,8) case
    auto pattern = generateBarlowPattern(3, 8);
    
    std::cout << "B(3,8) current result: " << patternToString(pattern) << std::endl;
    std::cout << "README.md expected:     10001010" << std::endl;
    std::cout << "Tests expected:         10001001" << std::endl;
    
    // Test indispensability values for each position
    std::cout << "\nIndispensability values for 8-step pattern:" << std::endl;
    std::vector<std::pair<int, double>> positions;
    for (int i = 0; i < 8; ++i) {
        double indispensability = calculateBarlowIndispensability(i, 8);
        positions.push_back({i, indispensability});
        std::cout << "Position " << i << ": " << indispensability << std::endl;
    }
    
    // Sort to show the order of selection
    std::sort(positions.begin(), positions.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
            if (std::abs(a.second - b.second) < 1e-10) {
                return a.first < b.first;
            }
            return a.second > b.second;
        });
    
    std::cout << "\nSelection order (highest indispensability first):" << std::endl;
    for (int i = 0; i < positions.size(); ++i) {
        std::cout << i+1 << ". Position " << positions[i].first << " (indispensability: " << positions[i].second << ")" << std::endl;
    }
    
    std::cout << "\nFor B(3,8), selecting top 3 positions: " << positions[0].first << ", " << positions[1].first << ", " << positions[2].first << std::endl;
    
    return 0;
}