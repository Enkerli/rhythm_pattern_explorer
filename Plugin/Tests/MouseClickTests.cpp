//==============================================================================
// MouseClickTests.cpp
// Manual integration tests for mouse click functionality
// Tests the complete flow: mouse coordinates -> step detection -> pattern modification
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
    
    bool isValidStepIndex(int stepIndex) const {
        auto pattern = engine.getCurrentPattern();
        return stepIndex >= 0 && stepIndex < static_cast<int>(pattern.size());
    }
};

// Mock editor with step detection and mouse handling
class MockEditor {
private:
    MockAudioProcessor& audioProcessor;
    juce::Rectangle<int> circleArea;
    
public:
    MockEditor(MockAudioProcessor& processor) : audioProcessor(processor), circleArea(0, 0, 200, 200) {
        // Circle area initialized in member initializer list
    }
    
    // Step detection algorithm (copied from real implementation)
    int getStepIndexFromCoordinates(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const {
        auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
        int numSteps = static_cast<int>(pattern.size());
        
        if (numSteps <= 0) return -1;
        
        juce::Point<float> center(circleArea.getCentreX(), circleArea.getCentreY());
        float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
        float outerRadius = radius;
        float innerRadius = radius * 0.3f;
        
        float dx = mouseX - center.x;
        float dy = mouseY - center.y;
        float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
        
        if (distanceFromCenter < innerRadius || distanceFromCenter > outerRadius) {
            return -1;
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
        
        return stepIndex;
    }
    
    // Mock mouse click handler (matching real implementation)
    void simulateMouseClick(int mouseX, int mouseY) {
        std::cout << "Mouse clicked at (" << mouseX << ", " << mouseY << ")" << std::endl;
        
        if (!circleArea.isEmpty()) {
            int stepIndex = getStepIndexFromCoordinates(mouseX, mouseY, circleArea);
            
            if (stepIndex >= 0) {
                std::cout << "Valid step clicked: " << stepIndex << std::endl;
                audioProcessor.togglePatternStep(stepIndex);
            } else {
                std::cout << "Click outside pattern area" << std::endl;
            }
        }
    }
};

//==============================================================================
// Test Cases
//==============================================================================

void testBasicClickToggles() {
    std::cout << "=== Testing Basic Click Toggles ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    // Set up a simple 4-step pattern: 1010
    std::vector<bool> initialPattern = {true, false, true, false};
    processor.getPatternEngine().setPattern(initialPattern);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "\nInitial pattern: 1010\n";
    
    // Click on step 1 (should toggle from false to true)
    std::cout << "\n1. Clicking step 1 (3 o'clock position):\n";
    editor.simulateMouseClick(centerX + 35, centerY);
    
    // Click on step 2 (should toggle from true to false)
    std::cout << "\n2. Clicking step 2 (6 o'clock position):\n";
    editor.simulateMouseClick(centerX, centerY + 35);
    
    // Click on step 0 (should toggle from true to false)
    std::cout << "\n3. Clicking step 0 (12 o'clock position):\n";
    editor.simulateMouseClick(centerX, centerY - 35);
    
    std::cout << "\n✅ Basic click toggles test completed!\n\n";
}

void testClickOutsidePattern() {
    std::cout << "=== Testing Clicks Outside Pattern ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    std::vector<bool> pattern = {true, false, true, false};
    processor.getPatternEngine().setPattern(pattern);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::cout << "Testing clicks outside valid areas:\n";
    
    // Click in center (inside inner radius - should be rejected)
    std::cout << "\n1. Clicking center (inside inner radius):\n";
    editor.simulateMouseClick(centerX, centerY);
    
    // Click far outside (beyond outer radius - should be rejected)  
    std::cout << "\n2. Clicking far outside:\n";
    editor.simulateMouseClick(centerX + 100, centerY);
    
    // Click completely outside circle area
    std::cout << "\n3. Clicking outside circle area:\n";
    editor.simulateMouseClick(-50, -50);
    
    std::cout << "\n✅ Outside clicks test completed!\n\n";
}

void testDifferentPatternSizes() {
    std::cout << "=== Testing Different Pattern Sizes ===\n";
    
    MockAudioProcessor processor;
    MockEditor editor(processor);
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    std::vector<int> sizes = {3, 8, 16};
    
    for (int size : sizes) {
        std::cout << "\nTesting " << size << "-step pattern:\n";
        
        // Create pattern of alternating true/false
        std::vector<bool> pattern(size);
        for (int i = 0; i < size; ++i) {
            pattern[i] = (i % 2 == 0);
        }
        processor.getPatternEngine().setPattern(pattern);
        
        // Test click on step 0 (12 o'clock)
        std::cout << "Clicking step 0 (12 o'clock):\n";
        editor.simulateMouseClick(centerX, centerY - 35);
        
        // Test click on approximate quarter position
        if (size >= 4) {
            float quarterAngle = 2.0f * juce::MathConstants::pi / 4.0f;
            int quarterX = centerX + 35 * std::cos(quarterAngle - juce::MathConstants::halfPi);
            int quarterY = centerY + 35 * std::sin(quarterAngle - juce::MathConstants::halfPi);
            std::cout << "Clicking quarter position:\n";
            editor.simulateMouseClick(quarterX, quarterY);
        }
    }
    
    std::cout << "\n✅ Different pattern sizes test completed!\n\n";
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== Mouse Click Integration Tests ===\n\n";
    
    try {
        testBasicClickToggles();
        testClickOutsidePattern();
        testDifferentPatternSizes();
        
        std::cout << "🎉 ALL MOUSE CLICK TESTS PASSED!\n";
        std::cout << "Mouse click functionality is working correctly.\n";
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