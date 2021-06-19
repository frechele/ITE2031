#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_INSTRUCTION 1024
#define MAX_LINE_LENGTH 1000
typedef char string_t[MAX_LINE_LENGTH];

int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(const char *);

void procFirstPass(FILE *, FILE *);
void procSecondPass(FILE *, FILE *);

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

struct
{
    struct
    {
        string_t label;
        int addr;
    } labels[MAX_INSTRUCTION];

    int numAddrs;
} labelTable;

int findLabelAddress(const char *label);

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
    procFirstPass(inFilePtr, outFilePtr);

    // this is how to rewind the file ptr so that you start reading from the beginning of the file.
    rewind(inFilePtr);

    // translate to machine code
    procSecondPass(inFilePtr, outFilePtr);

    fclose(inFilePtr);
    fclose(outFilePtr);

    return 0;
}

void procFirstPass(FILE *inFilePtr, FILE *outFilePtr)
{
    string_t label, temp;
    int curAddr;

    labelTable.numAddrs = 0;

    for (curAddr = 0; readAndParse(inFilePtr, label, temp, temp, temp, temp); ++curAddr)
    {
        if (strlen(label) > 0)
        {
            if (findLabelAddress(label) != -1)
            {
                printf("error: duplicate label\n");

                fclose(inFilePtr);
                fclose(outFilePtr);
                exit(1);
            }

            strncpy(labelTable.labels[labelTable.numAddrs].label, label, MAX_LINE_LENGTH);
            labelTable.labels[labelTable.numAddrs].addr = curAddr;

            ++labelTable.numAddrs;
        }
    }
}

enum ErrorCode
{
    ERR_OK,
    ERR_UNRECOGNIZED_OPCODE,
    ERR_UNDEFINED_LABEL,
    ERR_NOT_ENOUGH_ARGUMENTS,
    ERR_INVALID_ARGUMENT,
    ERR_ARG_OVERFLOW,
};

int procRType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char * arg2, inst_t *inst, int* errArg);
int procIType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char * arg2, inst_t *inst, int* errArg);
int procJType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char * arg2, inst_t *inst, int* errArg);
int procOType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char * arg2, inst_t *inst, int* errArg);

void procSecondPass(FILE *inFilePtr, FILE *outFilePtr)
{
    int curAddr, tmpAddr, errArg;
    inst_t inst;
    string_t label, opcode, arg0, arg1, arg2;

    for (curAddr = 0; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); ++curAddr)
    {
        if (curAddr)
        {
            fputc('\n', outFilePtr);
        }

        memset(&inst, 0, sizeof inst);
        tmpAddr = ERR_OK;
        errArg = -1;

        if (strcmp(opcode, ".fill") == 0)
        {
            if (strlen(arg0) == 0)
            {
                tmpAddr = ERR_NOT_ENOUGH_ARGUMENTS;

                goto bad;
            }

            if (isNumber(arg0))
            {
                fprintf(outFilePtr, "%d", atoi(arg0));
            }
            else
            {
                if ((tmpAddr = findLabelAddress(arg0)) == -1)
                {
                    tmpAddr = ERR_UNDEFINED_LABEL;
                    errArg = 0;

                    goto bad;
                }

                fprintf(outFilePtr, "%d", tmpAddr);
            }
        }
        else
        {
            if (strcmp(opcode, "add") == 0)
                tmpAddr = procRType(OP_ADD, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "nor") == 0)
                tmpAddr = procRType(OP_ADD, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "lw") == 0)
                tmpAddr = procIType(OP_LW, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "sw") == 0)
                tmpAddr = procIType(OP_SW, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "beq") == 0)
                tmpAddr = procIType(OP_BEQ, curAddr, arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "jalr") == 0)
                tmpAddr = procJType(OP_JALR,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "halt") == 0)
                tmpAddr = procOType(OP_HALT,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else if (strcmp(opcode, "noop") == 0)
                tmpAddr = procOType(OP_NOOP,curAddr,  arg0, arg1, arg2, &inst, &errArg);
            else
                tmpAddr = ERR_UNRECOGNIZED_OPCODE;

            if (tmpAddr != ERR_OK)
                goto bad;

            fprintf(outFilePtr, "%u", inst.code);
        }
    }

    return;

