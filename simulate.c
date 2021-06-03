#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_MEMORY 65536 // maximum number of words in memory.
#define NUM_REGS 8       // number of machine registers.
#define MAX_LINE_LENGTH 1000
typedef char string_t[MAX_LINE_LENGTH];

typedef struct
{
    int pc;
    int mem[NUM_MEMORY];
    int reg[NUM_REGS];
    int numMemory;
} state_t;

/*
 * Common Codes
 */
typedef union
{
    unsigned int code;

    struct
    {
        unsigned int destReg : 3;
        unsigned int unused1 : 13;
        unsigned int regB : 3;
        unsigned int regA : 3;
        unsigned int opcode : 3;
        unsigned int unused0 : 7;
    } r;

    struct
    {
        int offset : 16;
        unsigned int regB : 3;
        unsigned int regA : 3;
        unsigned int opcode : 3;
        unsigned int unused : 7;
    } i;

    struct
    {
        unsigned int unused1 : 16;
        unsigned int regB : 3;
        unsigned int regA : 3;
        unsigned int opcode : 3;
        unsigned int unused0 : 7;
    } j;

    struct
    {
        unsigned int unused1 : 22;
        unsigned int opcode : 3;
        unsigned int unused0 : 7;
    } o;
} inst_t;

enum OpCode
{
    OP_ADD = 0b000,
    OP_NOR = 0b001,
    OP_LW = 0b010,
    OP_SW = 0b011,
    OP_BEQ = 0b100,
    OP_JALR = 0b101,
    OP_HALT = 0b110,
    OP_NOOP = 0b111
};

/*
 * Common Codes - End
 */

// 0 is pc+1, 1 is jump addr
const char PCMuxTable[] = {
    [OP_ADD] 0,
    [OP_NOR] 0,
    [OP_LW] 0,
    [OP_SW] 0,
    [OP_BEQ] 1,
    [OP_JALR] 1,
    [OP_HALT] 0,
    [OP_NOOP] 0,
};

void printState(state_t *);

void OpADD(const inst_t *inst, state_t *state);
void OpNOR(const inst_t *inst, state_t *state);
void OpLW(const inst_t *inst, state_t *state);
void OpSW(const inst_t *inst, state_t *state);
void OpBEQ(const inst_t *inst, state_t *state);
void OpJALR(const inst_t *inst, state_t *state);

int main(int argc, char *argv[])
{
    string_t line;
    state_t state;
    inst_t inst;
    int numOfExecutedInsts = 0;

    FILE *filePtr;

    memset(&state, 0, sizeof state);

    if (argc != 2)
    {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    if ((filePtr = fopen(argv[1], "r")) == NULL)
    {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }

    // read in the entire machine-code file into memory.
    for (state.numMemory = 0; fgets(line, MAX_LINE_LENGTH, filePtr) != NULL; ++state.numMemory)
    {
        if (sscanf(line, "%d", state.mem + state.numMemory) != 1)
        {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }

        printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }

    while (1)
    {
        printState(&state);

        // Instruction Fetch & Decode
        inst.code = state.mem[state.pc];

        // Execute
        ++numOfExecutedInsts;
        switch (inst.o.opcode)
        {
        case OP_ADD:
            OpADD(&inst, &state);
            break;

        case OP_NOR:
            OpNOR(&inst, &state);
            break;

        case OP_LW:
            OpLW(&inst, &state);
            break;

        case OP_SW:
            OpSW(&inst, &state);
            break;

        case OP_BEQ:
            OpBEQ(&inst, &state);
            break;

        case OP_JALR:
            OpJALR(&inst, &state);
            break;

        case OP_HALT:
            ++state.pc;
            goto exit;

        case OP_NOOP:
            break;
        default:
            printf("error: invalid opcode\n");
            exit(1);
        }

        if (!PCMuxTable[inst.o.opcode])
            ++state.pc;
    }

exit:
    printf("machine halted\n");
    printf("total of %d instructions executed\n", numOfExecutedInsts);
    printf("final state of machine:");
    printState(&state);

    return 0;
}

void printState(state_t *state)
{
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", state->pc);

    printf("\tmemory:\n");
    for (i = 0; i < state->numMemory; ++i)
    {
        printf("\t\tmem[ %d ] %d\n", i, state->mem[i]);
    }

    printf("\tregisters:\n");
    for (i = 0; i < NUM_REGS; ++i)
    {
        printf("\t\treg[ %d ] %d\n", i, state->reg[i]);
    }
    printf("end state\n");
}

void OpADD(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->r.regA, regB = inst->r.regB, destReg = inst->r.destReg;

    state->reg[destReg] = state->reg[regA] + state->reg[regB];
}

void OpNOR(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->r.regA, regB = inst->r.regB, destReg = inst->r.destReg;

    state->reg[destReg] = !(state->reg[regA] | state->reg[regB]);
}

void OpLW(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->i.regA, regB = inst->i.regB;
    int offset = inst->i.offset;

    state->reg[regB] = state->mem[state->reg[regA] + offset];
}

void OpSW(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->i.regA, regB = inst->i.regB;
    int offset = inst->i.offset;

    state->mem[state->reg[regA] + offset] = state->reg[regB];
}

void OpBEQ(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->i.regA, regB = inst->i.regB;
    int offset = inst->i.offset;

    state->pc += 1 + offset * (state->reg[regA] == state->reg[regB]);
}

void OpJALR(const inst_t *inst, state_t *state)
{
    unsigned regA = inst->j.regA, regB = inst->j.regB;

    state->reg[regB] = state->pc + 1;
    state->pc = state->reg[regA];
}
