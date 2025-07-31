// Debug Bjorklund algorithm for E(3,4)
function bjorklundAlgorithmDebug(beats, steps) {
    console.log(`\n=== Debugging E(${beats},${steps}) ===`);
    
    if (beats > steps) beats = steps;
    if (beats <= 0) return new Array(steps).fill(false);
    
    let pattern = [];
    let counts = [];
    let remainders = [];
    
    let divisor = steps - beats;
    remainders[0] = beats;
    let level = 0;
    
    console.log(`Initial: divisor=${divisor}, remainders[0]=${remainders[0]}`);
    
    do {
        counts[level] = Math.floor(divisor / remainders[level]);
        remainders[level + 1] = divisor % remainders[level];
        console.log(`Level ${level}: counts[${level}]=${counts[level]}, remainders[${level+1}]=${remainders[level + 1]}`);
        divisor = remainders[level];
        level++;
    } while (remainders[level] > 1);
    
    counts[level] = divisor;
    console.log(`Final: counts[${level}]=${counts[level]}`);
    console.log(`Counts: [${counts.join(', ')}]`);
    console.log(`Remainders: [${remainders.join(', ')}]`);
    
    function build(level) {
        console.log(`Building level ${level}`);
        if (level == -1) {
            pattern.push(false);
            console.log('  Added 0');
        } else if (level == -2) {
            pattern.push(true);
            console.log('  Added 1');
        } else {
            console.log(`  Loop ${counts[level]} times on level ${level-1}`);
            for (let i = 0; i < counts[level]; i++) {
                build(level - 1);
            }
            if (remainders[level] != 0) {
                console.log(`  Building remainder level ${level-2}`);
                build(level - 2);
            }
        }
    }
    
    build(level);
    
    console.log(`Pattern before padding: [${pattern.map(b => b ? '1' : '0').join('')}]`);
    
    while (pattern.length < steps) {
        pattern.push(false);
    }
    
    console.log(`Pattern after padding: [${pattern.map(b => b ? '1' : '0').join('')}]`);
    
    // Normalize to start with first beat
    const firstBeatIndex = pattern.findIndex(beat => beat);
    console.log(`First beat at index: ${firstBeatIndex}`);
    if (firstBeatIndex > 0) {
        pattern = pattern.slice(firstBeatIndex).concat(pattern.slice(0, firstBeatIndex));
        console.log(`Pattern after rotation: [${pattern.map(b => b ? '1' : '0').join('')}]`);
    }
    
    return pattern;
}

function patternToString(pattern) {
    return pattern.map(b => b ? '1' : '0').join('');
}

// Test the problematic case
const result = bjorklundAlgorithmDebug(3, 4);
console.log(`\nFinal result: ${patternToString(result)}`);
console.log(`Expected:     1011`);

// Let's also test a working case for comparison
console.log('\n' + '='.repeat(40));
const result2 = bjorklundAlgorithmDebug(3, 8);
console.log(`\nFinal result: ${patternToString(result2)}`);
console.log(`Expected:     10010010`);