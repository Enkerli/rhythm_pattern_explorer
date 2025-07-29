/*
  ==============================================================================

    DebugLogger.h
    Simple debug logging system for iPad RPE development
    
    Writes debug info to Documents folder for easy access

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <fstream>
#include <iostream>

class DebugLogger
{
public:
    static DebugLogger& getInstance()
    {
        static DebugLogger instance;
        return instance;
    }
    
    void log(const juce::String& message)
    {
        #if DEBUG
        auto timestamp = juce::Time::getCurrentTime().toString(true, true, true, true);
        juce::String logEntry = "[" + timestamp + "] " + message + "\n";
        
        // Write to file
        if (logFile.is_open())
        {
            logFile << logEntry.toStdString();
            logFile.flush(); // Ensure immediate write
        }
        
        // Also output to console for Xcode debugging
        std::cout << logEntry.toStdString();
        #endif
    }
    
    void logTiming(const juce::String& event, double ppqPosition, double hostBPM, int step, int sample)
    {
        #if DEBUG
        juce::String message = juce::String::formatted(
            "TIMING: %s | PPQ: %.4f | BPM: %.1f | Step: %d | Sample: %d",
            event.toRawUTF8(), ppqPosition, hostBPM, step, sample
        );
        log(message);
        #endif
    }
    
    void logMIDI(const juce::String& event, int noteNumber, float velocity, int sample)
    {
        #if DEBUG
        juce::String message = juce::String::formatted(
            "MIDI: %s | Note: %d | Vel: %.2f | Sample: %d",
            event.toRawUTF8(), noteNumber, velocity, sample
        );
        log(message);
        #endif
    }

private:
    DebugLogger()
    {
        #if DEBUG
        // Try multiple locations for iOS compatibility
        juce::File logFileObj;
        
        // Option 1: App sandbox documents directory
        juce::File appDocuments = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
        logFileObj = appDocuments.getChildFile("RPE_iPad_Debug.log");
        
        // Ensure directory exists
        logFileObj.getParentDirectory().createDirectory();
        
        logFile.open(logFileObj.getFullPathName().toStdString(), std::ios::out | std::ios::app);
        
        if (logFile.is_open())
        {
            log("=== iPad RPE Debug Session Started ===");
            log("Log file location: " + logFileObj.getFullPathName());
        }
        else
        {
            // Fallback to console only
            std::cout << "Failed to open log file: " << logFileObj.getFullPathName().toStdString() << std::endl;
        }
        #endif
    }
    
    ~DebugLogger()
    {
        #if DEBUG
        if (logFile.is_open())
        {
            log("=== iPad RPE Debug Session Ended ===");
            logFile.close();
        }
        #endif
    }
    
    std::ofstream logFile;
};

// Convenience macros
#define DEBUG_LOG(message) DebugLogger::getInstance().log(message)
#define DEBUG_TIMING(event, ppq, bpm, step, sample) DebugLogger::getInstance().logTiming(event, ppq, bpm, step, sample)
#define DEBUG_MIDI(event, note, vel, sample) DebugLogger::getInstance().logMIDI(event, note, vel, sample)