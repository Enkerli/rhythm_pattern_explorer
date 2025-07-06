#!/usr/bin/env python3
"""
Interactive Pattern Testing Script
Tests critical patterns and guides manual verification
"""

import subprocess
import time
import os

def test_pattern_in_webapp(pattern):
    """
    Test a pattern in the webapp by opening it in browser
    Returns expected output for comparison with plugin
    """
    webapp_url = f"file:///Users/alex/Documents/Coding/rhythm_pattern_explorer/WebApp/app/index.html"
    print(f"🌐 Testing pattern '{pattern}' in webapp...")
    print(f"   Open: {webapp_url}")
    print(f"   Enter pattern: {pattern}")
    print(f"   Note the hex output and step count")

def clear_debug_logs():
    """Clear debug log files for fresh testing"""
    log_files = [
        "/tmp/rhythm_progressive_debug.log",
        "/tmp/bitwig_debug.log"
    ]
    
    for log_file in log_files:
        try:
            if os.path.exists(log_file):
                os.remove(log_file)
                print(f"🗑️  Cleared {log_file}")
        except:
            pass

def check_debug_logs():
    """Check debug logs for useful information"""
    log_files = [
        "/tmp/rhythm_progressive_debug.log",
        "/tmp/bitwig_debug.log"
    ]
    
    for log_file in log_files:
        if os.path.exists(log_file):
            print(f"\n📋 Contents of {log_file}:")
            try:
                with open(log_file, 'r') as f:
                    lines = f.readlines()[-10:]  # Last 10 lines
                    for line in lines:
                        print(f"   {line.strip()}")
            except:
                print(f"   (Error reading {log_file})")
        else:
            print(f"\n📋 {log_file}: Not found")

def run_comprehensive_tests():
    """Run the comprehensive test suite"""
    
    print("🧪 COMPREHENSIVE PATTERN TESTING")
    print("="*50)
    print()
    
    # Clear logs for fresh start
    clear_debug_logs()
    
    test_cases = [
        {
            "name": "Single Polygon P(7,0)",
            "pattern": "P(7,0)",
            "expected_steps": 7,
            "expected_pattern": "1111111",
            "expected_hex": "0x7F",
            "critical": False
        },
        {
            "name": "Single Polygon P(11,0)", 
            "pattern": "P(11,0)",
            "expected_steps": 11,
            "expected_pattern": "11111111111",
            "expected_hex": "0x7FF",
            "critical": False
        },
        {
            "name": "CRITICAL: Polygon Combination P(7,0)+P(11,0)",
            "pattern": "P(7,0)+P(11,0)",
            "expected_steps": 77,
            "expected_pattern": "Complex 77-step pattern",
            "expected_hex": "NOT 0x7F, should be complex",
            "critical": True
        },
        {
            "name": "Progressive Offset P(7,0)+2",
            "pattern": "P(7,0)+2",
            "expected_steps": 7,
            "expected_pattern": "Changes with each trigger",
            "expected_hex": "Progressive behavior",
            "critical": False
        },
        {
            "name": "Scene with Combination",
            "pattern": "P(3,0)|P(7,0)+P(11,0)|E(5,8)",
            "expected_steps": "3, then 77, then 8",
            "expected_pattern": "Scene cycling",
            "expected_hex": "Varies by scene",
            "critical": True
        },
        {
            "name": "Multi-Polygon P(3,1)+P(5,0)",
            "pattern": "P(3,1)+P(5,0)",
            "expected_steps": 15,
            "expected_pattern": "LCM(3,5) = 15 steps",
            "expected_hex": "Complex 15-step pattern",
            "critical": True
        }
    ]
    
    print("📋 TEST CASES TO VERIFY IN PLUGIN:")
    print()
    
    for i, test in enumerate(test_cases, 1):
        marker = "🔴 CRITICAL" if test["critical"] else "🟢 STANDARD"
        print(f"{i}. {marker} {test['name']}")
        print(f"   Pattern: {test['pattern']}")
        print(f"   Expected Steps: {test['expected_steps']}")
        print(f"   Expected Output: {test['expected_pattern']}")
        print(f"   Expected Hex: {test['expected_hex']}")
        print()
    
    print("🎯 MANUAL TESTING INSTRUCTIONS:")
    print()
    print("1. Open your DAW (Bitwig Studio recommended)")
    print("2. Load the Rhythm Pattern Explorer plugin")
    print("3. Verify parameter interface:")
    print("   - Should see ONLY 3 parameters:")
    print("     * Use Host Transport (boolean)")
    print("     * MIDI Note (0-127, default 36)")
    print("     * Tick (boolean)")
    print("   - Should NOT see: BPM, Pattern Type, Onsets, Steps, Playing")
    print()
    print("4. Test each pattern above:")
    print("   a. Enter pattern in plugin UI text field")
    print("   b. Press Parse button or trigger Tick parameter")
    print("   c. Check pattern display for step count and visualization")
    print("   d. Note hex output if visible")
    print("   e. For scenes, trigger multiple times to cycle")
    print()
    print("5. CRITICAL TEST - P(7,0)+P(11,0):")
    print("   ❌ FAIL: Shows 7 steps with pattern '1111111'")
    print("   ✅ PASS: Shows 77 steps with complex pattern")
    print()
    
    input("Press Enter when ready to start testing...")
    
    # Test webapp patterns for comparison
    print("\n🌐 WEBAPP REFERENCE TESTING:")
    print("Open webapp to get reference outputs...")
    
    for test in test_cases:
        if test["critical"]:
            print(f"\n🔍 Testing: {test['pattern']}")
            test_pattern_in_webapp(test["pattern"])
            input("   Press Enter when you've noted the webapp output...")
    
    print("\n🔧 PLUGIN TESTING:")
    print("Now test the same patterns in the plugin and compare outputs.")
    input("Press Enter when you've completed plugin testing...")
    
    # Check debug logs
    print("\n📊 DEBUG LOG ANALYSIS:")
    check_debug_logs()
    
    print("\n📝 RESULTS SUMMARY:")
    print("Please report your findings:")
    
    for i, test in enumerate(test_cases, 1):
        if test["critical"]:
            result = input(f"{i}. {test['name']} - PASS/FAIL? ").upper()
            if result.startswith('F'):
                print(f"   ❌ FAILED: {test['pattern']}")
            else:
                print(f"   ✅ PASSED: {test['pattern']}")
    
    print("\n🎉 Testing complete!")
    print("If any CRITICAL tests failed, those need to be addressed.")

if __name__ == "__main__":
    run_comprehensive_tests()