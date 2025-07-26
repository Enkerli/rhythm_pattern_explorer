//==============================================================================
// StepDetectionTests.cpp
// Unit tests for coordinate-to-step conversion algorithm
// Verifies accuracy of mouse click detection in circular pattern layout
//==============================================================================

#include <iostream>
#include <cassert>
#include <cmath>
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
    };
    
    namespace MathConstants {
        static constexpr float pi = 3.14159265359f;
        static constexpr float halfPi = pi / 2.0f;
    }
}

// Minimal mock for testing
class MockPatternEngine {
private:
    std::vector<bool> pattern;
public:
    void setPattern(const std::vector<bool>& p) { pattern = p; }
    const std::vector<bool>& getCurrentPattern() const { return pattern; }
};

class MockAudioProcessor {
private:
    MockPatternEngine engine;
public:
    MockPatternEngine& getPatternEngine() { return engine; }
    const MockPatternEngine& getPatternEngine() const { return engine; }
};

// Step detection algorithm implementation (copied from main code)
class StepDetector {
private:
    MockAudioProcessor processor;
    
public:
    void setTestPattern(const std::vector<bool>& pattern) {
        processor.getPatternEngine().setPattern(pattern);
    }
    
    int getStepIndexFromCoordinates(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const
    {
        auto pattern = processor.getPatternEngine().getCurrentPattern();
        int numSteps = static_cast<int>(pattern.size());
        
        if (numSteps <= 0) return -1; // Invalid pattern
        
        // Calculate circle dimensions (matching drawPatternCircle logic)
        juce::Point<float> center(circleArea.getCentreX(), circleArea.getCentreY());
        
        // Use same radius calculations as drawPatternCircle
        float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
        float outerRadius = radius;
        float innerRadius = radius * 0.3f; // 30% inner radius for donut effect
        
        // Calculate distance from center
        float dx = mouseX - center.x;
        float dy = mouseY - center.y;
        float distanceFromCenter = std::sqrt(dx * dx + dy * dy);
        
        // Check if click is within the ring (between inner and outer radius)
        if (distanceFromCenter < innerRadius || distanceFromCenter > outerRadius) {
            return -1; // Outside clickable area
        }
        
        // Calculate angle from center
        // atan2 returns angle in range [-π, π], we need [0, 2π]
        float angleFromCenter = std::atan2(dy, dx);
        if (angleFromCenter < 0) {
            angleFromCenter += 2.0f * juce::MathConstants::pi;
        }
        
        // Adjust for 12 o'clock alignment (matching drawPatternCircle)
        // The pattern starts at 12 o'clock (north), so we need to offset by +π/2
        // This converts from standard math coordinates to our 12-o'clock-first system
        float adjustedAngle = angleFromCenter + juce::MathConstants::halfPi;
        if (adjustedAngle >= 2.0f * juce::MathConstants::pi) {
            adjustedAngle -= 2.0f * juce::MathConstants::pi;
        }
        
        // Calculate slice angle and step index
        float sliceAngle = 2.0f * juce::MathConstants::pi / numSteps;
        
        // Add half slice offset to ensure we're detecting the center of slices correctly
        adjustedAngle += sliceAngle * 0.5f;
        if (adjustedAngle >= 2.0f * juce::MathConstants::pi) {
            adjustedAngle -= 2.0f * juce::MathConstants::pi;
        }
        int stepIndex = static_cast<int>(adjustedAngle / sliceAngle);
        
        // Clamp to valid range
        if (stepIndex < 0) stepIndex = 0;
        if (stepIndex >= numSteps) stepIndex = numSteps - 1;
        
        return stepIndex;
    }
    
    bool isCoordinateInCircleArea(int mouseX, int mouseY, juce::Rectangle<int> circleArea) const
    {
        return getStepIndexFromCoordinates(mouseX, mouseY, circleArea) >= 0;
    }
};

//==============================================================================
// Test Cases
//==============================================================================

void testBasicStepDetection() {
    std::cout << "Testing basic step detection...\n";
    
    StepDetector detector;
    detector.setTestPattern({true, false, true, false}); // 4-step pattern
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200); // 200x200 circle area
    
