#include <iostream>
#include <vector>
#include <cassert>

/**
 * AccentSequence Unit Tests
 * 
 * Comprehensive test suite for the new robust accent system.
 * These tests define the expected behavior before implementation.
 */

// Forward declaration - will be implemented after tests
class AccentSequence {
public:
    AccentSequence(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern);
    bool isAccentedAtStep(uint32_t stepInSequence) const;
    std::vector<bool> getAccentMapForCycle(uint32_t cycleStartStep) const;
    uint32_t getSequenceLength() const;
    bool isValid() const;
    
private:
    std::vector<bool> accentTable;
    uint32_t rhythmCycleLength;
    uint32_t accentCycleLength;
    uint32_t fullSequenceLength;
};

// Test helper functions
void testSimplePattern();
void testPolymetricPattern(); 
void testEdgeCases();
void testComplexPattern();
void testPerformance();

int main() {
    std::cout << "=== AccentSequence Unit Tests ===" << std::endl;
    
    try {
        testSimplePattern();
        std::cout << "✅ Simple pattern tests passed" << std::endl;
        
        testPolymetricPattern();
        std::cout << "✅ Polymetric pattern tests passed" << std::endl;
        
        testEdgeCases();
        std::cout << "✅ Edge case tests passed" << std::endl;
        
        testComplexPattern();
        std::cout << "✅ Complex pattern tests passed" << std::endl;
        
        testPerformance();
        std::cout << "✅ Performance tests passed" << std::endl;
        
        std::cout << std::endl << "🎉 ALL TESTS PASSED!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cout << "❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}

void testSimplePattern() {
    // Test Case: {10} on E(3,4) -> 1010 rhythm, 10 accent
    std::vector<bool> rhythm = {1,0,1,0}; // E(3,4) -> 4 steps, 2 onsets
    std::vector<bool> accent = {1,0};     // {10} -> accent every 2nd onset
    
    AccentSequence sequence(rhythm, accent);
    assert(sequence.isValid());
    
    // Expected sequence: rhythm has onsets at steps 0,2
    // Accent pattern {10} -> accent 1st onset (not 2nd)
    // Step 0: onset 0 % 2 = 0 -> accent[0] = 1 -> ACCENTED
    // Step 2: onset 1 % 2 = 1 -> accent[1] = 0 -> NOT ACCENTED
    
    assert(sequence.isAccentedAtStep(0) == true);   // First onset is accented
    assert(sequence.isAccentedAtStep(1) == false);  // No onset at step 1
    assert(sequence.isAccentedAtStep(2) == false);  // Second onset is not accented
    assert(sequence.isAccentedAtStep(3) == false);  // No onset at step 3
    
    // Test cycle repeats correctly
    assert(sequence.isAccentedAtStep(4) == true);   // Same as step 0
    assert(sequence.isAccentedAtStep(6) == false);  // Same as step 2
}

void testPolymetricPattern() {
    // Test Case: {10} on E(5,8) -> 10110110 rhythm, 10 accent
    std::vector<bool> rhythm = {1,0,1,1,0,1,1,0}; // 8 steps, 5 onsets at 0,2,3,5,6
    std::vector<bool> accent = {1,0};             // {10} -> accent every 2nd onset
    
    AccentSequence sequence(rhythm, accent);
    assert(sequence.isValid());
    
    // LCM(5 onsets, 2 accents) = 10 onsets = 16 steps (2 rhythm cycles)
    assert(sequence.getSequenceLength() == 16);
    
    // First cycle onsets: 0,2,3,5,6 -> accent positions 0,2,4 (onsets 0,2,4)
    assert(sequence.isAccentedAtStep(0) == true);   // Onset 0: 0%2=0 -> accent[0]=1
    assert(sequence.isAccentedAtStep(2) == false);  // Onset 1: 1%2=1 -> accent[1]=0  
    assert(sequence.isAccentedAtStep(3) == true);   // Onset 2: 2%2=0 -> accent[0]=1
    assert(sequence.isAccentedAtStep(5) == false);  // Onset 3: 3%2=1 -> accent[1]=0
    assert(sequence.isAccentedAtStep(6) == true);   // Onset 4: 4%2=0 -> accent[0]=1
    
    // Second cycle onsets: 8,10,11,13,14 -> accent positions 1,3 (onsets 5,7,9)
    assert(sequence.isAccentedAtStep(8) == false);  // Onset 5: 5%2=1 -> accent[1]=0
    assert(sequence.isAccentedAtStep(10) == true);  // Onset 6: 6%2=0 -> accent[0]=1
    assert(sequence.isAccentedAtStep(11) == false); // Onset 7: 7%2=1 -> accent[1]=0
    assert(sequence.isAccentedAtStep(13) == true);  // Onset 8: 8%2=0 -> accent[0]=1
    assert(sequence.isAccentedAtStep(14) == false); // Onset 9: 9%2=1 -> accent[1]=0
    
    // Pattern repeats after LCM
    assert(sequence.isAccentedAtStep(16) == sequence.isAccentedAtStep(0));
    assert(sequence.isAccentedAtStep(18) == sequence.isAccentedAtStep(2));
}

void testEdgeCases() {
    // Test Case 1: E(1,1) - Single step pattern
    std::vector<bool> singleStep = {1};
    std::vector<bool> singleAccent = {1};
    
    AccentSequence singleSequence(singleStep, singleAccent);
    assert(singleSequence.isValid());
    assert(singleSequence.getSequenceLength() == 1);
    assert(singleSequence.isAccentedAtStep(0) == true);
    assert(singleSequence.isAccentedAtStep(1) == true);  // Should repeat
    
    // Test Case 2: Pattern with no onsets
    std::vector<bool> noOnsets = {0,0,0,0};
    std::vector<bool> anyAccent = {1,0};
    
    AccentSequence noOnsetSequence(noOnsets, anyAccent);
    assert(noOnsetSequence.isValid());
    for (int i = 0; i < 10; i++) {
        assert(noOnsetSequence.isAccentedAtStep(i) == false);  // No onsets = no accents
    }
    
    // Test Case 3: Empty accent pattern
    std::vector<bool> normalRhythm = {1,0,1,0};
    std::vector<bool> noAccents = {};
    
    AccentSequence noAccentSequence(normalRhythm, noAccents);
    assert(noAccentSequence.isValid());
    for (int i = 0; i < 10; i++) {
        assert(noAccentSequence.isAccentedAtStep(i) == false);  // No accent pattern = no accents
    }
}

void testComplexPattern() {
    // Test Case: {101} on E(7,13) -> Complex LCM calculation
    std::vector<bool> rhythm;  // E(7,13) - 7 onsets in 13 steps
    rhythm.resize(13, false);
    
    // Euclidean distribution of 7 onsets in 13 steps
    // Mathematical distribution: approximately every 13/7 ≈ 1.86 steps
    std::vector<int> onsetPositions = {0, 2, 4, 6, 7, 9, 11};  // E(7,13) pattern
    for (int pos : onsetPositions) {
        rhythm[pos] = true;
    }
    
    std::vector<bool> accent = {1,0,1};  // {101} - 3-step accent pattern
    
    AccentSequence complexSequence(rhythm, accent);
    assert(complexSequence.isValid());
    
    // LCM(7 onsets, 3 accents) = 21 onsets = 39 steps (3 rhythm cycles)
    assert(complexSequence.getSequenceLength() == 39);
    
    // Verify first few accent positions
    // Onset 0 at step 0: 0%3=0 -> accent[0]=1 -> ACCENTED
    // Onset 1 at step 2: 1%3=1 -> accent[1]=0 -> NOT ACCENTED  
    // Onset 2 at step 4: 2%3=2 -> accent[2]=1 -> ACCENTED
    // Onset 3 at step 6: 3%3=0 -> accent[0]=1 -> ACCENTED
    
    assert(complexSequence.isAccentedAtStep(0) == true);   // Onset 0
    assert(complexSequence.isAccentedAtStep(2) == false);  // Onset 1
    assert(complexSequence.isAccentedAtStep(4) == true);   // Onset 2  
    assert(complexSequence.isAccentedAtStep(6) == true);   // Onset 3
}

void testPerformance() {
    // Test with large pattern to ensure O(1) lookup performance
    std::vector<bool> largeRhythm(100, false);
    for (int i = 0; i < 100; i += 3) {
        largeRhythm[i] = true;  // Every 3rd step has onset
    }
    
    std::vector<bool> largeAccent(17, false);  // Prime number to maximize LCM
    largeAccent[0] = true;
    largeAccent[5] = true;
    largeAccent[11] = true;
    
    AccentSequence largeSequence(largeRhythm, largeAccent);
    assert(largeSequence.isValid());
    
    // Test many lookups - should be fast O(1) operations
    for (int i = 0; i < 10000; i++) {
        bool accented = largeSequence.isAccentedAtStep(i);
        // Just verify the call doesn't crash - timing would be measured externally
        (void)accented;  // Suppress unused variable warning
    }
}

// Placeholder implementation for compilation - will be replaced with real implementation
AccentSequence::AccentSequence(const std::vector<bool>& rhythmPattern, const std::vector<bool>& accentPattern) {
    // Placeholder - real implementation will follow
    rhythmCycleLength = rhythmPattern.size();
    accentCycleLength = accentPattern.size();
    fullSequenceLength = rhythmCycleLength;  // Simplified for compilation
    accentTable.resize(fullSequenceLength, false);
}

bool AccentSequence::isAccentedAtStep(uint32_t stepInSequence) const {
    // Placeholder - will fail tests intentionally until real implementation
    return false;
}

std::vector<bool> AccentSequence::getAccentMapForCycle(uint32_t cycleStartStep) const {
    // Placeholder
    return std::vector<bool>(rhythmCycleLength, false);
}

uint32_t AccentSequence::getSequenceLength() const {
    return fullSequenceLength;
}

bool AccentSequence::isValid() const {
    return !accentTable.empty();
}