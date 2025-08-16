#include <iostream>
#include <vector>
#include <string>

// Simple test without JUCE dependencies
int main() {
    std::cout << "=== Simple Accent Pattern Analysis ===" << std::endl;
    
    // Based on our analysis:
    // E(5,8) should generate: 10110110 (onsets at steps 0,2,3,5,6)
    // E(1,3) should generate: 100 (accent every 3rd onset)
    
    std::vector<bool> mainPattern = {true, false, true, true, false, true, true, false}; // E(5,8)
    std::vector<bool> accentPattern = {true, false, false}; // E(1,3)
    
    std::cout << "Main pattern E(5,8): ";
    for (bool bit : mainPattern) std::cout << (bit ? "1" : "0");
    std::cout << " (onsets at steps: ";
    for (int i = 0; i < mainPattern.size(); i++) {
        if (mainPattern[i]) std::cout << i << " ";
    }
    std::cout << ")" << std::endl;
    
    std::cout << "Accent pattern E(1,3): ";
    for (bool bit : accentPattern) std::cout << (bit ? "1" : "0");
    std::cout << " (accent every 3rd onset)" << std::endl;
    
    std::cout << "\n=== Expected Accent Distribution ===" << std::endl;
    std::cout << "Should accent onsets: 0, 3, 6, 9, 12, ..." << std::endl;
    std::cout << "Which correspond to steps: 0, 5, 6, ?, ?, ..." << std::endl;
    
    std::cout << "\n=== Simulating Multiple Cycles ===" << std::endl;
    int globalOnsetCounter = 0;
    
    for (int cycle = 0; cycle < 3; cycle++) {
        std::cout << "\nCycle " << (cycle + 1) << ":" << std::endl;
        
        for (int step = 0; step < mainPattern.size(); step++) {
            if (mainPattern[step]) { // This is an onset
                int accentPosition = globalOnsetCounter % accentPattern.size();
                bool isAccented = accentPattern[accentPosition];
                
                std::cout << "  Step " << step << " (global onset " << globalOnsetCounter 
                          << ") -> accent pos " << accentPosition 
                          << " -> " << (isAccented ? "ACCENT" : "normal") << std::endl;
                
                globalOnsetCounter++;
            }
        }
    }
    
    std::cout << "\n=== Analysis ===" << std::endl;
    std::cout << "If working correctly:" << std::endl;
    std::cout << "- Cycle 1: accent on step 0 (onset 0), step 5 (onset 3)" << std::endl;
    std::cout << "- Cycle 2: accent on step 6 (onset 6)" << std::endl;
    std::cout << "- Cycle 3: accent on step 2 (onset 9)" << std::endl;
    std::cout << "\nIf broken (single accent per cycle):" << std::endl;
    std::cout << "- Would show only one accent per 8-step cycle, moving position" << std::endl;
    
    return 0;
}