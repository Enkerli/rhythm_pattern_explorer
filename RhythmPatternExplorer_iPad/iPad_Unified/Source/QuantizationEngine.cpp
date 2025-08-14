/*
  ==============================================================================

    Lascabettes-Style Quantization Engine Implementation
    Port of webapp pattern-processing.js quantization algorithms

  ==============================================================================
*/

#include "QuantizationEngine.h"

//==============================================================================
// Main quantization function - core algorithm
QuantizationEngine::QuantizationResult QuantizationEngine::quantizePattern(
    const std::vector<bool>& originalPattern, 
    int newStepCount, 
    bool clockwise)
{
    // Validate inputs
    if (originalPattern.empty()) {
        return createError("Original pattern is empty");
    }
    
    if (!isValidStepCount(newStepCount)) {
        return createError("Invalid step count: " + juce::String(newStepCount) + 
                          " (must be between " + juce::String(MIN_STEP_COUNT) + 
                          " and " + juce::String(MAX_STEP_COUNT) + ")");
    }
    
    int originalStepCount = static_cast<int>(originalPattern.size());
    
    // Edge case: same step count
    if (originalStepCount == newStepCount) {
        return QuantizationResult(originalPattern, originalStepCount, newStepCount, clockwise);
    }
    
    // Extract onset positions from original pattern
    std::vector<int> onsetPositions = extractOnsetPositions(originalPattern);
    
    // Edge case: no onsets
    if (onsetPositions.empty()) {
        std::vector<bool> emptyPattern(newStepCount, false);
        return QuantizationResult(emptyPattern, originalStepCount, newStepCount, clockwise);
    }
    
    // Core algorithm: Angular mapping
    std::set<int> quantizedPositions; // Use set to prevent conflicts
    
    for (int originalPos : onsetPositions) {
        // Convert position to angular position
        double angle = positionToAngle(originalPos, originalStepCount, clockwise);
        
        // Convert angle back to position in new grid
        int newPos = angleToPosition(angle, newStepCount);
        
        // Add to set (automatically handles conflicts)
        quantizedPositions.insert(newPos);
    }
    
    // Create quantized pattern
    std::vector<bool> quantizedPattern(newStepCount, false);
    for (int pos : quantizedPositions) {
        if (pos >= 0 && pos < newStepCount) {
            quantizedPattern[pos] = true;
        }
    }
    
    // Create result with metadata
    QuantizationResult result(quantizedPattern, originalStepCount, newStepCount, clockwise);
    result.originalOnsetCount = static_cast<int>(onsetPositions.size());
    
    return result;
}

//==============================================================================
// Parse semicolon notation
QuantizationEngine::QuantizationParams QuantizationEngine::parseQuantizationNotation(const juce::String& input)
{
    QuantizationParams params;
    
    if (!hasQuantizationNotation(input)) {
        params.errorMessage = "No quantization notation found";
        return params;
    }
    
    // Find semicolon position
    int semicolonPos = input.lastIndexOf(";");
    if (semicolonPos < 0) {
        params.errorMessage = "Semicolon not found";
        return params;
    }
    
    // Split pattern and quantization parts
    params.patternPart = input.substring(0, semicolonPos).trim();
    juce::String quantizationPart = input.substring(semicolonPos + 1).trim();
    
    if (params.patternPart.isEmpty()) {
        params.errorMessage = "Pattern part is empty";
        return params;
    }
    
    if (quantizationPart.isEmpty()) {
        params.errorMessage = "Quantization part is empty";
        return params;
    }
    
    // Parse quantization value
    bool isNegative = quantizationPart.startsWith("-");
    if (isNegative) {
        quantizationPart = quantizationPart.substring(1); // Remove minus sign
        params.clockwise = false;
    } else {
        params.clockwise = true;
    }
    
    // Convert to integer
    int stepCount = quantizationPart.getIntValue();
    
    if (stepCount <= 0 || !isValidStepCount(stepCount)) {
        params.errorMessage = "Invalid step count: " + juce::String(isNegative ? -stepCount : stepCount);
        return params;
    }
    
    params.newStepCount = stepCount;
    params.isValid = true;
    
    return params;
}

