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
// make it static to help the compiler optimize logbranch
static UINT64 bcountTaken = 0;  // Conditional branches taken
static UINT64 bcountNotTaken = 0; // Conditional branches not taken

typedef struct BranchInfo {
    ADDRINT instPtr;        // Instruction pointer
    ADDRINT branchTarget;   // Branch target address
    ADDRINT branchFallThrough;  // Fall through target
    BOOL branchTaken;       // Branch taken or not
    struct BranchInfo* next; // Pointer to the next node
} BranchInfo;

BranchInfo* branchListHead = NULL;

// basic linked list code copied from internet
void AddBranchInfo(ADDRINT instPtr, ADDRINT branchTarget, ADDRINT branchFallThrough, BOOL branchTaken) {
    BranchInfo* newNode = (BranchInfo*)malloc(sizeof(BranchInfo));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->instPtr = instPtr;
    newNode->branchTarget = branchTarget;
    newNode->branchFallThrough = branchFallThrough;
    newNode->branchTaken = branchTaken;
    newNode->next = branchListHead;
    branchListHead = newNode;
}


// This function is called before every instruction is executed
VOID logbranch(ADDRINT instPtr, ADDRINT branchTarget, ADDRINT branchFallThrough, BOOL branchTaken) {
    if (branchTaken){
        bcountTaken++;
    }
    else{
        bcountNotTaken++;
    }
    AddBranchInfo(instPtr, branchTarget, branchFallThrough, branchTaken);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v) {

    // https://software.intel.com/sites/landingpage/pintool/docs/98484/Pin/html/group__IMG.html#ga2be7f100f47a86fadee6d0d65641e0c0
    // from the PIN library
    // TRUE for the image Pin was applied on in the command line (i.e. first param after –)
    // skips non main ones

    IMG img = IMG_FindByAddress(INS_Address(ins));

    if (!IMG_Valid(img) || !IMG_IsMainExecutable(img)) {
        return;
    }

    // std::cout << "Current image: " << IMG_Name(img) << endl;

    if (INS_IsBranch(ins) && INS_HasFallThrough(ins)) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) logbranch, 
                       IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_FALLTHROUGH_ADDR, IARG_BRANCH_TAKEN, IARG_END);
    }
}


KNOB< string > KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "branchcount.out", "specify output file name");


VOID SaveBranchListToFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        exit(1);
    }

    BranchInfo* current = branchListHead;
    while (current) {
        fprintf(file, "InstPtr: 0x%lx | Target: 0x%lx | Fall Through: 0x%lx | Taken: %d\n",
                (unsigned long)current->instPtr,
                (unsigned long)current->branchTarget,
                (unsigned long)current->branchFallThrough,
                current->branchTaken);
        current = current->next;
    }

    fclose(file);
}



// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Conditional Branches Taken: " << bcountTaken << endl;
    OutFile << "Conditional Branches Not Taken: " << bcountNotTaken << endl;
    OutFile.close();

    SaveBranchListToFile("branchlog.txt");
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
