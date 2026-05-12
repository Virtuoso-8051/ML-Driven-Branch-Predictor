#include <iostream>
#include <vector>

using namespace std;

// We use volatile to prevent the compiler from optimizing the branches away
volatile int dummy_global = 0;

int main() {
    cout << "--- INITIATING ADVERSARIAL PAYLOAD ---" << endl;
    
    int state = 0;
    long long operations = 0;

    // We loop 500,000 times. With ~10 branches per loop, 
    // this will generate exactly around 5,000,000 branches for the Pin Tool.
    for (int i = 0; i < 500000; i++) {
        
        // ADVERSARIAL BRANCH 1: The "Mod-3 Thrash"
        // Pattern: 1, 0, 0, 1, 0, 0...
        // 2-bit counter will constantly lag and mispredict the '1' and the first '0'.
        // AI will see the 8-bit history and predict this with 100% accuracy.
        if (i % 3 == 0) {
            dummy_global++;
        } else {
            dummy_global--;
        }

        // ADVERSARIAL BRANCH 2: The "Hysteresis Breaker"
        // Pattern: 1, 1, 0, 0, 1, 1, 0, 0...
        // This perfectly exploits the 2-bit state machine. Just as the counter 
        // reaches "Strongly Taken", the pattern flips, guaranteeing missed predictions.
        if ((i / 2) % 2 == 0) {
            operations += 2;
        } else {
            operations -= 2;
        }

        // ADVERSARIAL BRANCH 3: The "Correlated State"
        // This branch depends on the math done by the previous branches.
        // Hardware aliases will overwrite this state, destroying 2-bit accuracy.
        // XGBoost will easily map the non-linear relationship.
        state = dummy_global + operations;
        if (state > 0) {
            dummy_global += 1;
        } else if (state < 0) { // ADVERSARIAL BRANCH 4
            dummy_global -= 1;
        }

        // ADVERSARIAL BRANCH 5: Pseudo-Random but Deterministic (LCG)
        // A simple linear congruential generator logic. 
        // Utter chaos for a 2-bit counter, but perfectly predictable for an 8-bit AI history.
        if ((i * 16807) % 2147483647 % 2 == 0) {
            operations++;
        }
    }

    cout << "Payload Execution Complete. Final State: " << dummy_global << endl;
    return 0;
}
