#ifndef RM_ASM_H
#define RM_ASM_H 1
#define _DEFAULT_SOURCE
#define _
// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h> // isdigit(), etc

#define REG_E 9
#define INT_E 16

// instruction codes
#define _INP 0x10 // REG
#define _PRT 0x11 // REG
#define _LD  0x14 // INT,REG
#define _MOV 0x18 // REG,REG
#define _ADD 0x19 // REG,REG
#define _SUB 0x1A // REG,REG
#define _MUL 0x1B // REG,REG
#define _DIV 0x1C // REG,REG
#define _MOD 0x1D // REG,REG
#define _JMP 0x08 // INT
#define _CMP 0x1E // REG,REG
#define _JEQ 0x09 // INT
#define _JNE 0x0A // INT
#define _JGE 0x0B // INT
#define _JNG 0x0C // INT
#define _JLT 0x0D // INT
#define _JNL 0x0E // INT
typedef char * string;
typedef enum { REG, INT } erange_t;
enum arg_type { SREG, DREG, SINT, INT_REG, NONE };
typedef enum {
    STP = 0xff, // NONE arg type
    INP = 0x01, // SREG arg type
    PRT = 0x02,
    JMP = 0x10, // SINT arg type
    JEQ = 0x11,
    JNE = 0x12,
    JGE = 0x13,
    JNG = 0x14,
    JLT = 0x15,
    JNL = 0x16,
    MOV = 0x20, // DREG arg type
    ADD = 0x21,
    SUB = 0x22,
    MUL = 0x23,
    DIV = 0x24,
    MOD = 0x25,
    CMP = 0x26,
    LD  = 0x30  // INT_REG arg type
    } opcode_t;
const char OP_N = 18;
const string opstr[] = {
    "STP",
    "INP",
    "PRT",
    "JMP",
    "JEQ",
    "JNE",
    "JGE",
    "JNG",
    "JLT", 
    "JNL",
    "MOV",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "CMP",
    "LD"
};

typedef struct { 
    opcode_t opcode;
    enum arg_type type;
    short args[2];
} instruction_t;


// Structs for each instruction argtype.
// Ops with different argtypes have different 
// length, therefore separate structs.
// __packed__ disables padding structs to full bytes 
//=================================================
//                      UNUSED
//=================================================
/*
typedef struct __attribute__((__packed__)){
    unsigned short opcode : 5;
    unsigned short reg   : 9;
} bin_op_reg1;

typedef struct  __attribute__((__packed__)){
    unsigned short opcode : 5;
    unsigned short reg1  : 9;
    unsigned short reg2  : 9;
} bin_op_reg2;

typedef struct  __attribute__((__packed__)){
    unsigned short opcode : 5;
    short int_arg : 16;
    unsigned short reg   : 9;
} bin_op_int_reg;

typedef struct  __attribute__((__packed__)){
    unsigned short opcode : 5;
    unsigned short int_arg : 16;
} bin_op_int;

typedef union {
    bin_op_reg1     op_reg1;
    bin_op_reg2     op_reg2;
    bin_op_int_reg  op_int_reg;
    bin_op_int      op_int;
} bin_op_t;

typedef union {
    short int_arg;
    unsigned int reg : 9;
} op_arg_t;

void bin_op_reset(bin_op_t* b)
{   // set all largest struct's memebers to 0
    // this ensures all union's bits are set to 0
    b->op_int_reg.opcode = 0;
    b->op_int_reg.int_arg = 0;
    b->op_int_reg.reg = 0;
}
*/
instruction_t *new_instruction(opcode_t code)
{
    instruction_t *op = malloc(sizeof(instruction_t));
    op->opcode = code;
    switch (code)
    {   // set argument type appropriately
        case INP: case PRT:
            op->type = SREG; 
            break;

        case LD: 
            op->type = INT_REG;
            break;
        
        case MOV: case ADD: case SUB:
        case MUL: case DIV: case MOD:
        case CMP:
            op->type = DREG;
            break;
        
        case JMP: case JEQ: case JNE:
        case JGE: case JNG: case JLT:
        case JNL:
            op->type = SINT;
            break;
        case STP:
            op->type = NONE;
            break;
        default:    // invalid opcode
            free(op);
            return NULL;
            break;
    }
    return op;
}

instruction_t STOP = {STP};
static string header = "AKiSO ASM";
static string footer = "STOP";
//{'A', 'K', 'i', 'S', 'O', ' ', 'A', 'S', 'M'};
//{ x41, 0x4b, 0x69, 0x53, 0x4f, 0x20, 0x41, 0x53, 0x4d}

