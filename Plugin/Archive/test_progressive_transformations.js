#!/usr/bin/env node

// Comprehensive Progressive Transformation Test
// Tests all 8 progressive transformations with the webapp implementation

const fs = require('fs');
const path = require('path');

// Load the webapp modules
const appDir = path.join(__dirname, 'app');

// Read and evaluate the modules in order
function loadModule(filename) {
    const filepath = path.join(appDir, filename);
    const code = fs.readFileSync(filepath, 'utf8');
    eval(code);
}

console.log('Loading webapp modules...');
loadModule('math-core.js');
loadModule('pattern-generators.js');
loadModule('pattern-processing.js');
loadModule('pattern-analysis.js');

// Test configuration for all 8 transformations
const testConfigs = [
    {
        name: "E(8,8)B>1",
        description: "Barlow concentration: 8 to 1 onsets",
        basePattern: "E(8,8)",
        transformer: "B",
        targetOnsets: 1,
        algorithm: "Barlow Indispensability",
        mode: "concentration"
    },
    {
        name: "E(1,8)B>8", 
        description: "Barlow dilution: 1 to 8 onsets",
        basePattern: "E(1,8)",
        transformer: "B",
        targetOnsets: 8,
        algorithm: "Barlow Indispensability",
        mode: "dilution"
    },
    {
        name: "E(8,8)W>1",
        description: "Wolrab concentration: 8 to 1 onsets", 
        basePattern: "E(8,8)",
        transformer: "W",
        targetOnsets: 1,
        algorithm: "Wolrab (Anti-Barlow)",
        mode: "concentration"
    },
    {
        name: "E(1,8)W>8",
        description: "Wolrab dilution: 1 to 8 onsets",
        basePattern: "E(1,8)", 
        transformer: "W",
        targetOnsets: 8,
        algorithm: "Wolrab (Anti-Barlow)",
        mode: "dilution"
    },
    {
        name: "E(8,8)E>1",
        description: "Euclidean concentration: 8 to 1 onsets",
        basePattern: "E(8,8)",
        transformer: "E", 
        targetOnsets: 1,
        algorithm: "Euclidean (Bjorklund)",
        mode: "concentration"
    },
    {
        name: "E(1,8)E>8",
        description: "Euclidean dilution: 1 to 8 onsets",
        basePattern: "E(1,8)",
        transformer: "E",
        targetOnsets: 8, 
        algorithm: "Euclidean (Bjorklund)",
        mode: "dilution"
    },
    {
        name: "E(8,8)D>1", 
        description: "Dilcue concentration: 8 to 1 onsets",
        basePattern: "E(8,8)",
        transformer: "D",
        targetOnsets: 1,
        algorithm: "Dilcue (Anti-Euclidean)",
        mode: "concentration"
    },
    {
        name: "E(1,8)D>8",
        description: "Dilcue dilution: 1 to 8 onsets", 
        basePattern: "E(1,8)",
        transformer: "D",
        targetOnsets: 8,
        algorithm: "Dilcue (Anti-Euclidean)",
        mode: "dilution"
    }
];

// Helper function to format binary pattern
function formatPattern(steps) {
    return steps.map(s => s ? '1' : '0').join('');
}

// Helper function to generate base patterns
function generateBasePattern(patternNotation) {
    if (patternNotation === "E(8,8)") {
        return EuclideanGenerator.generate(8, 8, 0);
    } else if (patternNotation === "E(1,8)") {
        return EuclideanGenerator.generate(1, 8, 0);
    }
    throw new Error(`Unknown base pattern: ${patternNotation}`);
}

// Generate Barlow/Wolrab transformation
function generateBarlowTransformation(currentSteps, targetOnsets, wolrabMode = false) {
    try {
        const result = BarlowTransformer.transformPattern(
            currentSteps,
            targetOnsets,
            {
                wolrabMode: wolrabMode,
                preserveDownbeat: true,
                minimumIndispensability: 0.0
            }
        );
        return result && result.pattern ? result.pattern : null;
    } catch (error) {
        console.error('Error in Barlow transformation:', error);
        return null;
    }
}

// Generate Euclidean/Dilcue transformation  
function generateEuclideanTransformation(currentSteps, targetOnsets, antiMode = false) {
    try {
        const currentOnsets = currentSteps.filter(s => s).length;
        const result = EuclideanTransformer.transform(
            currentSteps,
            targetOnsets,
            {
                operation: targetOnsets > currentOnsets ? 'concentrate' : 'dilute',
                mode: antiMode ? 'anti' : 'normal'
            }
        );
        return result && result.transformed ? result.transformed : null;
    } catch (error) {
        console.error('Error in Euclidean transformation:', error);
        return null;
    }
}