//==============================================================================
// Check for quantization notation
bool QuantizationEngine::hasQuantizationNotation(const juce::String& input)
{
    if (input.isEmpty()) return false;
    
    // Must contain semicolon
    int semicolonPos = input.lastIndexOf(";");
    if (semicolonPos < 0) return false;
    
    // Must have content before and after semicolon
    if (semicolonPos == 0 || semicolonPos >= input.length() - 1) return false;
    
    // Check if part after semicolon looks like a number
    juce::String afterSemicolon = input.substring(semicolonPos + 1).trim();
    if (afterSemicolon.isEmpty()) return false;
    
    // Allow negative numbers (for counterclockwise)
    if (afterSemicolon.startsWith("-")) {
        afterSemicolon = afterSemicolon.substring(1);
    }
    
    // Check if remaining part is numeric
    for (int i = 0; i < afterSemicolon.length(); ++i) {
        if (!juce::CharacterFunctions::isDigit(afterSemicolon[i])) {
            return false;
        }
    }
    
    return true;
}

//==============================================================================
// Utility functions

bool QuantizationEngine::isValidStepCount(int stepCount)
{
    return stepCount >= MIN_STEP_COUNT && stepCount <= MAX_STEP_COUNT;
}

QuantizationEngine::QualityMetrics QuantizationEngine::calculateQualityMetrics(
    const std::vector<bool>& original,
    const QuantizationResult& quantized)
{
    QualityMetrics metrics;
    
    if (!quantized.isValid || original.empty()) {
        return metrics;
    }
    
    // Calculate onset preservation ratio
    if (quantized.originalOnsetCount > 0) {
        metrics.onsetPreservationRatio = static_cast<double>(quantized.quantizedOnsetCount) / 
                                        static_cast<double>(quantized.originalOnsetCount);
    }
    
    // Calculate density change
    double originalDensity = static_cast<double>(quantized.originalOnsetCount) / 
                            static_cast<double>(quantized.originalStepCount);
    double quantizedDensity = static_cast<double>(quantized.quantizedOnsetCount) / 
                             static_cast<double>(quantized.quantizedStepCount);
    
    if (originalDensity > 0) {
        metrics.densityChange = quantizedDensity / originalDensity;
    }
    
    // Calculate conflict count (onsets lost due to mapping to same position)
    metrics.conflictCount = quantized.originalOnsetCount - quantized.quantizedOnsetCount;
    metrics.hasConflicts = metrics.conflictCount > 0;
    
    return metrics;
}

//==============================================================================
// Private implementation functions

std::vector<int> QuantizationEngine::extractOnsetPositions(const std::vector<bool>& pattern)
{
    std::vector<int> positions;
    positions.reserve(pattern.size()); // Reserve space for efficiency
    
    for (int i = 0; i < static_cast<int>(pattern.size()); ++i) {
        if (pattern[i]) {
            positions.push_back(i);
        }
    }
    
    return positions;
}

double QuantizationEngine::positionToAngle(int position, int stepCount, bool clockwise)
{
    // Convert step position to angular position (0 to 2*PI)
    double angle = (static_cast<double>(position) / static_cast<double>(stepCount)) * TWO_PI;
    
    // Handle direction
    if (!clockwise) {
        // Counterclockwise: invert angle
        angle = TWO_PI - angle;
    }
    
    return normalizeAngle(angle);
}

int QuantizationEngine::angleToPosition(double angle, int stepCount)
{
    // Normalize angle to [0, 2*PI)
    angle = normalizeAngle(angle);
    
    // Convert angle back to step position
    double exactPosition = (angle / TWO_PI) * static_cast<double>(stepCount);
    
    // Round to nearest integer position
    int position = static_cast<int>(std::round(exactPosition));
    
    // Handle wraparound (position might equal stepCount due to rounding)
    if (position >= stepCount) {
        position = 0;
    }
    
    // Ensure position is within bounds
    return std::max(0, std::min(position, stepCount - 1));
}

double QuantizationEngine::normalizeAngle(double angle)
{
    // Normalize angle to [0, 2*PI) range
    while (angle < 0.0) {
        angle += TWO_PI;
    }
    while (angle >= TWO_PI) {
        angle -= TWO_PI;
    }
    return angle;
}

QuantizationEngine::QuantizationResult QuantizationEngine::createError(const juce::String& message)
{
    QuantizationResult result;
    result.isValid = false;
    result.errorMessage = message;
    return result;
}