// misc functions
void str_to_upper(string str)
{ 
    int i = 0;
    while(str[i] != '\0')
    { 
        str[i] = toupper(str[i]);
        i++;
    }
}

int is_num(string s)
{   // return 1 if s is a number (only contains decimal digits)
    int i = 0;
    if(s[i] == '-')
        i++;
    while(s[i] != '\0')
    {
        if(!isdigit(s[i]))
                return 0;
        i++;
    }
    return 1;
}

int is_instr(string str)
{   // return 1 is str is a correct instruction (not case-sensitive)
    string s = malloc(sizeof(char) * (strlen(str)+1));
    strcpy(s, str);
    str_to_upper(s); 
    return ! (strcmp(s, "INP")
           && strcmp(s, "PRT")
           && strcmp(s, "LD" )
           && strcmp(s, "MOV")
           && strcmp(s, "ADD")
           && strcmp(s, "SUB")
           && strcmp(s, "MUL")
           && strcmp(s, "DIV")
           && strcmp(s, "MOD")
           && strcmp(s, "JMP")
           && strcmp(s, "CMP")
           && strcmp(s, "JEQ")
           && strcmp(s, "JNE")
           && strcmp(s, "JGE")
           && strcmp(s, "JNG")
           && strcmp(s, "JLT")
           && strcmp(s, "JNL")
           && strcmp(s, "STP"));
}


// write operation to file and ensure it got written
void write_op(FILE* file, instruction_t *op)
{ 
    if(file == NULL)
    {
        fprintf(stderr, "Error: Output file is NULL\n");
        exit(EXIT_FAILURE);
    }
    if(op == NULL)
    {
        fprintf(stderr, "Error: Trying to write NULL instruction\n");
        exit(EXIT_FAILURE);
    }
    int write_st[3] = {0}; // write status for each write
    int two_args = (op->type == DREG || op->type == INT_REG);
    write_st[0] = fwrite(&(op->opcode), sizeof(char), 1, file);
    if(op->opcode != STP)
        write_st[1] = fwrite(&(op->args[0]), sizeof(short), 1, file);
    if(two_args)
        write_st[2] = fwrite(&(op->args[1]), sizeof(short), 1, file);

    if(write_st[0] == 0 || (op->opcode != STP && (write_st[1] == 0 ||
        (two_args && write_st[2] == 0))))
    {
        perror("write_op");
        fprintf(stderr, "Error writing to file\n");
        exit(EXIT_FAILURE);
    }
}

// get opcode matching the instruction in str
unsigned char get_instr(string str)
{
    char *instr = malloc(sizeof(char) * (strlen(str)+1));
    strcpy(instr, str);
    str_to_upper(instr);
    opcode_t opcode = 0x00;
    if (strcmp(instr, "INP") == 0)
        opcode = INP;
    else if (strcmp(instr, "PRT") == 0)
        opcode = PRT;
    else if (strcmp(instr, "LD" ) == 0)
        opcode = LD;
    else if (strcmp(instr, "MOV") == 0)
        opcode = MOV;
    else if (strcmp(instr, "ADD") == 0)
        opcode = ADD;
    else if (strcmp(instr, "SUB") == 0)
        opcode = SUB;
    else if (strcmp(instr, "MUL") == 0)
        opcode = MUL;
    else if (strcmp(instr, "DIV") == 0)
        opcode = DIV;
    else if (strcmp(instr, "MOD") == 0)
        opcode = MOD;
    else if (strcmp(instr, "JMP") == 0)
        opcode = JMP;
    else if (strcmp(instr, "CMP") == 0)
        opcode = CMP;
    else if (strcmp(instr, "JEQ") == 0)
        opcode = JEQ;
    else if (strcmp(instr, "JNE") == 0)
        opcode = JNE;
    else if (strcmp(instr, "JGE") == 0)
        opcode = JGE;
    else if (strcmp(instr, "JNG") == 0)
        opcode = JNG;
    else if (strcmp(instr, "JLT") == 0)
        opcode = JLT;
    else if (strcmp(instr, "JNL") == 0)
        opcode = JNL;
    else if (strcmp(instr, "STP") == 0)
        opcode = STP;
    else
    {
        fprintf(stderr, "invalid instruction: %s (%s)\n", instr, str);
        exit(EXIT_FAILURE);
    }
    return opcode;
}

// print opcode's name
string get_opstr(opcode_t op)
{
    for(int i = 0; i < OP_N; i++)
        if(get_instr(opstr[i]) == op)
            return(opstr[i]);
    return NULL; 
}
#endif

