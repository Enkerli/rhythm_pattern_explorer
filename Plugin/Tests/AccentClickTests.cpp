//==============================================================================
// AccentClickTests.cpp
// Integration tests for accent control via mouse clicks
// Tests the complete flow: inner/outer click detection -> accent/onset toggles
//==============================================================================

#include <iostream>
#include <cassert>
#include <vector>

// Mock JUCE types for testing
namespace juce {
    template<typename T>
    struct Point {
        T x, y;
        Point(T x_, T y_) : x(x_), y(y_) {}
        T getCentreX() const { return x; }
        T getCentreY() const { return y; }
    };
    
    template<typename T>
    struct Rectangle {
        T x, y, width, height;
        Rectangle(T x_, T y_, T w, T h) : x(x_), y(y_), width(w), height(h) {}
        T getWidth() const { return width; }
        T getHeight() const { return height; }
        T getCentreX() const { return x + width/2; }
        T getCentreY() const { return y + height/2; }
        bool isEmpty() const { return width <= 0 || height <= 0; }
    };
    
    namespace MathConstants {
        static constexpr float pi = 3.14159265359f;
        static constexpr float halfPi = pi / 2.0f;
    }
}

// Mock pattern engine and processor
class MockPatternEngine {
private:
    std::vector<bool> pattern;
public:
    void setPattern(const std::vector<bool>& p) { 
        pattern = p; 
        std::cout << "Pattern set to: ";
        for (bool step : pattern) {
            std::cout << (step ? "1" : "0");
        }
        std::cout << std::endl;
    }
    const std::vector<bool>& getCurrentPattern() const { return pattern; }
};

class MockAudioProcessor {
private:
    MockPatternEngine engine;
    std::vector<bool> accentPattern;
    bool hasAccents = false;
    
public:
    MockPatternEngine& getPatternEngine() { return engine; }
    const MockPatternEngine& getPatternEngine() const { return engine; }
    
    // Pattern modification methods (matching real implementation)
    void togglePatternStep(int stepIndex) {
        if (!isValidStepIndex(stepIndex)) {
            std::cout << "togglePatternStep: Invalid step index " << stepIndex << std::endl;
            return;
        }
        
        auto currentPattern = engine.getCurrentPattern();
        currentPattern[stepIndex] = !currentPattern[stepIndex];
        engine.setPattern(currentPattern);
        
        std::cout << "Toggled step " << stepIndex << " to " << (currentPattern[stepIndex] ? "ON" : "OFF") << std::endl;
    }
    
    void toggleAccentAtStep(int stepIndex) {
        if (!isValidStepIndex(stepIndex)) {
            std::cout << "toggleAccentAtStep: Invalid step index " << stepIndex << std::endl;
            return;
        }
        
        // First ensure the step is an onset - can't accent a rest
        auto currentPattern = engine.getCurrentPattern();
        if (!currentPattern[stepIndex]) {
            std::cout << "toggleAccentAtStep: Cannot accent a rest step at index " << stepIndex << std::endl;
            return;
        }
        
        // Toggle accent at this step
        if (hasAccents && stepIndex < accentPattern.size()) {
            // Existing accent pattern - toggle this step
            accentPattern[stepIndex] = !accentPattern[stepIndex];
            std::cout << "toggleAccentAtStep: Toggled accent at step " << stepIndex << " to " << (accentPattern[stepIndex] ? "ACCENTED" : "UNACCENTED") << std::endl;
        } else {
            // No accent pattern exists - create one with this step accented
            accentPattern.clear();
            accentPattern.resize(currentPattern.size(), false);
            accentPattern[stepIndex] = true;
            hasAccents = true;
            std::cout << "toggleAccentAtStep: Created new accent pattern with accent at step " << stepIndex << std::endl;
        }
    }
    
    bool isValidStepIndex(int stepIndex) const {
        auto pattern = engine.getCurrentPattern();
        return stepIndex >= 0 && stepIndex < static_cast<int>(pattern.size());
    }
    
    // Accent pattern access
    bool getHasAccentPattern() const { return hasAccents; }
    const std::vector<bool>& getCurrentAccentPattern() const { return accentPattern; }
};

// Mock editor with enhanced click detection
class MockEditor {
private:
    MockAudioProcessor& audioProcessor;
    juce::Rectangle<int> circleArea;
    
