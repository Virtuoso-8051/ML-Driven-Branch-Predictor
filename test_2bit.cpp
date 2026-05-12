#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

using namespace std;

// Real hardware uses a fixed number of entries (e.g., 8192)
#define BHT_ENTRIES 8192
#define BHT_MASK (BHT_ENTRIES - 1)

int main() {
    cout << "--- BOOTING TRUE HARDWARE BIMODAL SIMULATOR ---" << endl;
    ifstream file("branch_data.csv");
    string line;
    
    if (!file.is_open()) {
        cout << "Error: Could not open branch_data.csv." << endl;
        return 1;
    }

    // Skip the first row (the header names)
    getline(file, line); 

    // The Hardware Branch History Table (BHT)
    // Initialized to 1 (Weakly Not Taken) exactly like cold-booted silicon
    vector<int> bht(BHT_ENTRIES, 1);
    
    long long total_branches = 0;
    long long correct_predictions = 0;

    cout << "Simulating strict 2-bit logic with Hardware Aliasing..." << endl;

    while (getline(file, line)) {
        stringstream ss(line);
        string pc_str, t_str, b_str, h_str, taken_str;
        
        getline(ss, pc_str, ',');     // PC
        getline(ss, t_str, ',');      // Target
        getline(ss, b_str, ',');      // IsBackward
        getline(ss, h_str, ',');      // LocalHistory
        getline(ss, taken_str, ',');  // Taken

        // Convert hex PC to integer
        long long pc = stoull(pc_str, nullptr, 16);
        int actual = stoi(taken_str);

        // REAL HARDWARE LOGIC: Map the massive PC address to a small table index
        int index = pc & BHT_MASK; 

        // Fetch the current 2-bit state from the specific table slot
        int state = bht[index];
        
        // STEP 1: MAKE THE PREDICTION (2 or 3 = Taken, 0 or 1 = Not Taken)
        int prediction = (state >= 2) ? 1 : 0;

        if (prediction == actual) {
            correct_predictions++;
        }

        // STEP 2: SATURATING UPDATE
        if (actual == 1) {
            if (state < 3) bht[index]++; 
        } else {
            if (state > 0) bht[index]--; 
        }
        
        total_branches++;
    }

    double accuracy = ((double)correct_predictions / total_branches) * 100.0;
    cout << "==========================================" << endl;
    cout << "  TRUE BIMODAL 2-BIT ACCURACY: " << fixed << setprecision(2) << accuracy << "%" << endl;
    cout << "==========================================" << endl;

    return 0;
}