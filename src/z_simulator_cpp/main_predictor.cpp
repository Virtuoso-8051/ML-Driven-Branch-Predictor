#include <iostream>
#include <unordered_map>
#include "ai_predictor.h" // Your generated AI Brain!

using namespace std;

// This structure mimics the '2-bit' history state
struct BranchHistory {
    double last_outcome = 0.0;
    double second_last_outcome = 0.0;
};

// A table to remember the history of every PC we encounter
unordered_map<long long, BranchHistory> history_table;

// Function 1: Ask the AI for a prediction
bool get_ai_prediction(long long pc, long long target) {
    BranchHistory& hist = history_table[pc];

    // Prepare the input array
    double input[4];
    input[0] = (double)pc;
    input[1] = (double)target;
    input[2] = hist.last_outcome;
    input[3] = hist.second_last_outcome;

    // Prepare an empty output array for the AI to write its probabilities into
    double output[2] = {0.0, 0.0};

    // Call the brain! It reads 'input' and writes to 'output'
    predict_branch_ai(input, output);

    // output[1] contains the probability that the branch is TAKEN. 
    // If it is > 0.5 (50%), we predict Taken.
    return output[1] > 0.5;
}

// Function 2: Update the history table after the branch actually executes
void update_history(long long pc, bool actual_taken) {
    BranchHistory& hist = history_table[pc];
    hist.second_last_outcome = hist.last_outcome;
    hist.last_outcome = actual_taken ? 1.0 : 0.0;
}

int main() {
    cout << "\n--- INITIALIZING AI BRANCH PREDICTOR ---" << endl;
    
    // Let's simulate a standard 'for' loop at PC 0x4000
    long long test_pc = 0x4000;
    long long test_target = 0x4020;
    
    // In a loop that runs 4 times, it is Taken 3 times, then Not Taken to exit.
    bool actual_outcomes[] = {true, true, true, false};
    
    for (int i = 0; i < 4; i++) {
        cout << "\nExecution " << i + 1 << ":" << endl;
        
        // 1. Ask the AI what it thinks will happen BEFORE it executes
        bool predicted = get_ai_prediction(test_pc, test_target);
        cout << " -> AI Predicted:    " << (predicted ? "TAKEN" : "NOT TAKEN") << endl;
        
        // 2. The actual hardware executes the branch
        bool actual = actual_outcomes[i];
        cout << " -> Actual Hardware: " << (actual ? "TAKEN" : "NOT TAKEN") << endl;
        
        // 3. Tell the AI what actually happened so it can update its History Table
        update_history(test_pc, actual);
        
        if (predicted == actual) {
            cout << " -> Result: SUCCESS (AI Guessed Correctly)" << endl;
        } else {
            cout << " -> Result: MISPREDICT" << endl;
        }
    }
    
    cout << "\n==========================================" << endl;
    cout << "   C++ INTEGRATION TEST COMPLETE!         " << endl;
    cout << "==========================================\n" << endl;

    return 0;
}