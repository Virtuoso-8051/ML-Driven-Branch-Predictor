#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>  // Needed for uint64_t and uint8_t
#include <cmath>    // Needed for m2cgen math functions
#include <cstring>  // Needed for safe memory handling in m2cgen

#include "ai_predictor.h" // The Transpiled Mega Brain

using namespace std;

int main() {
    cout << "\n--- BOOTING BARE-METAL AI SIMULATOR ---" << endl;
    
    ifstream file("branch_data.csv");
    string line;

    if (!file.is_open()) {
        cout << "Error: Could not open branch_data.csv. Did you run the Pin tool?" << endl;
        return 1;
    }

    // Skip the V2 header row ("PC,Target,IsBackward,LocalHistory,Taken")
    getline(file, line);

    long long total_branches = 0;
    long long correct_predictions = 0;

    cout << " -> Booting Bare-Metal AI Brain..." << endl;
    cout << " -> Processing Adversarial Trace..." << endl;

    // Read the file line by line
    while (getline(file, line)) {
        // Skip empty lines to prevent stoi crashes
        if (line.empty()) continue; 

        stringstream ss(line);
        string pc_str, target_str, backward_str, hist_str, taken_str;

        // Parse our 5-column CSV format safely
        getline(ss, pc_str, ',');
        getline(ss, target_str, ',');
        getline(ss, backward_str, ',');
        getline(ss, hist_str, ',');
        
        // FIX: Read to the end of the line (no comma delimiter) for the final column
        getline(ss, taken_str);

        try {
            // Convert data types natively
            uint64_t pc = stoull(pc_str, nullptr, 16);
            uint64_t target = stoull(target_str, nullptr, 16);
            int isBackward = stoi(backward_str);
            uint8_t localHistory = static_cast<uint8_t>(stoi(hist_str));
            int actual_taken = stoi(taken_str);

            // AI makes its prediction in bare-metal C++
            int predicted = AIPredictor::predict(pc, target, isBackward, localHistory);
            
            // Tally the score
            if (predicted == actual_taken) {
                correct_predictions++;
            }

            total_branches++;
        } catch (const std::exception& e) {
            // If there's a malformed row at the end of the CSV, gracefully skip it
            continue;
        }
    }

    // Calculate final accuracy
    if (total_branches == 0) {
        cout << "Error: No valid branch data found." << endl;
        return 1;
    }

    double accuracy = ((double)correct_predictions / total_branches) * 100.0;

    cout << "\n==========================================" << endl;
    cout << "          C++ SIMULATION COMPLETE!        " << endl;
    cout << "  Total Branches:  " << total_branches << endl;
    cout << "  Correct Guesses: " << correct_predictions << endl;
    cout << "  C++ AI ACCURACY: " << fixed << setprecision(2) << accuracy << "%" << endl;
    cout << "==========================================\n" << endl;

    return 0;
}