bad:
    if (tmpAddr == ERR_NOT_ENOUGH_ARGUMENTS)
    {
        printf("error: not enought arguments\n");
    }
    else if (tmpAddr == ERR_UNDEFINED_LABEL)
    {
        printf("error: undefined label\n");
        switch (errArg)
        {
        case 0:
            printf("%s", arg0);
            break;
        case 1:
            printf("%s", arg1);
            break;
        case 2:
            printf("%s", arg2);
            break;
        }
        printf("\n");
    }
    else if (tmpAddr == ERR_INVALID_ARGUMENT)
    {
        printf("error: invalid argument\n");
        switch (errArg)
        {
        case 0:
            printf("%s", arg0);
            break;
        case 1:
            printf("%s", arg1);
            break;
        case 2:
            printf("%s", arg2);
            break;
        }
        printf("\n");
    }
    else if (tmpAddr == ERR_UNRECOGNIZED_OPCODE)
    {
        printf("error: unrecognized opcode\n%s\n", opcode);
    }
    else if (tmpAddr == ERR_ARG_OVERFLOW)
    {
        printf("error: argument overflow\n");
    }

    fclose(inFilePtr);
    fclose(outFilePtr);
    exit(1);
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
int isNumber(const char *string)
{
    int i;

    return (sscanf(string, "%d", &i) == 1);
}

// returns 1 if value is out of range.
int checkOffsetRange(const char *string)
{
    int val = atoi(string);

    return (val > 32767) || (val < -32768);
}

// return -1 if there is no consistent label.
int findLabelAddress(const char *label)
{
    int i;
    for (i = 0; i < labelTable.numAddrs; ++i)
    {
        if (strcmp(label, labelTable.labels[i].label) == 0)
            return labelTable.labels[i].addr;
    }

    return -1;
}

int labelOrImmediate(int labelOffset, const char *arg, enum ErrorCode *err)
{
    int addr;

    if (isNumber(arg))
    {
        return atoi(arg);
    }

    if ((addr = findLabelAddress(arg)) == -1)
    {
        *err = ERR_UNDEFINED_LABEL;
        return -1;
    }

    return addr - labelOffset;
}

int procRType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char* arg2, inst_t *inst, int* errArg)
{
    if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    if (!isNumber(arg0))
    {
        *errArg = 0;
        return ERR_INVALID_ARGUMENT;
    }

    if (!isNumber(arg1))
    {
        *errArg = 1;
        return ERR_INVALID_ARGUMENT;
    }

    if (!isNumber(arg2))
    {
        *errArg = 2;
        return ERR_INVALID_ARGUMENT;
    }

    inst->r.opcode = opcode;
    inst->r.regA = atoi(arg0);
    inst->r.regB = atoi(arg1);
    inst->r.destReg = atoi(arg2);

    return ERR_OK;
}

int procIType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char* arg2, inst_t *inst, int* errArg)
{
    enum ErrorCode err = ERR_OK;

    if (strlen(arg0) == 0 || strlen(arg1) == 0 || strlen(arg2) == 0)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    if (!isNumber(arg0))
    {
        *errArg = 0;
        return ERR_INVALID_ARGUMENT;
    }

    if (!isNumber(arg1))
    {
        *errArg = 1;
        return ERR_INVALID_ARGUMENT;
    }

    if (isNumber(arg2) && checkOffsetRange(arg2))
    {
        *errArg = 2;
        return ERR_ARG_OVERFLOW;
    }

    inst->i.opcode = opcode;
    inst->i.regA = atoi(arg0);
    inst->i.regB = atoi(arg1);
    inst->i.offset = labelOrImmediate((opcode == OP_BEQ ? curAddr + 1 : 0), arg2, &err);

    // maybe error is in arg2
    if (err != ERR_OK)
        *errArg = 2;

    return err;
}

int procJType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char* arg2, inst_t *inst, int* errArg)
{
    if (strlen(arg0) == 0 || strlen(arg1) == 0)
        return ERR_NOT_ENOUGH_ARGUMENTS;

    if (!isNumber(arg0))
    {
        *errArg = 0;
        return ERR_INVALID_ARGUMENT;
    }

    if (!isNumber(arg1))
    {
        *errArg = 1;
        return ERR_INVALID_ARGUMENT;
    }

    inst->j.opcode = opcode;
    inst->j.regA = atoi(arg0);
    inst->j.regB = atoi(arg1);

    return ERR_OK;
}

int procOType(enum OpCode opcode, int curAddr, const char *arg0, const char *arg1, const char* arg2, inst_t *inst, int* errArg)
{
    inst->o.opcode = opcode;

    return ERR_OK;
}
