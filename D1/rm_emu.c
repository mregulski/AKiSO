#include "reg_m.h"
typedef short rm_addr;
typedef short rm_int;
typedef int reg_t;
typedef struct {
    unsigned char eq : 1;
    unsigned char gt : 1;
    unsigned char lt : 1;
} flag_reg;

typedef struct {
    unsigned int PC;        // program counter
    flag_reg F;             // flags
    reg_t registers[510];   // regular registers
} reg_list;

static reg_list registers;
enum opt {
//  OPT NAME      IDX      LONG             SHORT 
    O_VERBOSE  =  0,    // --verbose        -v
    O_DISASSM  =  1,    // --disassemble    -d
    O_STEP     =  2,    // --step           -s
};

static char opts[3] = { 0 };

// option checks
char m_verbose()    { return opts[O_VERBOSE]; }
char m_disassm()    { return opts[O_DISASSM]; }
char m_step()       { return opts[O_STEP]; }

void access_violation_error(short address, int type)
{
    string err_msg[] = {
        "ERROR [PC: %d] attempt to access protected register %d.\n",
        "ERROR [PC: %d] register address out of range: %d.\n",
        "ERROR [PC:%d] jump address out of range: %d.\n"};
    int msg_i;
    if(type == REG_E && address <= 1)
        msg_i = 0;
    if(type == REG_E && address >=512)
        msg_i = 1;
    if(type == INT_E && address < 0)
        msg_i = 2;

    fprintf(stderr, err_msg[msg_i], registers.PC, address);
    fprintf(stderr, "Program terminated\n");
    exit(EXIT_FAILURE);
}

void instruction_error(opcode_t op)
{
    fprintf(stderr, "[PC: %d] ERROR unknown instruction: %x."
            "Program terminated\n", registers.PC, op);
    exit(EXIT_FAILURE);
}

// provide safe access to registers
reg_t* reg(rm_addr address)
{
    if (address <= 1 || address >= 512)
        access_violation_error(address, REG_E);        
    return &(registers.registers[address-2]);   // offset because first 
                                                // 2 regs are not in array
}

void init_regs()
{
    registers.PC = 0;
    registers.F.eq = 0;
    registers.F.gt = 0;
    registers.F.lt = 0;
    // preload 4xx registers (xx := 32..90) with ASCII codes 32-90 (dec)
    for(int i = 432; i <= 490; i++)
        *reg(i) = i - 400;
    *reg(491) = 10;
}

/********************
 *  I/O operations  *
 ********************/

// get value from stdin and put it in register #addr
int rm_inp(rm_addr addr)
{
    char buf[6];
    short val;
    fgets(buf, 6, stdin);
    sscanf(buf, "%hd", &val);
    *reg(addr) = val;
    if(m_verbose() > 1)
        printf("INP: %d  -> reg#%d\n", val, addr);
    return 0;
}

// print value in register #addr to stdout
int rm_prt(rm_addr addr)
{ 
    printf("reg %d: ", addr);
    printf("0x%X", *reg(addr));
    if(*reg(addr) >= 32 && *reg(addr) <=90) // print some ascii as chars
        printf("(%c)", *reg(addr));
    printf("\n");
    return 0;
}

/**********************
 *  Jump operations  *
 *********************/

// unconditional jump: set PC to addr
int rm_jmp(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("JMP to %d\n", addr);
    registers.PC = addr;
    return 0;
}

// conditional jumps: jump according to state of F register
// if f[0] == 1 (EQUAL)
int rm_jeq(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 2)
        printf("F.eq = %d | JEQ to %d\n", registers.F.eq, addr);
    if(registers.F.eq)
        registers.PC = addr;
    return 0;

}
// if f[0] == 0 (NOT EQUAL)
int rm_jne(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("F.eq = %d | JNE to %d\n", registers.F.eq, addr);
    if(!registers.F.eq)
        registers.PC = addr;
    return 0;

}

// if f[1] == 1 (GREATER THAN)
int rm_jge(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("F.gt = %d | JGE to %d\n", registers.F.gt, addr);
    if(registers.F.gt)
        registers.PC = addr;
    return 0;
}

//if f[1] == 0 (LESS OR EQUAL)
int rm_jng(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("F.gt = %d | JNG to %d\n", registers.F.gt, addr);
    if(!registers.F.gt)
        registers.PC = addr;
    return 0;
}
// if f[2] == 1 (LESS THAN)
int rm_jlt(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("F.lt = %d | JLT to %d\n", registers.F.lt, addr);
    if(registers.F.lt)
        registers.PC = addr;
    return 0;
}