    // Test center coordinates - should be outside (inner radius)
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    assert(detector.getStepIndexFromCoordinates(centerX, centerY, circleArea) == -1);
    
    // Test 12 o'clock position (should be step 0)
    int topX = centerX;
    int topY = centerY - 35; // Within ring radius
    int step = detector.getStepIndexFromCoordinates(topX, topY, circleArea);
    std::cout << "  12 o'clock position: step " << step << " (expected: 0)\n";
    assert(step == 0);
    
    // Test 3 o'clock position (should be step 1 for 4-step pattern)
    int rightX = centerX + 35;
    int rightY = centerY;
    step = detector.getStepIndexFromCoordinates(rightX, rightY, circleArea);
    std::cout << "  3 o'clock position: step " << step << " (expected: 1)\n";
    assert(step == 1);
    
    // Test 6 o'clock position (should be step 2)
    int bottomX = centerX;
    int bottomY = centerY + 35;
    step = detector.getStepIndexFromCoordinates(bottomX, bottomY, circleArea);
    std::cout << "  6 o'clock position: step " << step << " (expected: 2)\n";
    assert(step == 2);
    
    // Test 9 o'clock position (should be step 3)
    int leftX = centerX - 35;
    int leftY = centerY;
    step = detector.getStepIndexFromCoordinates(leftX, leftY, circleArea);
    std::cout << "  9 o'clock position: step " << step << " (expected: 3)\n";
    assert(step == 3);
    
    std::cout << "✅ Basic step detection passed!\n\n";
}

void testVariousPatternSizes() {
    std::cout << "Testing various pattern sizes...\n";
    
    StepDetector detector;
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    // Test different pattern sizes
    std::vector<int> patternSizes = {3, 5, 8, 12, 16, 32};
    
    for (int size : patternSizes) {
        std::vector<bool> pattern(size, true); // All onsets for simplicity
        detector.setTestPattern(pattern);
        
        // Test 12 o'clock (should always be step 0)
        int step = detector.getStepIndexFromCoordinates(centerX, centerY - 35, circleArea);
        std::cout << "  Pattern size " << size << ": 12 o'clock -> step " << step << "\n";
        assert(step == 0);
        
        // Test approximate quarter positions
        float quarterAngle = 2.0f * juce::MathConstants::pi / 4.0f;
        int quarterStep = size / 4;
        
        int quarterX = centerX + 35 * std::cos(quarterAngle - juce::MathConstants::halfPi);
        int quarterY = centerY + 35 * std::sin(quarterAngle - juce::MathConstants::halfPi);
        int detectedStep = detector.getStepIndexFromCoordinates(quarterX, quarterY, circleArea);
        
        std::cout << "    Quarter position -> step " << detectedStep << " (expected ~" << quarterStep << ")\n";
        // Allow some tolerance for rounding
        assert(std::abs(detectedStep - quarterStep) <= 1);
    }
    
    std::cout << "✅ Various pattern sizes passed!\n\n";
}

void testBoundaryConditions() {
    std::cout << "Testing boundary conditions...\n";
    
    StepDetector detector;
    detector.setTestPattern({true, true, true, true, true, true, true, true}); // 8-step pattern
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    // Calculate actual radii
    float radius = std::min(circleArea.getWidth(), circleArea.getHeight()) * 0.4f;
    float outerRadius = radius;
    float innerRadius = radius * 0.3f;
    
    // Test just inside inner radius (should be invalid)
    int innerX = centerX + (innerRadius - 1);
    int innerY = centerY;
    assert(detector.getStepIndexFromCoordinates(innerX, innerY, circleArea) == -1);
    std::cout << "  Just inside inner radius: correctly rejected\n";
    
    // Test just outside inner radius (should be valid)
    int validInnerX = centerX + (innerRadius + 1);
    int validInnerY = centerY;
    assert(detector.getStepIndexFromCoordinates(validInnerX, validInnerY, circleArea) >= 0);
    std::cout << "  Just outside inner radius: correctly accepted\n";
    
    // Test just inside outer radius (should be valid)
    int validOuterX = centerX + (outerRadius - 1);
    int validOuterY = centerY;
    assert(detector.getStepIndexFromCoordinates(validOuterX, validOuterY, circleArea) >= 0);
    std::cout << "  Just inside outer radius: correctly accepted\n";
    
    // Test just outside outer radius (should be invalid)
    int outerX = centerX + (outerRadius + 1);
    int outerY = centerY;
    assert(detector.getStepIndexFromCoordinates(outerX, outerY, circleArea) == -1);
    std::cout << "  Just outside outer radius: correctly rejected\n";
    
    // Test completely outside circle area
    assert(detector.getStepIndexFromCoordinates(-50, -50, circleArea) == -1);
    assert(detector.getStepIndexFromCoordinates(300, 300, circleArea) == -1);
    std::cout << "  Far outside coordinates: correctly rejected\n";
    
    std::cout << "✅ Boundary conditions passed!\n\n";
}

