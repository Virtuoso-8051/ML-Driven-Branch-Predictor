// Aditi Chauhan (2024CSB1095)
// Anurag Raj (2024CSB1101)

#include "pin.H"
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;
ofstream OutFile;

// ---> EXTREME TUNING KNOBS <---
#define HISTORY_BITS 8 
const UINT32 HISTORY_MASK = (1 << HISTORY_BITS) - 1; 

// ---> THE EMERGENCY BRAKE <---
// Stop tracing after 5 Million branches to prevent massive CSV files
#define MAX_BRANCHES 5000000 
UINT64 branchCount = 0;

unordered_map<ADDRINT, UINT32> bhr_table;

// Upgraded Recording Function
VOID RecordBranch(VOID * ip, VOID * target, BOOL taken) {
    ADDRINT pc = (ADDRINT)ip;
    ADDRINT targ = (ADDRINT)target;

    // FEATURE 1: Branch Direction (1 = Backward/Loop, 0 = Forward/If-Else)
    BOOL isBackward = (targ < pc) ? 1 : 0;

    // FEATURE 2: Fetch the current Local History for this specific PC
    UINT32 current_history = bhr_table[pc];

    // Increment our master counter
    branchCount++;

    // Write data using "\n" instead of "endl" for massive speed boost!
    OutFile << ip << "," 
            << target << "," 
            << isBackward << "," 
            << current_history << "," 
            << taken << "\n";

    // Update history for the next time this PC is hit
    bhr_table[pc] = ((current_history << 1) | (taken ? 1 : 0)) & HISTORY_MASK;

    // THE BRAKE: If we hit 5 million, force the program to safely exit
    if (branchCount >= MAX_BRANCHES) {
        PIN_ExitApplication(0);
    }
}

// Instrumentation Routine
VOID Instruction(INS ins, VOID *v) {
    // Only target conditional branches
    if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {
        INS_InsertCall(
            ins, IPOINT_BEFORE, (AFUNPTR)RecordBranch,
            IARG_INST_PTR,
            IARG_BRANCH_TARGET_ADDR,
            IARG_BRANCH_TAKEN,
            IARG_END);
    }
}

VOID Fini(INT32 code, VOID *v) {
    OutFile.close();
    cout << "\n==========================================" << endl;
    cout << "  TRACE EXTRACTION COMPLETE (V2 - MEGA AI)" << endl;
    cout << "  Branches Extracted: " << branchCount << endl;
    cout << "==========================================\n" << endl;
}

int main(int argc, char * argv[]) {
    if (PIN_Init(argc, argv)) return 1;
    
    OutFile.open("branch_data.csv");
    // Write header with \n
    OutFile << "PC,Target,IsBackward,LocalHistory,Taken\n";
    
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);
    PIN_StartProgram();
    
    return 0;
}