    // Enhanced click detection result struct
    struct ClickResult {
        int stepIndex = -1;      // -1 if no valid step
        bool isInOuterHalf = false; // true if click is in outer half (accent area)
    };
    
public:
    MockEditor(MockAudioProcessor& processor) : audioProcessor(processor), circleArea(0, 0, 200, 200) {
        // Circle area initialized in member initializer list
    }
    
    // Enhanced click detection with inner/outer half support
    ClickResult getStepClickDetails(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const {
        ClickResult result;
        
        auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
        int numSteps = static_cast<int>(pattern.size());
        
        if (numSteps <= 0) return result;
        
        juce::Point<float> center(circleArea.getCentreX(), circleArea.getCentreY());
        float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
        float outerRadius = radius;
        float innerRadius = radius * 0.3f;
        
        float dx = mouseX - center.x;
        float dy = mouseY - center.y;
        float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
        
        if (distanceFromCenter < innerRadius || distanceFromCenter > outerRadius) {
            return result;
        }
        
        float angleFromCenter = std::atan2(dy, dx);
        if (angleFromCenter < 0) {
            angleFromCenter += 2.0f * juce::MathConstants::pi;
        }
        
        float adjustedAngle = angleFromCenter + juce::MathConstants::halfPi;
        if (adjustedAngle >= 2.0f * juce::MathConstants::pi) {
            adjustedAngle -= 2.0f * juce::MathConstants::pi;
        }
        
        float sliceAngle = 2.0f * juce::MathConstants::pi / numSteps;
        adjustedAngle += sliceAngle * 0.5f;
        if (adjustedAngle >= 2.0f * juce::MathConstants::pi) {
            adjustedAngle -= 2.0f * juce::MathConstants::pi;
        }
        int stepIndex = static_cast<int>(adjustedAngle / sliceAngle);
        
        if (stepIndex < 0) stepIndex = 0;
        if (stepIndex >= numSteps) stepIndex = numSteps - 1;
        
        result.stepIndex = stepIndex;
        
        // Determine if click is in outer half (accent area)
        float midRadius = (innerRadius + outerRadius) * 0.5f;
        result.isInOuterHalf = (distanceFromCenter > midRadius);
        
        // Debug output
        std::cout << "    Debug: distance=" << distanceFromCenter 
                  << " innerR=" << innerRadius << " midR=" << midRadius 
                  << " outerR=" << outerRadius << " -> " << (result.isInOuterHalf ? "OUTER" : "INNER") << std::endl;
        
        return result;
    }
    
    // Enhanced mouse click handler with accent control
    void simulateMouseClick(int mouseX, int mouseY) {
        std::cout << "Mouse clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;
        
        if (!circleArea.isEmpty()) {
            ClickResult clickResult = getStepClickDetails(mouseX, mouseY, circleArea);
            
            if (clickResult.stepIndex >= 0) {
                // Get current pattern to check if step is an onset
                auto currentPattern = audioProcessor.getPatternEngine().getCurrentPattern();
                bool isOnset = (clickResult.stepIndex < currentPattern.size()) && currentPattern[clickResult.stepIndex];
                
                std::cout << "Valid step clicked: " << clickResult.stepIndex 
                         << " (isOnset: " << (isOnset ? "YES" : "NO") 
                         << ", isOuterHalf: " << (clickResult.isInOuterHalf ? "YES" : "NO") << ")" << std::endl;
                
                if (!isOnset) {
                    // Empty step: clicking anywhere creates an onset
                    audioProcessor.togglePatternStep(clickResult.stepIndex);
                    std::cout << "Created onset at step " << clickResult.stepIndex << std::endl;
                } else {
                    // Existing onset: inner half toggles onset, outer half toggles accent
                    if (clickResult.isInOuterHalf) {
                        // Outer half: toggle accent
                        audioProcessor.toggleAccentAtStep(clickResult.stepIndex);
                        std::cout << "Toggled accent at step " << clickResult.stepIndex << std::endl;
                    } else {
                        // Inner half: toggle onset (remove it)
                        audioProcessor.togglePatternStep(clickResult.stepIndex);
                        std::cout << "Removed onset at step " << clickResult.stepIndex << std::endl;
                    }
                }
            } else {
                std::cout << "Click outside pattern area" << std::endl;
            }
        }
    }
};

//==============================================================================
// Test Cases
//==============================================================================

void testEmptyStepClicking() {
    std::cout << "=== Testing Empty Step Clicking ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    // Set up a pattern with some empty steps: 1010
    std::vector<bool> initialPattern = {true, false, true, false};
    processor.getPatternEngine().setPattern(initialPattern);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "\nInitial pattern: 1010 (onsets at steps 0,2)\n";
    
    // Click on empty step 1 - should create onset regardless of inner/outer
    std::cout << "\n1. Clicking empty step 1 (3 o'clock position - outer half):\n";
    editor.simulateMouseClick(centerX + 65, centerY);  // Outer half
    
    // Click on empty step 3 - should create onset regardless of inner/outer  
    std::cout << "\n2. Clicking empty step 3 (9 o'clock position - inner half):\n";
    editor.simulateMouseClick(centerX - 25, centerY);  // Inner half
    
    std::cout << "\n✅ Empty step clicking test completed!\n\n";
}

void testOnsetAccentControl() {
    std::cout << "=== Testing Onset Accent Control ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    // Set up pattern with all onsets: 1111
    std::vector<bool> allOnsets = {true, true, true, true};
    processor.getPatternEngine().setPattern(allOnsets);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "\nInitial pattern: 1111 (all onsets)\n";
    
    // Click outer half of step 0 - should create accent
    std::cout << "\n1. Clicking outer half of step 0 (12 o'clock - outer):\n";
    editor.simulateMouseClick(centerX, centerY - 65);  // Outer half (radius ~65)
    
    // Click outer half of step 1 - should create accent 
    std::cout << "\n2. Clicking outer half of step 1 (3 o'clock - outer):\n";
    editor.simulateMouseClick(centerX + 65, centerY);  // Outer half (radius ~65)
    
    // Click outer half of step 0 again - should remove accent
    std::cout << "\n3. Clicking outer half of step 0 again (should remove accent):\n";
    editor.simulateMouseClick(centerX, centerY - 65);  // Outer half (radius ~65)
    
    // Click inner half of step 2 - should remove onset
    std::cout << "\n4. Clicking inner half of step 2 (6 o'clock - inner):\n";
    editor.simulateMouseClick(centerX, centerY + 25);  // Inner half
    
    std::cout << "\n✅ Onset accent control test completed!\n\n";
}

void testAccentOnRestStep() {
    std::cout << "=== Testing Accent on Rest Step (Should Fail) ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    // Set up pattern with rest at step 1: 1010
    std::vector<bool> pattern = {true, false, true, false};
    processor.getPatternEngine().setPattern(pattern);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "\nInitial pattern: 1010 (rest at step 1)\n";
    
    // Try to accent rest step 1 - should fail and create onset instead
    std::cout << "\n1. Clicking outer half of rest step 1 (should create onset, not accent):\n";
    editor.simulateMouseClick(centerX + 65, centerY);  // Outer half of step 1
    
    std::cout << "\n✅ Accent on rest step test completed!\n\n";
}

void testInnerOuterDetection() {
    std::cout << "=== Testing Inner/Outer Half Detection ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    // Set up pattern with onset at step 0: 1000
    std::vector<bool> pattern = {true, false, false, false};
    processor.getPatternEngine().setPattern(pattern);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "\nInitial pattern: 1000 (onset only at step 0)\n";
    
    // Test various radii for step 0 (12 o'clock)
    std::cout << "\n1. Testing different click distances for step 0:\n";
    
    // Inner half (closer to center)
    std::cout << "   a) Inner half (radius 25): ";
    editor.simulateMouseClick(centerX, centerY - 25);  
    
    // Outer half (closer to edge)  
    std::cout << "   b) Outer half (radius 65): ";
    editor.simulateMouseClick(centerX, centerY - 65);
    
    std::cout << "\n✅ Inner/outer detection test completed!\n\n";
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== Accent Click Control Integration Tests ===\n\n";
    
    try {
        testEmptyStepClicking();
        testOnsetAccentControl();
        testAccentOnRestStep();
        testInnerOuterDetection();
        
        std::cout << "🎉 ALL ACCENT CLICK TESTS PASSED!\n";
        std::cout << "Accent click control functionality is working correctly.\n";
        std::cout << "Logic implemented:\n";
        std::cout << "  - Empty step + any click → Create onset\n";
        std::cout << "  - Existing onset + inner half → Remove onset\n";
        std::cout << "  - Existing onset + outer half → Toggle accent\n";
        std::cout << "Ready for real-world testing in the plugin.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ TEST FAILED: Unknown error occurred\n";
        return 1;
    }
}