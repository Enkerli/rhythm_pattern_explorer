#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class MockString {
private:
    std::string str;
public:
    MockString(const std::string& s) : str(s) {}
    MockString(const char* s) : str(s) {}
    
    MockString toLowerCase() const {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return MockString(lower);
    }
    
    size_t length() const { return str.length(); }
    char operator[](size_t i) const { return str[i]; }
    bool operator==(const std::string& other) const { return str == other; }
    MockString operator+=(const std::string& other) { str += other; return *this; }
    std::string toStdString() const { return str; }
    
    static MockString charToString(char c) {
        return MockString(std::string(1, c));
    }
};

std::vector<bool> parseMorse(const MockString& morseStr)
{
    std::vector<bool> pattern;
    
    MockString processed = morseStr.toLowerCase();
    std::cout << "Input: '" << processed.toStdString() << "'" << std::endl;
    
    if (processed == "sos")
        processed = MockString("...---...");
    else if (processed == "cq")
        processed = MockString("-.-.--.-");
    else {
        MockString morseCode("");
        bool hasValidLetters = false;
        
        std::cout << "Processing each character:" << std::endl;
        for (size_t i = 0; i < processed.length(); ++i) {
            char letter = processed[i];
            MockString letterMorse("");
            
            switch (letter) {
                case 'a': letterMorse = MockString(".-"); break;
                case 'l': letterMorse = MockString(".-.."); break;
                default: 
                    letterMorse = MockString::charToString(letter);
                    break;
            }
            
            std::cout << "  '" << letter << "' -> '" << letterMorse.toStdString() << "'";
            
            if (letterMorse.toStdString() \!= MockString::charToString(letter).toStdString()) {
                hasValidLetters = true;
                std::cout << " (converted)";
            } else {
                std::cout << " (kept as-is)";
            }
            
            morseCode += letterMorse.toStdString();
            
            // Add space between letters (except for last letter)
            if (i < processed.length() - 1 && letterMorse.toStdString() \!= MockString::charToString(letter).toStdString()) {
                morseCode += " ";
                std::cout << " + space";
            }
            std::cout << std::endl;
        }
        
        if (hasValidLetters) {
            processed = morseCode;
        }
    }
    
    std::cout << "Final morse string: '" << processed.toStdString() << "'" << std::endl;
    std::cout << "Converting to pattern:" << std::endl;
    
    for (size_t i = 0; i < processed.length(); ++i)
    {
        char c = processed[i];
        std::cout << "  '" << c << "' -> ";
        if (c == '.')
        {
            pattern.push_back(true);
            std::cout << "1 (dot)";
        }
        else if (c == '-')
        {
            pattern.push_back(true);
            pattern.push_back(false);
            std::cout << "10 (dash+rest)";
        }
        else if (c == ' ')
        {
            pattern.push_back(false);
            std::cout << "0 (space)";
        }
        std::cout << std::endl;
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
    auto result = parseMorse(MockString("al"));
    std::cout << "\nFinal pattern: " << patternToString(result) << std::endl;
    std::cout << "Expected:      11011011" << std::endl;
    return 0;
}
EOF < /dev/null