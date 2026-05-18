#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <chrono>

// Include BOTH brains
#include "ai_predictor_VANILLA.h"
#include "ai_predictor_LATEST.h"

using namespace std;

// Struct to hold our branch data in RAM
struct BranchRecord {
    uint64_t pc;
    uint64_t target;
    int isBackward;
    uint8_t localHistory;
    int actual_taken;
};

int main() {
    cout << "\n==================================================" << endl;
    cout << "   XGBOOST SILICON SHOWDOWN: VANILLA VS PATCHED   " << endl;
    cout << "==================================================" << endl;
    
    ifstream file("branch_data.csv");
    string line;

    if (!file.is_open()) {
        cout << "Error: Could not open branch_data.csv." << endl;
        return 1;
    }

    getline(file, line); // Skip header

    // 1. LOAD DATA INTO RAM (To isolate I/O latency from compute latency)
    cout << "\n[1/3] Caching Adversarial Trace into RAM..." << endl;
    vector<BranchRecord> traces;
    // Reserve space to prevent reallocation slowdowns (assuming ~3M branches)
    traces.reserve(3000000); 

    while (getline(file, line)) {
        if (line.empty()) continue; 

        stringstream ss(line);
        string pc_str, target_str, backward_str, hist_str, taken_str;

        getline(ss, pc_str, ',');
        getline(ss, target_str, ',');
        getline(ss, backward_str, ',');
        getline(ss, hist_str, ',');
        getline(ss, taken_str);

        try {
            BranchRecord rec;
            rec.pc = stoull(pc_str, nullptr, 16);
            rec.target = stoull(target_str, nullptr, 16);
            rec.isBackward = stoi(backward_str);
            rec.localHistory = static_cast<uint8_t>(stoi(hist_str));
            rec.actual_taken = stoi(taken_str);
            traces.push_back(rec);
        } catch (...) {
            continue;
        }
    }
    
    long long total_branches = traces.size();
    cout << "      Loaded " << total_branches << " branches successfully." << endl;

    // 2. RUN VANILLA BENCHMARK
    cout << "[2/3] Executing Vanilla Model (v1.7.3) Benchmarks..." << endl;
    long long correct_vanilla = 0;
    
    auto start_vanilla = chrono::high_resolution_clock::now();
    for (const auto& t : traces) {
        if (AIPredictorVanilla::predict(t.pc, t.target, t.isBackward, t.localHistory) == t.actual_taken) {
            correct_vanilla++;
        }
    }
    auto end_vanilla = chrono::high_resolution_clock::now();
    auto duration_vanilla = chrono::duration_cast<chrono::nanoseconds>(end_vanilla - start_vanilla).count();

    // 3. RUN LATEST PATCHED BENCHMARK
    cout << "[3/3] Executing Latest Model (v3.x Patched) Benchmarks..." << endl;
    long long correct_latest = 0;
    
    auto start_latest = chrono::high_resolution_clock::now();
    for (const auto& t : traces) {
        if (AIPredictorLatest::predict(t.pc, t.target, t.isBackward, t.localHistory) == t.actual_taken) {
            correct_latest++;
        }
    }
    auto end_latest = chrono::high_resolution_clock::now();
    auto duration_latest = chrono::duration_cast<chrono::nanoseconds>(end_latest - start_latest).count();

    // 4. CALCULATE METRICS
    double acc_vanilla = ((double)correct_vanilla / total_branches) * 100.0;
    double acc_latest = ((double)correct_latest / total_branches) * 100.0;
    
    double ns_per_branch_vanilla = (double)duration_vanilla / total_branches;
    double ns_per_branch_latest = (double)duration_latest / total_branches;

    // 5. PRINT THE SHOWDOWN RESULTS
    cout << "\n==================================================" << endl;
    cout << "             FINAL C++ HARDWARE METRICS           " << endl;
    cout << "==================================================" << endl;
    cout << left << setw(20) << "Metric" << " | " << setw(12) << "Vanilla" << " | " << "Latest (Patched)" << endl;
    cout << "--------------------------------------------------" << endl;
    
    cout << left << setw(20) << "Total Branches" << " | " << setw(12) << total_branches << " | " << total_branches << endl;
    cout << left << setw(20) << "Correct Guesses" << " | " << setw(12) << correct_vanilla << " | " << correct_latest << endl;
    cout << left << setw(20) << "Hardware Accuracy" << " | " << fixed << setprecision(3) << acc_vanilla << "%  | " << acc_latest << "%" << endl;
    cout << left << setw(20) << "Latency / Branch" << " | " << fixed << setprecision(2) << ns_per_branch_vanilla << " ns  | " << ns_per_branch_latest << " ns" << endl;
    cout << "==================================================\n" << endl;

    return 0;
}