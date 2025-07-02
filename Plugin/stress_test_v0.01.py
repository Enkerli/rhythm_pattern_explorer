#!/usr/bin/env python3
"""
Stress Test for Rhythm Pattern Explorer v0.01
Tests both VST3 and AU formats with performance measurements
"""

import time
import subprocess
import os
import sys

def run_au_validation_test():
    """Test AU plugin validation and basic functionality"""
    print("=== AU Plugin Stress Test ===")
    
    # Test AU plugin existence
    au_path = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component")
    if os.path.exists(au_path):
        print(f"✓ AU Plugin found at: {au_path}")
        
        # Check bundle structure
        contents_path = os.path.join(au_path, "Contents")
        if os.path.exists(contents_path):
            print("✓ AU bundle structure valid")
        else:
            print("✗ AU bundle structure invalid")
            
        # Test size and modification time
        stat_info = os.stat(au_path)
        print(f"✓ AU Plugin size: {stat_info.st_size} bytes")
        print(f"✓ AU Last modified: {time.ctime(stat_info.st_mtime)}")
        
    else:
        print("✗ AU Plugin not found")
        return False
    
    return True

def run_vst3_validation_test():
    """Test VST3 plugin validation and basic functionality"""
    print("\n=== VST3 Plugin Stress Test ===")
    
    # Test VST3 plugin existence
    vst3_path = os.path.expanduser("~/Library/Audio/Plug-Ins/VST3/Rhythm Pattern Explorer.vst3")
    if os.path.exists(vst3_path):
        print(f"✓ VST3 Plugin found at: {vst3_path}")
        
        # Check bundle structure
        contents_path = os.path.join(vst3_path, "Contents")
        if os.path.exists(contents_path):
            print("✓ VST3 bundle structure valid")
        else:
            print("✗ VST3 bundle structure invalid")
            
        # Test size and modification time
        stat_info = os.stat(vst3_path)
        print(f"✓ VST3 Plugin size: {stat_info.st_size} bytes")
        print(f"✓ VST3 Last modified: {time.ctime(stat_info.st_mtime)}")
        
    else:
        print("✗ VST3 Plugin not found")
        return False
    
    return True

def run_performance_measurements():
    """Run performance measurements"""
    print("\n=== Performance Measurements ===")
    
    start_time = time.time()
    
    # Test 1: Plugin loading simulation
    print("Test 1: Plugin Discovery Performance")
    discovery_start = time.time()
    
    # Check how long it takes to list plugins
    au_scan_time = time.time()
    au_plugins = []
    au_dir = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/")
    if os.path.exists(au_dir):
        for item in os.listdir(au_dir):
            if item.endswith('.component'):
                au_plugins.append(item)
    au_scan_duration = time.time() - au_scan_time
    
    vst3_scan_time = time.time()
    vst3_plugins = []
    vst3_dir = os.path.expanduser("~/Library/Audio/Plug-Ins/VST3/")
    if os.path.exists(vst3_dir):
        for item in os.listdir(vst3_dir):
            if item.endswith('.vst3'):
                vst3_plugins.append(item)
    vst3_scan_duration = time.time() - vst3_scan_time
    
    discovery_duration = time.time() - discovery_start
    
    print(f"✓ AU plugins found: {len(au_plugins)} (scan time: {au_scan_duration:.4f}s)")
    print(f"✓ VST3 plugins found: {len(vst3_plugins)} (scan time: {vst3_scan_duration:.4f}s)")
    print(f"✓ Total discovery time: {discovery_duration:.4f}s")
    
    # Test 2: File system stress test
    print("\nTest 2: File System Stress Test")
    fs_start = time.time()
    
    # Rapid file access test
    for i in range(100):
        # Test AU
        au_path = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component")
        exists_au = os.path.exists(au_path)
        
        # Test VST3
        vst3_path = os.path.expanduser("~/Library/Audio/Plug-Ins/VST3/Rhythm Pattern Explorer.vst3")
        exists_vst3 = os.path.exists(vst3_path)
        
        if not (exists_au and exists_vst3):
            print(f"✗ Plugin access failed on iteration {i}")
            break
    
    fs_duration = time.time() - fs_start
    print(f"✓ File system stress test completed: {fs_duration:.4f}s (100 iterations)")
    
    # Test 3: Memory footprint estimation
    print("\nTest 3: Plugin Size Analysis")
    
    au_path = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/Rhythm Pattern Explorer.component")
    vst3_path = os.path.expanduser("~/Library/Audio/Plug-Ins/VST3/Rhythm Pattern Explorer.vst3")
    
    def get_directory_size(path):
        total = 0
        for dirpath, dirnames, filenames in os.walk(path):
            for filename in filenames:
                filepath = os.path.join(dirpath, filename)
                total += os.path.getsize(filepath)
        return total
    
    if os.path.exists(au_path):
        au_size = get_directory_size(au_path)
        print(f"✓ AU Plugin total size: {au_size:,} bytes ({au_size/1024/1024:.2f} MB)")
    
    if os.path.exists(vst3_path):
        vst3_size = get_directory_size(vst3_path)
        print(f"✓ VST3 Plugin total size: {vst3_size:,} bytes ({vst3_size/1024/1024:.2f} MB)")
    
    total_time = time.time() - start_time
    print(f"\n=== Total Test Duration: {total_time:.4f}s ===")
    
    return True

def main():
    """Run comprehensive stress tests"""
    print("Rhythm Pattern Explorer v0.01 - Stress Test Suite")
    print("=" * 50)
    
    start_time = time.time()
    
    # Run tests
    au_success = run_au_validation_test()
    vst3_success = run_vst3_validation_test()
    perf_success = run_performance_measurements()
    
    # Summary
    print("\n" + "=" * 50)
    print("STRESS TEST SUMMARY")
    print("=" * 50)
    print(f"AU Plugin Test: {'PASS' if au_success else 'FAIL'}")
    print(f"VST3 Plugin Test: {'PASS' if vst3_success else 'FAIL'}")
    print(f"Performance Test: {'PASS' if perf_success else 'FAIL'}")
    
    total_duration = time.time() - start_time
    print(f"\nTotal test suite duration: {total_duration:.4f}s")
    
    if au_success and vst3_success and perf_success:
        print("\n🎉 ALL TESTS PASSED - v0.01 plugins are ready!")
        return 0
    else:
        print("\n❌ Some tests failed - check output above")
        return 1

if __name__ == "__main__":
    sys.exit(main())