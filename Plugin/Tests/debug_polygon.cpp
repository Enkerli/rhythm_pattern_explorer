#include <iostream>
#include <vector>
#include <cmath>

std::vector<bool> parsePolygon(int sides, int offset, int totalSteps = 0)
{
    if (totalSteps == 0) 
        totalSteps = sides;
    
    std::vector<bool> pattern(totalSteps, false);
    
    std::cout << "parsePolygon(" << sides << ", " << offset << ", " << totalSteps << ")" << std::endl;
    
    // Place polygon vertices with proper spacing
    for (int i = 0; i < sides; ++i)
    {
        // Calculate position with floating point for accuracy, then round
        double exactPos = (double)(i * totalSteps) / sides;
        int pos = ((int)std::round(exactPos) + offset) % totalSteps;
        if (pos < 0) pos += totalSteps; // Handle negative modulo
        
        std::cout << "  Vertex " << i << ": exactPos=" << exactPos << ", pos=" << pos << std::endl;
        
        if (pos >= 0 && pos < totalSteps)
            pattern[pos] = true;
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
    auto triangle = parsePolygon(3, 0);
    std::cout << "Result: " << patternToString(triangle) << std::endl;
    
    auto square = parsePolygon(4, 0);
    std::cout << "Result: " << patternToString(square) << std::endl;
    
    return 0;
}
EOF < /dev/null