void testAngleAccuracy() {
    std::cout << "Testing angle calculation accuracy...\n";
    
    StepDetector detector;
    detector.setTestPattern({true, true, true, true, true, true, true, true}); // 8-step pattern
    
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    int centerX = circleArea.getCentreX();
    int centerY = circleArea.getCentreY();
    
    float radius = 35; // Middle of ring
    
    // Test precise angles for 8-step pattern
    for (int expectedStep = 0; expectedStep < 8; ++expectedStep) {
        // Calculate expected angle for this step - matching step marker positioning
        // This should match the centerAngle calculation from drawPatternCircle
        float sliceAngle = 2.0f * juce::MathConstants::pi / 8.0f;
        float centerAngle = (expectedStep * sliceAngle) - juce::MathConstants::halfPi;
        
        // Use the center angle directly (this matches how step markers are positioned)
        float angle = centerAngle;
        
        int testX = centerX + radius * std::cos(angle);
        int testY = centerY + radius * std::sin(angle);
        
        int detectedStep = detector.getStepIndexFromCoordinates(testX, testY, circleArea);
        std::cout << "  Step " << expectedStep << " angle: " << (centerAngle * 180.0f / juce::MathConstants::pi) 
                  << "° detected step " << detectedStep << " at (" << testX << "," << testY << ")\n";
        
        // For debugging: show exact angle calculations
        if (detectedStep != expectedStep) {
            std::cout << "    DEBUG: centerAngle=" << centerAngle << ", sliceAngle=" << sliceAngle << "\n";
            std::cout << "    DEBUG: Expected " << expectedStep << ", got " << detectedStep << "\n";
        }
        
        assert(detectedStep == expectedStep);
    }
    
    std::cout << "✅ Angle accuracy passed!\n\n";
}

void testEdgeCasePatterns() {
    std::cout << "Testing edge case patterns...\n";
    
    StepDetector detector;
    juce::Rectangle<int> circleArea(0, 0, 200, 200);
    
    // Test empty pattern
    detector.setTestPattern({});
    assert(detector.getStepIndexFromCoordinates(100, 65, circleArea) == -1);
    std::cout << "  Empty pattern: correctly rejected\n";
    
    // Test single step pattern
    detector.setTestPattern({true});
    int step = detector.getStepIndexFromCoordinates(100, 65, circleArea); // 12 o'clock
    assert(step == 0);
    std::cout << "  Single step pattern: step " << step << " (expected: 0)\n";
    
    // Test two step pattern
    detector.setTestPattern({true, false});
    step = detector.getStepIndexFromCoordinates(100, 65, circleArea); // 12 o'clock
    assert(step == 0);
    step = detector.getStepIndexFromCoordinates(135, 100, circleArea); // 6 o'clock
    assert(step == 1);
    std::cout << "  Two step pattern: steps detected correctly\n";
    
    std::cout << "✅ Edge case patterns passed!\n\n";
}

//==============================================================================
// Main Test Runner
//==============================================================================

int main() {
    std::cout << "=== Step Detection Algorithm Tests ===\n\n";
    
    try {
        testBasicStepDetection();
        testVariousPatternSizes();
        testBoundaryConditions();
        testAngleAccuracy();
        testEdgeCasePatterns();
        
        std::cout << "🎉 ALL TESTS PASSED! Step detection algorithm is working correctly.\n";
        std::cout << "Ready for integration into the main plugin.\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ TEST FAILED: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ TEST FAILED: Unknown error occurred\n";
        return 1;
    }
}