// Generate progressive sequence
function generateProgressiveSequence(config) {
    try {
        // Generate base pattern
        const basePattern = generateBasePattern(config.basePattern);
        console.log(`Generated base pattern ${config.basePattern}: ${formatPattern(basePattern)}`);
        
        const baseOnsetCount = basePattern.filter(s => s).length;
        const targetOnsets = config.targetOnsets;
        
        if (baseOnsetCount === targetOnsets) {
            return {
                basePattern: basePattern,
                transformations: [],
                error: 'Current and target onset counts are the same'
            };
        }
        
        const transformations = [];
        const direction = targetOnsets > baseOnsetCount ? 1 : -1;
        let currentPattern = [...basePattern];
        
        // Generate each step in the sequence
        for (let onsets = baseOnsetCount + direction; 
             direction > 0 ? onsets <= targetOnsets : onsets >= targetOnsets; 
             onsets += direction) {
             
            let transformedPattern;
            
            switch (config.transformer) {
                case 'B':
                    transformedPattern = generateBarlowTransformation(currentPattern, onsets, false);
                    break;
                case 'W':
                    transformedPattern = generateBarlowTransformation(currentPattern, onsets, true);
                    break;
                case 'E':
                    transformedPattern = generateEuclideanTransformation(currentPattern, onsets, false);
                    break;
                case 'D':
                    transformedPattern = generateEuclideanTransformation(currentPattern, onsets, true);
                    break;
                default:
                    throw new Error(`Unknown transformer type: ${config.transformer}`);
            }
            
            if (transformedPattern) {
                transformations.push({
                    onsetCount: onsets,
                    stepNumber: Math.abs(onsets - baseOnsetCount),
                    pattern: transformedPattern,
                    binary: formatPattern(transformedPattern)
                });
                currentPattern = transformedPattern;
            } else {
                console.error(`Transformation failed for ${onsets} onsets`);
                break;
            }
        }
        
        return {
            basePattern: basePattern,
            transformations: transformations,
            success: true
        };
        
    } catch (error) {
        return {
            basePattern: null,
            transformations: [],
            error: error.message
        };
    }
}

// Run all tests
function runAllTests() {
    console.log('\n' + '='.repeat(80));
    console.log('COMPREHENSIVE PROGRESSIVE TRANSFORMATION TEST RESULTS');
    console.log('='.repeat(80));
    
    const testResults = {
        patterns: [],
        summary: {
            total: testConfigs.length,
            passed: 0,
            failed: 0,
            errors: []
        }
    };
    
    testConfigs.forEach((config, index) => {
        console.log(`\n${index + 1}. ${config.name}`);
        console.log(`Description: ${config.description}`);
        console.log(`Algorithm: ${config.algorithm}`);
        console.log(`Mode: ${config.mode}`);
        
        const result = generateProgressiveSequence(config);
        
        if (result.error) {
            console.log(`❌ Error: ${result.error}`);
            testResults.summary.failed++;
            testResults.summary.errors.push(`${config.name}: ${result.error}`);
        } else {
            testResults.summary.passed++;
            
            // Base pattern
            const basePattern = formatPattern(result.basePattern);
            const baseOnsets = result.basePattern.filter(s => s).length;
            console.log(`Base Pattern: ${config.basePattern} = ${basePattern} (${baseOnsets} onsets)`);
            
            // Transformations
            if (result.transformations.length > 0) {
                console.log('Progressive Steps:');
                console.log('Step | Onsets | Pattern  | Algorithm');
                console.log('-----|--------|----------|----------');
                
                result.transformations.forEach((transform) => {
                    console.log(`  ${transform.stepNumber.toString().padStart(2)} |    ${transform.onsetCount}   | ${transform.binary} | ${config.algorithm}`);
                });
                
                // Final result
                const finalTransform = result.transformations[result.transformations.length - 1];
                console.log(`✅ Final Result: ${baseOnsets} → ${finalTransform.onsetCount} onsets using ${config.algorithm}`);
                
            } else {
                console.log('No transformations generated');
            }
        }
        
        // Store results
        testResults.patterns.push({
            config: config,
            result: result
        });
    });
    
    // Summary
    console.log('\n' + '='.repeat(80));
    console.log('TEST SUMMARY');
    console.log('='.repeat(80));
    console.log(`Total Tests: ${testResults.summary.total}`);
    console.log(`Passed: ${testResults.summary.passed}`);
    console.log(`Failed: ${testResults.summary.failed}`);
    
    if (testResults.summary.errors.length > 0) {
        console.log('\nErrors:');
        testResults.summary.errors.forEach(error => {
            console.log(`- ${error}`);
        });
    }
    
    console.log('\n' + '='.repeat(80));
    console.log('PLUGIN IMPLEMENTATION REFERENCE');
    console.log('='.repeat(80));
    console.log('Base Pattern Generators:');
    console.log('- E(8,8) = "11111111" (8 onsets in 8 steps)');
    console.log('- E(1,8) = "10000000" (1 onset in 8 steps)');
    console.log('');
    console.log('Progressive Transformation Algorithms:');
    console.log('1. Barlow (B): Uses indispensability values to determine onset priorities');
    console.log('2. Wolrab (W): Anti-Barlow, removes/adds most indispensable positions first');
    console.log('3. Euclidean (E): Uses Bjorklund algorithm for maximal evenness');
    console.log('4. Dilcue (D): Anti-Euclidean, uses complement patterns');
    console.log('');
    console.log('Expected patterns for plugin implementation validation:');
    
    testResults.patterns.forEach((test, index) => {
        if (test.result.success && test.result.transformations.length > 0) {
            console.log(`\n${test.config.name}:`);
            console.log(`  Base: ${formatPattern(test.result.basePattern)}`);
            test.result.transformations.forEach(t => {
                console.log(`  Step ${t.stepNumber}: ${t.binary} (${t.onsetCount} onsets)`);
            });
        }
    });
    
    return testResults;
}

// Run the tests
console.log('Starting comprehensive progressive transformation tests...');
const results = runAllTests();
console.log('\nTests completed successfully.');