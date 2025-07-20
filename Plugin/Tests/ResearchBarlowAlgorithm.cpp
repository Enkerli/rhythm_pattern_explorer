/*
  ==============================================================================
    Research and Test Correct Barlow Algorithm
    
    Based on Clarence Barlow's indispensability theory:
    - Position 0 (downbeat) has highest indispensability
    - Indispensability decreases based on metric position
    - Uses prime factorization and musical hierarchy
  ==============================================================================
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>

// True Barlow indispensability calculation based on Clarence Barlow's theory
double calculateTrueBarlowIndispensability(int position, int stepCount)
{
    if (position == 0) return 1.0; // Downbeat always most indispensable
    
    // Barlow's formula: indispensability = sum of (p-1)/p for prime factors p of (stepCount/gcd(position, stepCount))
    int gcd_val = std::gcd(position, stepCount);
    int quotient = stepCount / gcd_val;
    
    double indispensability = 0.0;
    
    // Find prime factors of quotient and apply Barlow's formula
    for (int p = 2; p * p <= quotient; ++p) {
        while (quotient % p == 0) {
            indispensability += (double)(p - 1) / p;
            quotient /= p;
        }
    }
    if (quotient > 1) {
        indispensability += (double)(quotient - 1) / quotient;
    }
    
    return indispensability;
}

// Alternative: Metric position based (simpler approach)
double calculateMetricIndispensability(int position, int stepCount)
{
    if (position == 0) return 1.0; // Downbeat
    
    double positionRatio = (double)position / stepCount;
    
    // Based on metric hierarchy in 4/4 time
    if (fmod(positionRatio * 4, 1.0) < 0.001) return 0.75;     // Beat positions
    if (fmod(positionRatio * 8, 1.0) < 0.001) return 0.5;      // 8th note positions
    if (fmod(positionRatio * 16, 1.0) < 0.001) return 0.25;    // 16th note positions
    
    return 0.1; // Syncopated positions
}

// Hierarchical approach - fill positions by decreasing indispensability
std::vector<bool> generateBarlowHierarchical(int onsets, int steps)
{
    std::vector<bool> pattern(steps, false);
    
    // Calculate indispensability for all positions
    std::vector<std::pair<int, double>> positions;
    for (int i = 0; i < steps; ++i) {
        double indispensability = calculateTrueBarlowIndispensability(i, steps);
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

// Test metric-based approach
std::vector<bool> generateBarlowMetric(int onsets, int steps)
{
    std::vector<bool> pattern(steps, false);
    
    // Calculate indispensability for all positions
    std::vector<std::pair<int, double>> positions;
    for (int i = 0; i < steps; ++i) {
        double indispensability = calculateMetricIndispensability(i, steps);
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

void printIndispensability(const std::string& name, int steps, double (*calc)(int, int)) {
    std::cout << name << " indispensability for " << steps << " steps:" << std::endl;
    for (int i = 0; i < steps; ++i) {
        std::cout << "  Position " << i << ": " << calc(i, steps) << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    std::cout << "=== Barlow Algorithm Research ===" << std::endl;
    std::cout << "Testing different approaches for B(3,8)" << std::endl;
    std::cout << "User expects: 10001001" << std::endl;
    std::cout << "CLAUDE.md says: 10001010" << std::endl << std::endl;
    
    // Test indispensability calculations
    printIndispensability("True Barlow", 8, calculateTrueBarlowIndispensability);
    printIndispensability("Metric-based", 8, calculateMetricIndispensability);
    
    // Generate patterns
    auto truePattern = generateBarlowHierarchical(3, 8);
    auto metricPattern = generateBarlowMetric(3, 8);
    
    std::cout << "True Barlow B(3,8):   " << patternToString(truePattern) << std::endl;
    std::cout << "Metric-based B(3,8):  " << patternToString(metricPattern) << std::endl;
    
    // Test a few more cases
    std::cout << std::endl << "=== Additional Test Cases ===" << std::endl;
    auto b28_true = generateBarlowHierarchical(2, 8);
    auto b28_metric = generateBarlowMetric(2, 8);
    std::cout << "B(2,8) True Barlow:   " << patternToString(b28_true) << std::endl;
    std::cout << "B(2,8) Metric-based:  " << patternToString(b28_metric) << std::endl;
    
    auto b37_true = generateBarlowHierarchical(3, 7);
    auto b37_metric = generateBarlowMetric(3, 7);
    std::cout << "B(3,7) True Barlow:   " << patternToString(b37_true) << std::endl;
    std::cout << "B(3,7) Metric-based:  " << patternToString(b37_metric) << std::endl;
    
    return 0;
}