//if f[2] == 0 (GREATER OR EQUAL)
int rm_jnl(rm_int addr)
{
    if(addr < 0)
        access_violation_error(addr, INT_E);
    if(m_verbose() > 1)
        printf("F.lt = %d | JNL to %d\n", registers.F.lt, addr);
    if(!registers.F.lt)
        registers.PC = addr;
    return 0;
}

/*      END JUMP    */

// copy contents of src to dst
int rm_mov(rm_addr src, rm_addr dst)
{
    if(m_verbose() > 1)
        printf("%d = (%d) -> %d\n", src, *reg(src), dst);
    *reg(dst) = *reg(src);
    return 0;
}

/***************************
 *  Arithmetic operations  *
 ***************************/

// reg2 = reg2 + reg1
int rm_add(rm_addr reg1, rm_addr reg2)
{
    if(m_verbose() > 1)
        printf("%d = %d (%d) + %d (%d)\n", reg2, reg1, *reg(reg1),
                reg2, *reg(reg2));
    *reg(reg2) = *reg(reg1) + *reg(reg2);
    return 0;
}

// reg2 = reg2 - reg1
int rm_sub(rm_addr reg1, rm_addr reg2)
{   
    if(m_verbose() > 1)
        printf("%d = %d (%d) - %d (%d)\n", reg2, reg1, *reg(reg1),
                reg2, *reg(reg2));
    *reg(reg2) = *reg(reg2) - *reg(reg1);
    return 0;
}

// reg2 = reg2 * reg130(0)
int rm_mul(rm_addr reg1, rm_addr reg2)
{
    if(m_verbose() > 1)
        printf("%d = %d (%d) * %d (%d)\n", reg2, reg1, *reg(reg1),
                reg2, *reg(reg2));
    *reg(reg2) = *reg(reg2) * *reg(reg1);
    return 0;
}

// reg2 = reg2 / reg1 (integer division)
int rm_div(rm_addr reg1, rm_addr reg2)
{
    if(m_verbose() > 1)
        printf("%d = %d (%d) / %d (%d)\n", reg2, reg1, *reg(reg1),
                reg2, *reg(reg2));
   
                *reg(reg2) = *reg(reg2) / *reg(reg1);
    return 0;
}

// reg2 = reg2 % reg1
int rm_mod(rm_addr reg1, rm_addr reg2)
{
    if(m_verbose() > 1)
        printf("%hd = %hd (%d) %% %d (%d)\n", reg2, reg2, *reg(reg2),
                reg1, *reg(reg1));
    *reg(reg2) = *reg(reg2) % *reg(reg1);
    return 0;
}

/*      END ARITHMETIC      */

// compare reg1 and reg2 and set F register
int rm_cmp(rm_addr reg1, rm_addr reg2)
{
    registers.F.eq = *reg(reg1) == *reg(reg2);
    registers.F.gt = *reg(reg1) > *reg(reg2);
    registers.F.lt = *reg(reg1) < *reg(reg2);
    if(m_verbose())
    {
        printf("CMP %d, %d\n",*reg(reg1), *reg(reg2));
        printf("F: eq: %hhu, gt: %hhu, lt: %hhu\n", registers.F.eq, 
                registers.F.gt, registers.F.lt);
    }
    return 0;
}

// put val in register dst
int rm_ld(rm_int val, rm_addr dst)
{
    *reg(dst) = val;
    return 0;
}

void read_arg(short *arg, FILE* f)
{
    fseek(f, registers.PC+9, SEEK_SET);
    if((fread(arg, sizeof(short), 1, f) == 0) && ferror(f))
    {
        fprintf(stderr, "Error reading argument\n");
        exit(EXIT_FAILURE);
    }
    registers.PC += 2;
}

void read_op(opcode_t *op, FILE *f)
{
    fseek(f, registers.PC+9, SEEK_SET);
    if((fread(op, sizeof(char), 1, f) == 0) && ferror(f))
    {
        fprintf(stderr, "Error reading opcode\n");
        exit(EXIT_FAILURE);
    }
         registers.PC++;
}

void bprintf(char *bytes, int nbytes)
{
    for(int i = 0; i < nbytes; i++)
    {
        printf("%x ", bytes[i]);
    }
    printf("\n");
}

long filesize(FILE *f)
{
    long size;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f,0,SEEK_SET);
    return size;

}

