#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INSTRUCTION 1024
#define MAX_LINE_LENGTH 1000
typedef char string_t[MAX_LINE_LENGTH];

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(char *);

void procFirstPass(FILE*);
void procSecondPass(FILE*, FILE*);

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
    OP_ADD  = 0b000,
    OP_NOR  = 0b001,
    OP_LW   = 0b010,
    OP_SW   = 0b011,
    OP_BEQ  = 0b100,
    OP_JALR = 0b101,
    OP_HALT = 0b110,
    OP_NOOP = 0b111
};

/*
 * Common Codes - End
 */

struct
{
    struct {
        string_t label;
        int addr;
    } labels[MAX_INSTRUCTION];

    int numAddrs;
} labelTable;

int findLabelAddress(const char* label);

int main(int argc, char *argv[])
{
    char *inFileString;
    char *outFileString;
    FILE *inFilePtr;
    FILE *outFilePtr;

    if (argc != 3)
    {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    if ((inFilePtr = fopen(inFileString, "r")) == NULL)
    {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    if ((outFilePtr = fopen(outFileString, "w")) == NULL)
    {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // calculate address of each symbolic label.
    procFirstPass(inFilePtr);

    // this is how to rewind the file ptr so that you start reading from the beginning of the file.
    rewind(inFilePtr);

    // translate to machine code
    procSecondPass(inFilePtr, outFilePtr);

    return 0;
}

void procFirstPass(FILE* inFilePtr)
{
    string_t label, temp;
    int curAddr;

    labelTable.numAddrs = 0;

    for (curAddr = 0; readAndParse(inFilePtr, label, temp, temp, temp, temp); ++curAddr)
    {
        if (strlen(label) > 0)
        {
            strncpy(labelTable.labels[labelTable.numAddrs].label, label, MAX_LINE_LENGTH);
            labelTable.labels[labelTable.numAddrs].addr = curAddr;

            ++labelTable.numAddrs;
        }
    }
}

void procSecondPass(FILE* inFilePtr, FILE* outFilePtr)
{
    string_t label, opcode, arg0, arg1, arg2;

    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        
    }
}

int findLabelAddress(const char* label)
{
    int i;
    for (i = 0; i < labelTable.numAddrs; ++i)
    {
        if (strcmp(label, labelTable.labels[i].label) == 0)
            return labelTable.labels[i].addr;
    }

    return -1;
}

/*
 * Read and parse a line of the assembly-language file. Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0, char *arg1, char *arg2)
{
    string_t line;
    char *ptr = line;

    // delete prior values.
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    // read the line from the assembly-language file.
    if (fgets(line, MAX_LINE_LENGTH, inFilePtr) == NULL)
    {
        // reached end of file.
        return 0;
    }

    // check for line too long (by looking for a \n).
    if (strchr(line, '\n') == NULL)
    {
        // line too long.
        printf("error: line too long\n");
        exit(1);
    }

    // is there a label?
    ptr = line;
    if (sscanf(ptr, "%[^\t\n\r ]", label))
    {
        // successfully read label; advance pointer over the label.
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line. Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
    return 1;
}

// return 1 if string is a number.
int isNumber(char *string)
{
    int i;

    return (sscanf(string, "%d", &i) == 1);
}
