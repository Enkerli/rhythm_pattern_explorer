// Quick test to verify module loading and basic functionality
const fs = require('fs');

// Read and evaluate modules in order
try {
    console.log('Loading math-core.js...');
    eval(fs.readFileSync('math-core.js', 'utf8'));
    console.log('✅ MathUtils loaded');
    
    console.log('Loading pattern-generators.js...');
    eval(fs.readFileSync('pattern-generators.js', 'utf8'));
    console.log('✅ Pattern generators loaded');
    
    console.log('Loading pattern-analysis.js...');
    eval(fs.readFileSync('pattern-analysis.js', 'utf8'));
    console.log('✅ Pattern analysis loaded');
    
    console.log('Loading pattern-processing.js...');
    eval(fs.readFileSync('pattern-processing.js', 'utf8'));
    console.log('✅ Pattern processing loaded');
    
    console.log('Loading pattern-exploration.js...');
    eval(fs.readFileSync('pattern-exploration.js', 'utf8'));
    console.log('✅ Pattern exploration loaded');
    
    // Test basic functionality
    console.log('\nTesting basic functionality...');
    
    // Test MathUtils
    const gcd = MathUtils.gcd(12, 8);
    console.log(`GCD(12,8) = ${gcd}`);
    
    // Test RegularPolygonGenerator
    const triangle = RegularPolygonGenerator.generate(3, 0);
    console.log(`Triangle: ${triangle.stepCount} steps, ${triangle.steps.filter(s => s).length} onsets`);
    
    // Test PerfectBalanceAnalyzer
    const balance = PerfectBalanceAnalyzer.calculateBalance(triangle.steps, triangle.stepCount);
    console.log(`Triangle balance: ${balance.balanceScore}`);
    
    // Test SystematicExplorer
    const explorer = new SystematicExplorer();
    console.log(`SystematicExplorer created: ${explorer.results.length} results, running: ${explorer.isRunning}`);
    
    // Test combination generation
    const combinations = explorer.generateAllCombinations(3, 4, 2);
    console.log(`Generated ${combinations.length} combinations`);
    
    console.log('\n🎉 All modules loaded and basic tests passed!');
    
} catch (error) {
    console.error('❌ Error:', error.message);
    console.error('Stack:', error.stack);
}