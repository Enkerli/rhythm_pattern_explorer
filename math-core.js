// === MODULE 1: MATH CORE ===
class MathUtils {
    static gcd(a, b) {
        a = Math.abs(a);
        b = Math.abs(b);
        while (b !== 0) {
            [a, b] = [b, a % b];
        }
        return a;
    }
    
    static lcm(a, b) {
        if (a === 0 || b === 0) return 0;
        return Math.abs(a * b) / this.gcd(a, b);
    }
    
    static lcmArray(numbers) {
        if (!Array.isArray(numbers) || numbers.length === 0) {
            throw new Error('Input must be a non-empty array of numbers');
        }
        const validNumbers = numbers.filter(n => n !== 0);
        if (validNumbers.length === 0) return 0;
        return validNumbers.reduce((lcm, num) => this.lcm(lcm, num), 1);
    }
    
    static isPrime(n) {
        if (n < 2) return false;
        if (n === 2) return true;
        if (n % 2 === 0) return false;
        for (let i = 3; i <= Math.sqrt(n); i += 2) {
            if (n % i === 0) return false;
        }
        return true;
    }
    
    static areCoprimes(a, b) {
        return this.gcd(a, b) === 1;
    }
    
    static getPrimeFactors(n) {
        const factors = [];
        for (let i = 2; i <= n; i++) {
            while (n % i === 0) {
                factors.push(i);
                n /= i;
            }
        }
        return factors;
    }
}