void RhythmPatternExplorerAudioProcessorEditor::drawPatternCircle(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto pattern = audioProcessor.getPatternEngine().getCurrentPattern();
    if (pattern.empty())
        return;
    
    auto center = bounds.getCentre();
    float maxRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.3f;
    float innerRadius = maxRadius * 0.3f;  // Larger inner hole for better donut effect
    float outerRadius = maxRadius;
    float markerRadius = maxRadius * 0.85f;
    
    int numSteps = static_cast<int>(pattern.size());
    int currentStep = audioProcessor.getCurrentStep();
    bool isPlaying = audioProcessor.isCurrentlyPlaying();
    
    // Force output to see debug info (use std::cout instead of DBG)
    std::cout << "DRAW: Step=" << currentStep << " Playing=" << (isPlaying ? "YES" : "NO") 
              << " Size=" << numSteps << std::endl;
    
    // Draw the background circle first
    g.setColour(juce::Colour(0xff2d3748));
    g.fillEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2);
    
    // Draw each slice using the most basic approach possible
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float startAngle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        float endAngle = startAngle + sliceAngle;
        
        // Only draw onset slices
        if (pattern[i])
        {
            // Create a proper filled pie sector using multiple triangles
            juce::Path slice;
            
            // Start from center
            slice.startNewSubPath(center.x, center.y);
            
            // Add multiple points around the arc to create a smooth sector
            int numSegments = jmax(8, int(sliceAngle * 20)); // More segments for larger slices
            for (int seg = 0; seg <= numSegments; ++seg)
            {
                float angle = startAngle + (sliceAngle * seg / numSegments);
                float x = center.x + outerRadius * std::cos(angle);
                float y = center.y + outerRadius * std::sin(angle);
                slice.lineTo(x, y);
            }
            
            // Close back to center
            slice.closeSubPath();
            
            g.setColour(juce::Colour(0xff48bb78));  // Green for onsets
            g.fillPath(slice);
            
            std::cout << "Drew slice " << i << " from " << startAngle << " to " << endAngle << std::endl;
        }
    }
    
    // Draw inner circle to create donut effect AFTER all slices
    if (innerRadius > 0)
    {
        g.setColour(juce::Colour(0xff2d3748)); // Same as background
        g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                     innerRadius * 2, innerRadius * 2);
    }
    
    // Draw playback highlighting OVER everything
    if (isPlaying && currentStep >= 0 && currentStep < numSteps)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float startAngle = (currentStep * sliceAngle) - juce::MathConstants<float>::halfPi;
        float endAngle = startAngle + sliceAngle;
        
        // Create highlight sector same way as regular slices
        juce::Path highlightSlice;
        highlightSlice.startNewSubPath(center.x, center.y);
        
        int numSegments = jmax(8, int(sliceAngle * 20));
        for (int seg = 0; seg <= numSegments; ++seg)
        {
            float angle = startAngle + (sliceAngle * seg / numSegments);
            float x = center.x + outerRadius * std::cos(angle);
            float y = center.y + outerRadius * std::sin(angle);
            highlightSlice.lineTo(x, y);
        }
        highlightSlice.closeSubPath();
        
        g.setColour(juce::Colour(0xffff6b35));  // Bright orange highlight
        g.fillPath(highlightSlice);
        
        std::cout << "HIGHLIGHT step " << currentStep << std::endl;
        
        // Redraw inner circle on top of highlight
        if (innerRadius > 0)
        {
            g.setColour(juce::Colour(0xff2d3748));
            g.fillEllipse(center.x - innerRadius, center.y - innerRadius, 
                         innerRadius * 2, innerRadius * 2);
        }
    }
    
    // Draw slice separator lines AFTER filling
    g.setColour(juce::Colour(0xff4a5568));
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        
        float lineStartX = center.x + innerRadius * std::cos(angle);
        float lineStartY = center.y + innerRadius * std::sin(angle);
        float lineEndX = center.x + outerRadius * std::cos(angle);
        float lineEndY = center.y + outerRadius * std::sin(angle);
        g.drawLine(lineStartX, lineStartY, lineEndX, lineEndY, 1.5f);
    }
    
    // Draw outer and inner circle outlines
    g.setColour(juce::Colour(0xff4a5568));
    g.drawEllipse(center.x - outerRadius, center.y - outerRadius, outerRadius * 2, outerRadius * 2, 2.0f);
    if (innerRadius > 0)
        g.drawEllipse(center.x - innerRadius, center.y - innerRadius, innerRadius * 2, innerRadius * 2, 2.0f);
    
    // Draw step markers INSIDE the available space
    for (int i = 0; i < numSteps; ++i)
    {
        float sliceAngle = 2.0f * juce::MathConstants<float>::pi / numSteps;
        float angle = (i * sliceAngle) - juce::MathConstants<float>::halfPi;
        
        // Position marker at the CENTER of each slice
        float centerAngle = angle + (sliceAngle * 0.5f);
        float x = center.x + markerRadius * std::cos(centerAngle);
        float y = center.y + markerRadius * std::sin(centerAngle);
        
        // Draw step marker circles
        g.setColour(juce::Colour(0xff4a5568));
        g.fillEllipse(x - 8, y - 8, 16, 16);
        
        // Draw step numbers
        juce::String stepNumber = juce::String(i);
        g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        
        // Center the text in the marker
        juce::Rectangle<float> textBounds(x - 8, y - 6, 16, 12);
        g.drawText(stepNumber, textBounds, juce::Justification::centred);
    }
}