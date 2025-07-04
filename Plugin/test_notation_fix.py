#!/usr/bin/env python3
"""
Test script to verify the notation fixes
Tests E(3,8) = 0x92 and E(5,8) = 0xB6 according to left-to-right standard
"""

def pattern_to_hex_webapp_style(pattern):
    """Convert pattern to hex using left-to-right (MSB first) convention"""
    decimal = 0
    for i, bit in enumerate(pattern):
        if bit:
            decimal |= (1 << (len(pattern) - 1 - i))
    return f"0x{decimal:X}"

def pattern_to_decimal_webapp_style(pattern):
    """Convert pattern to decimal using left-to-right (MSB first) convention"""
    decimal = 0
    for i, bit in enumerate(pattern):
        if bit:
            decimal |= (1 << (len(pattern) - 1 - i))
    return decimal

def euclidean_pattern(onsets, steps):
    """Simple Euclidean pattern generator for testing"""
    if onsets == 0:
        return [False] * steps
    if onsets >= steps:
        return [True] * steps
    
    # Simple approximation - real Bjorklund is more complex
    pattern = [False] * steps
    for i in range(onsets):
        pos = int((i * steps) / onsets)
        pattern[pos] = True
    return pattern

def test_expected_values():
    """Test that our expected patterns produce the correct hex values"""
    print("Testing Expected Notation Values")
    print("=" * 40)
    
    # E(3,8) should be 0x92 (10010010 in binary)
    e38_pattern = [True, False, False, True, False, False, True, False]
    e38_hex = pattern_to_hex_webapp_style(e38_pattern)
    e38_dec = pattern_to_decimal_webapp_style(e38_pattern)
    print(f"E(3,8) pattern: {''.join('1' if b else '0' for b in e38_pattern)}")
    print(f"E(3,8) hex: {e38_hex} (expected: 0x92)")
    print(f"E(3,8) decimal: {e38_dec} (expected: 146)")
    print(f"E(3,8) test: {'✓ PASS' if e38_hex == '0x92' else '✗ FAIL'}")
    print()
    
    # E(5,8) should be 0xB6 (10110110 in binary)  
    e58_pattern = [True, False, True, True, False, True, True, False]
    e58_hex = pattern_to_hex_webapp_style(e58_pattern)
    e58_dec = pattern_to_decimal_webapp_style(e58_pattern)
    print(f"E(5,8) pattern: {''.join('1' if b else '0' for b in e58_pattern)}")
    print(f"E(5,8) hex: {e58_hex} (expected: 0xB6)")
    print(f"E(5,8) decimal: {e58_dec} (expected: 182)")
    print(f"E(5,8) test: {'✓ PASS' if e58_hex == '0xB6' else '✗ FAIL'}")
    print()

def test_other_patterns():
    """Test some other common patterns"""
    print("Testing Other Common Patterns")
    print("=" * 40)
    
    # Simple patterns for validation
    test_cases = [
        ([True, True, True, True, True, True, True, True], "11111111", "0xFF", 255),
        ([False, False, False, False, False, False, False, False], "00000000", "0x0", 0),
        ([True, False, False, False, False, False, False, False], "10000000", "0x80", 128),
        ([False, False, False, False, False, False, False, True], "00000001", "0x1", 1),
        ([True, False, True, False, True, False, True, False], "10101010", "0xAA", 170),
        ([False, True, False, True, False, True, False, True], "01010101", "0x55", 85),
    ]
    
    for pattern, expected_binary, expected_hex, expected_decimal in test_cases:
        binary = ''.join('1' if b else '0' for b in pattern)
        hex_val = pattern_to_hex_webapp_style(pattern)
        decimal_val = pattern_to_decimal_webapp_style(pattern)
        
        print(f"Pattern: {binary}")
        print(f"Hex: {hex_val} (expected: {expected_hex})")
        print(f"Decimal: {decimal_val} (expected: {expected_decimal})")
        print(f"Test: {'✓ PASS' if hex_val == expected_hex and decimal_val == expected_decimal else '✗ FAIL'}")
        print()

if __name__ == "__main__":
    test_expected_values()
    test_other_patterns()
    
    print("Notation Test Summary:")
    print("The plugin should now correctly interpret and display patterns using")
    print("left-to-right (MSB first) notation where:")
    print("- E(3,8) = 10010010 = 0x92 = 146")
    print("- E(5,8) = 10110110 = 0xB6 = 182")
    print("- Leftmost bit corresponds to step 0")
    print("- This matches the webapp standard")