int execute(opcode_t op, short arg1, short arg2);

void set_opt(enum opt option);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("usage: /path/rm_emu <program file>\n");
        exit(EXIT_FAILURE);
    }

    FILE *prog = fopen(argv[1], "rb");
    if (prog == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    int i = 2;
    while(argv[i] != NULL)
    {
        if(strcmp(argv[i], "-v") == 0 
            || strcmp(argv[i], "--verbose") == 0)
            set_opt(O_VERBOSE);
        else if(strcmp(argv[i], "-d") == 0 
            || strcmp(argv[i], "--disassemble") == 0)
            set_opt(O_DISASSM);
        else if(strcmp(argv[i], "-s") == 0
            || strcmp(argv[i], "--step") == 0)
            set_opt(O_STEP);
        i++;
    }
    short arg1;
    short arg2;
    opcode_t op;
    init_regs();
    long fsize = filesize(prog);
    //printf("filesize: %ld\n", fsize);
    char *head_buf = malloc(sizeof(char)*9);
    fread(head_buf, sizeof(char), 9, prog);

    //registers.PC = 9; 
    //bprintf(head_buf, 9);
    //printf("%s\n", head_buf);
    if(memcmp(head_buf, "AKiSO ASM", 9))
        printf("Specified file (%s) is not a proper program file.\n", 
                argv[1]);
    char pc_dsp[17]; // 6 chars + 1 NULL + 10 digits (max uint)

    while(ftell(prog) < fsize)  // go until EOF
    {
        // reset
        arg1 = 0;
        arg2 = 0;
        op = 0;
        // read & report
        read_op(&op, prog);
        // print PC of OP (-1 because reading op increments PC)
        if(registers.PC <= 99999)
            sprintf(pc_dsp, "[PC: %05d]",  registers.PC-1);
        else
            sprintf(pc_dsp, "[PC: %010d]", registers.PC-1);
        if(op != STP)
            read_arg(&arg1, prog); 
        if(op >= MOV && op < STP) // arg type is DREG or INT_REG
            read_arg(&arg2, prog); 
        if(m_verbose() > 0)
        {
            if (op >= MOV && op < STP)
                printf("%-17s %-3s %d, %d\n", pc_dsp, 
                        get_opstr(op), arg1, arg2);
            else if (op <= MOV)
                printf("%-17s %-3s %d \n", pc_dsp, get_opstr(op),
                        arg1);
            else if (op == STP)
                printf("%-17s %-3s\n", pc_dsp, get_opstr(op));
        }
        
        if(!m_disassm())
            execute(op, arg1, arg2);
            
        if(m_step())
            getchar();

    }
    return 0;
}

// actually just a wrapper for that huge switch
int execute(opcode_t op, short arg1, short arg2)
{
    switch(op)
    {
        case INP:
            return rm_inp(arg1);
        case PRT:
            return rm_prt(arg1);
        case JMP:
            return rm_jmp(arg1);
        case JEQ:
            return rm_jeq(arg1);
        case JNE:
            return rm_jne(arg1);
        case JGE:
            return rm_jge(arg1);
        case JNG:
            return rm_jng(arg1);
        case JLT:
            return rm_jlt(arg1);
        case JNL:
            return rm_jnl(arg1);
        case MOV:
            return rm_mov(arg1, arg2);
        case ADD:
            return rm_add(arg1, arg2);
        case SUB:
            return rm_sub(arg1, arg2);
        case MUL:
            return rm_mul(arg1, arg2);
        case DIV:
            return rm_div(arg1, arg2);
        case MOD:
            return rm_mod(arg1, arg2);
        case CMP:
            return rm_cmp(arg1, arg2);
        case LD:
            return rm_ld(arg1, arg2);
        case STP:
            exit(EXIT_SUCCESS);
        default:
            instruction_error(op);
    }
    return -1;
}

void set_opt(enum opt option)
{
    if(option == O_VERBOSE)
        opts[O_VERBOSE]++;      // allow multiple verbosity levels
    if(option == O_DISASSM)
    {
        opts[O_DISASSM] = 1;
        opts[O_VERBOSE] = opts[O_VERBOSE] == 0 ? 1 : opts[O_VERBOSE];
    }

    if(option == O_STEP)
    {
        opts[O_STEP] = 1;
        opts[O_VERBOSE] = opts[O_VERBOSE] == 0 ? 1 : opts[O_VERBOSE];
    }
        
}
