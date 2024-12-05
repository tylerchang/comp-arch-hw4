#include <iostream>
#include <fstream>
#include "pin.H"
using std::cerr;
using std::endl;
using std::ios;
using std::ofstream;
using std::string;

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 bcount = 0;  // Conditional branches
static UINT64 ubcount = 0; // Unconditional branches

// This function is called before every instruction is executed
VOID docount(BOOL isConditional) { 
    if (isConditional){
        bcount++;
    } else {
        ubcount++;
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v) {
    if (INS_IsBranch(ins)) {
        BOOL isConditional = INS_HasFallThrough(ins);  // A branch with fall through must be conditional
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) docount, IARG_BOOL, isConditional, IARG_END);
    }
}


KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "branchcount.out", "specify output file name");

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Conditional Branch Count: " << bcount << endl;
    OutFile << "Unconditional Branch Count: " << ubcount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of branch instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}