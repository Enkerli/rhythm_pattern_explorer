#!/bin/bash
echo "Monitoring Console for Serpe debug messages..."
echo "Pattern to test: {E(1,3)}E(5,8)"
echo "Expected: accent pattern size should be 3, not 8"
echo ""

# Monitor Console for debug messages from Serpe
log stream --predicate 'process == "Logic Pro" OR process == "Logic" OR process == "AudioUnitHost"' --info --debug | grep -i "ACCENT DEBUG\|onset-based\|step-based\|pattern size"