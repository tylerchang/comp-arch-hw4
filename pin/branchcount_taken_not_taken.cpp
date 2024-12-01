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
static UINT64 bcountTaken = 0;  // Conditional branches Taken
static UINT64 bcountNotTaken = 0; // Conditional branchec Not Taken
static UINT64 ubcount = 0; // Unconditional branches


// the imag here so its intalized right away
IMG mainExecutableImg;

typedef struct BranchInfo {
    ADDRINT instPtr;        // Instruction pointer
    ADDRINT branchTarget;   // Branch target address
    BOOL branchTaken;       // Branch taken or not
    struct BranchInfo* next; // Pointer to the next node
} BranchInfo;

BranchInfo* branchListHead = NULL;

// basic linked list code copied from internet
void AddBranchInfo(ADDRINT instPtr, ADDRINT branchTarget, BOOL branchTaken) {
    BranchInfo* newNode = (BranchInfo*)malloc(sizeof(BranchInfo));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    newNode->instPtr = instPtr;
    newNode->branchTarget = branchTarget;
    newNode->branchTaken = branchTaken;
    newNode->next = branchListHead;
    branchListHead = newNode;
}


// This function is called before every instruction is executed
VOID docount(BOOL isConditional, ADDRINT instPtr, ADDRINT branchTarget, BOOL branchTaken) {
    if (isConditional){
        if (branchTaken){
            bcountTaken++;
            AddBranchInfo(instPtr, branchTarget, branchTaken);
        }
        else{
            bcountNotTaken ++;
        }
    } else {
        ubcount++;
    }
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID* v) {

    // https://software.intel.com/sites/landingpage/pintool/docs/98484/Pin/html/group__IMG.html#ga2be7f100f47a86fadee6d0d65641e0c0
    // from the PIN library
    // TRUE for the image Pin was applied on in the command line (i.e. first param after –)
    // skips non main ones
    if (!IMG_IsMainExecutable(IMG_FindByAddress(INS_Address(ins)))) {
        return;
    }

    if (INS_IsBranch(ins)) {
        BOOL isConditional = INS_HasFallThrough(ins);  // A branch with fall through must be conditional
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR) docount, IARG_BOOL, isConditional, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
    }
}

// not sure but internet suggests this might solve?
VOID ImageLoad(IMG img, VOID* v) {
    if (IMG_IsMainExecutable(img) && !mainExecutableImg.is_valid()) {
        mainExecutableImg = img;
        cerr << "Main executable image loaded: " << IMG_Name(img) << endl;
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
        fprintf(file, "InstPtr: 0x%lx, Target: 0x%lx, Taken: %d\n",
                (unsigned long)current->instPtr,
                (unsigned long)current->branchTarget,
                current->branchTaken);
        current = current->next;
    }

    fclose(file);
}

void FreeBranchList() {
    while (branchListHead) {
        BranchInfo* temp = branchListHead;
        branchListHead = branchListHead->next;
        free(temp);
    }
}


// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "Conditional Branch Count: " << (bcountTaken + bcountNotTaken) << endl;
    OutFile << "Taken Braches: " << bcountTaken << endl;
    OutFile << "Taken Braches: " << bcountNotTaken << endl;
    OutFile << "Unconditional Branch Count: " << ubcount << endl;
    OutFile.close();

    SaveBranchListToFile("branches_taken.txt");
    FreeBranchList();
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

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
