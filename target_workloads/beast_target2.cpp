#include <iostream>
#include <vector>
#include <cstdlib>

using namespace std;

// Volatile sink to prevent compiler loop unrolling
volatile int global_sink = 0;

int main() {
    cout << "--- INITIATING SPEC-STYLE PARSER MICROBENCHMARK ---" << endl;
    
    int DATA_SIZE = 1500000; // Will generate ~4.5M to 5M branches
    vector<int> stream(DATA_SIZE);

    // 1. Generate a "Real-World" Data Stream
    // Mixes highly predictable structure with unpredictable noise
    for (int i = 0; i < DATA_SIZE; i++) {
        if (i % 8 == 0) stream[i] = 1;         // Simulates a "Packet Header"
        else if (i % 15 == 0) stream[i] = 2;   // Simulates an "Escape Character"
        else stream[i] = rand() % 100;         // Simulates random payload data
    }

    int state = 0;
    
    // 2. The SPEC-Style State Machine
    for (int i = 0; i < DATA_SIZE; i++) {
        int val = stream[i];

        // BRANCH 1 & 2: State Machine Logic (Mimics perlbench / gcc)
        // 2-bit counters struggle here because transitions depend on the data stream, not a loop counter.
        if (state == 0) {
            if (val == 1) {
                state = 1; // Found header, switch state
                global_sink++;
            } else {
                global_sink--;
            }
        } else if (state == 1) {
            if (val == 2) {
                state = 0; // Found escape, revert state
                global_sink += 2;
            } else {
                global_sink -= 1;
            }
        }

        // BRANCH 3: Thresholding / bounds checking (Mimics mcf / lbm)
        // Data-dependent branches are notoriously hard for hardware to predict 
        // unless they use local history correlation (which your XGBoost does!)
        if (val > 50) {
            global_sink += val;
        } else {
            global_sink -= val;
        }
    }

    cout << "SPEC-Style Execution Complete. Final Sink: " << global_sink << endl;
    return 0;
}