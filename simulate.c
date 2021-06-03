#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUM_MEMORY      65536 // maximum number of words in memory.
#define NUM_REGS        8     // number of machine registers.
#define MAX_LINE_LENGTH 1000
typedef char string_t[MAX_LINE_LENGTH];

typedef struct
{
    int pc;
    int mem[NUM_MEMORY];
    int reg[NUM_REGS];
    int numMemory;
} state_t;

void printState(state_t*);

int main(int argc, char* argv[])
{
    string_t line;
    state_t state;

    FILE* filePtr;

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

    return 0;
}

void printState(state_t* state)
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