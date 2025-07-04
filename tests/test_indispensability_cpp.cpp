// Test C++ indispensability implementation
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>

// Copy of the C++ implementation for testing
class IndispensabilityEngine {
public:
    std::vector<int> decompose(int n) {
        std::vector<int> result;
        
        if (n <= 0) return result;
        
        // Special handling for primes > 3
        if (isPrime(n) && n > 3) {
            int remaining = n;
            
            // Use 3s first for larger groups
            while (remaining >= 6) {
                result.push_back(3);
                remaining -= 3;
            }
            
            // Handle remainder
            if (remaining == 5) {
                result.push_back(3);
                result.push_back(2);
            } else if (remaining == 4) {
                result.push_back(2);
                result.push_back(2);
            } else if (remaining == 3) {
                result.push_back(3);
            } else if (remaining == 2) {
                result.push_back(2);
            }
        } else {
            // For composite numbers, use the original algorithm
            if (n % 2 == 1) { 
                result.push_back(3); 
                n -= 3; 
            }
            while (n > 0) { 
                result.push_back(2); 
                n -= 2; 
            }
        }
        
        std::reverse(result.begin(), result.end());
        return result;
    }
    
    std::vector<int> firstOrderBackward(const std::vector<int>& strata) {
        int total = std::accumulate(strata.begin(), strata.end(), 0);
        std::vector<std::vector<int>> groups;
        int beat = 0;
        
        // Create groups in reverse order
        for (int g : std::vector<int>(strata.rbegin(), strata.rend())) {
            std::vector<int> grp;
            for (int i = 0; i < g; ++i) {
                grp.push_back(beat++);
            }
            groups.push_back(grp);
        }
        
        // Build order - first beat of each group, then remaining by group size
        std::vector<int> order;
        for (auto& grp : groups) {
            if (!grp.empty()) {
                order.push_back(grp.front());
                grp.erase(grp.begin());
            }
        }
        
        // Add remaining beats by descending group size
        int maxlen = 0;
        for (const auto& g : groups)
            maxlen = std::max(maxlen, (int)g.size());
        
        while (maxlen > 0) {
            for (auto& grp : groups) {
                if ((int)grp.size() == maxlen && !grp.empty()) {
                    order.push_back(grp.front());
                    grp.erase(grp.begin());
                }
            }
            maxlen = 0;
            for (const auto& g : groups)
                maxlen = std::max(maxlen, (int)g.size());
        }
        
        return order;
    }
    
    std::vector<int> getBackwardPriorities(const std::vector<std::vector<int>>& strata) {
        std::vector<std::vector<int>> lists(strata.rbegin(), strata.rend());
        int R = lists.size();
        std::vector<int> dims(R), mult(R, 1);
        
        for (int i = 0; i < R; ++i)
            dims[i] = lists[i].size();
        
        for (int i = 1; i < R; ++i)
            mult[i] = mult[i-1] * dims[i-1];
        
        std::vector<int> out;
        std::vector<int> idx(R, 0);
        int total = 1;
        for (int d : dims) total *= d;
        
        for (int cnt = 0; cnt < total; ++cnt) {
            int sum = 0;
            for (int i = 0; i < R; ++i)
                sum += lists[i][idx[i]] * mult[i];
            out.push_back(sum);
            
            // Increment idx like mixed-radix
            for (int i = 0; i < R; ++i) {
                idx[i]++;
                if (idx[i] < dims[i]) break;
                idx[i] = 0;
            }
        }
        
        return out;
    }
    
    std::vector<double> indispensabilityArray(const std::vector<int>& backPrio, bool normalize = false) {
        int N = backPrio.size();
        std::vector<double> rev(N);
        
        for (int i = 0; i < N; ++i) {
            auto it = std::find(backPrio.begin(), backPrio.end(), i);
            int pos = std::distance(backPrio.begin(), it);
            rev[i] = (double)((N - 1) - pos);
        }
        
        // Rotate left by 1 and then reverse
        if (N > 1) {
            std::rotate(rev.begin(), rev.begin() + 1, rev.end());
            std::reverse(rev.begin(), rev.end());
        }
        
        if (normalize) {
            double maxv = *std::max_element(rev.begin(), rev.end());
            if (maxv > 0) {
                for (auto& val : rev)
                    val /= maxv;
            }
        }
        
        return rev;
    }
    
    std::vector<double> computeIndispensability(std::vector<int> strataRaw, bool normalize = false) {
        std::vector<std::vector<int>> strataStd;
        
        for (int s : strataRaw) {
            if (s > 2)
                strataStd.push_back(decompose(s));
            else
                strataStd.push_back(std::vector<int>{s});
        }
        
        std::vector<std::vector<int>> strataBack;
        for (const auto& st : strataStd)
            strataBack.push_back(firstOrderBackward(st));
        
        std::vector<int> backPrio = getBackwardPriorities(strataBack);
        return indispensabilityArray(backPrio, normalize);
    }
    
    bool isPrime(int n) {
        if (n < 2) return false;
        for (int i = 2; i * i <= n; ++i) {
            if (n % i == 0) return false;
        }
        return true;
    }
};

void testPattern(int steps) {
    IndispensabilityEngine engine;
    
    std::cout << "\n=== Testing " << steps << "-step pattern ===\n";
    
    // Show decomposition
    auto decomp = engine.decompose(steps);
    std::cout << "Decomposition of " << steps << ": ";
    for (int d : decomp) std::cout << d << " ";
    std::cout << "\n";
    
    // Show indispensability values
    auto indispArray = engine.computeIndispensability({steps}, true);
    std::cout << "Indispensability values:\n";
    for (int i = 0; i < indispArray.size(); ++i) {
        std::cout << "Position " << i << ": " << indispArray[i] << std::endl;
    }
    
    // Show pattern for different onset counts
    for (int onsets = 1; onsets <= std::min(steps, 5); ++onsets) {
        // Create position-indispensability pairs
        std::vector<std::pair<int, double>> positions;
        for (int i = 0; i < steps; ++i) {
            positions.emplace_back(i, indispArray[i]);
        }
        
        // Sort by indispensability (descending)
        std::sort(positions.begin(), positions.end(), 
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        // Select top 'onsets' positions
        std::vector<bool> pattern(steps, false);
        for (int i = 0; i < onsets && i < positions.size(); ++i) {
            pattern[positions[i].first] = true;
        }
        
        std::cout << onsets << " onsets: ";
        for (bool b : pattern) std::cout << (b ? '1' : '0');
        std::cout << std::endl;
    }
}

int main() {
    std::cout << "Testing Evanstein's hierarchical indispensability algorithm\n";
    std::cout << "=========================================================\n";
    
    // Test problematic prime patterns
    testPattern(7);
    testPattern(11);
    testPattern(13);
    
    // Test some composite patterns for comparison
    testPattern(8);
    testPattern(12);
    testPattern(16);
    
    return 0;
}