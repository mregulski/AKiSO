#include "reg_m.h"
#include <byteswap.h>
#include <limits.h>
#include <errno.h>
void write_predef(FILE *trg,  char predef[], int len)
{ 
    if(fwrite(predef, sizeof(unsigned char), len, trg) == 0)
    {
        fprintf(stderr, "Error wrting file header\n");
        exit(EXIT_FAILURE);
    }
}

void fclean(int exit_status, void *arg)
{
    if(arg != NULL)
    fclose((FILE*)arg);
}

void unknown_error(int where, string token)
{
    fprintf(stderr, "[line %d] unknown error.\n", where);
    exit(EXIT_FAILURE);
}

void syntax_error(int where, string token)
{   // exit program with a 'syntax error' message
    fprintf(stderr, "[line %d] unexpected '%s'\n", where, token);
    exit(EXIT_FAILURE);
}

void range_error(int where, erange_t type, string token)
{
    if (type == REG)
    {
        fprintf(stderr, 
            "[line %d] register number must be in [0..511] range.\n", 
            where);
    }
    else if (type == INT)
    {
        fprintf(stderr,
                "[line %d] integer argument must fit in a C short type.\n",
                where);
    }
    else
        fprintf(stderr, "[line %d] unknown range error.\n", where);
    
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
    char *in_name,      
         *out_name, 
         *tmp,              
         *line_buf = NULL,  // for getline() w/ auto-alloc
         *tok;              // current token
         
    instruction_t *op = 0;
//    bin_op_t bin_op ;       // container for converted (binary) operations
//    op_arg_t arg;           // container for instruction arguments
   
    size_t len = 0;         // for getline() w/ auto-alloc
    
    int stat = 1,           // getline() status
        line_n = 0,         // current line number
        arg_n = 0;          // argument counter

// process arguments
    if(argc != 2)
    {
        puts("USAGE: /path/rma <filename>.rma");
        exit(EXIT_FAILURE);
    }
    in_name = argv[1];
    
    // get output filename (in_name + .aks)
    int slen = strlen(in_name);
    out_name = malloc((strlen(in_name) + 4) * sizeof(char));
    strncpy(out_name, in_name, slen);
    strcat(out_name, ".aks");

// check in- and output files
    FILE *f_in = fopen(in_name, "rt");
    if(f_in == NULL)
    {
        fprintf(stderr, "error opening source file (%s)\n", in_name);
        exit(EXIT_SUCCESS);
    }

    FILE *f_out = fopen(out_name, "wt");
    if(f_out == NULL)
    {
        fprintf(stderr, "error opening output file (%s)\n", out_name);
        exit(EXIT_SUCCESS);
    }
    on_exit(fclean, f_in);
    on_exit(fclean, f_out);    // register 
    write_predef(f_out, header, 9);    
    
    printf("source : %s\n", in_name);

// start parsing
    while(stat > 0) // read file line by line
    {
        stat = getline(&line_buf, &len, f_in);
        line_n++;
        tok = (char *) strtok_r(line_buf, ", \t\n", &tmp);
        op = NULL;              // ensure op and bin_op are clean
        
// parse line: one instruction per line
        while (tok != NULL && tok[0] != '\0' )
        {   //  get tokens from each line
            if (is_instr(tok))
            {   // conver t menmonic to code if it's OK
                if(op != NULL) // instruction already found in line
                    syntax_error(line_n, tok);
                op = new_instruction(get_instr(tok));
                                
            }
            else if (is_num(tok))
            {   // place number if appropriate
                if(op == NULL) // can't have number before opcode
                    syntax_error(line_n, tok);
                
                errno = 0;
                long a = strtol(tok, NULL, 10);
                if(errno != 0)
                {
                    perror("strtol");
                    exit(EXIT_FAILURE);
                }

// Check argument count
                if(arg_n > 2)
                    syntax_error(line_n, tok);

                if(op->type == SREG || op->type == SINT)
                    if(arg_n > 0)
                        syntax_error(line_n, tok);
                
                if(op->type == DREG || op->type == INT_REG)
                    if(arg_n > 1)
                        syntax_error(line_n, tok);

// Check argument range
                if(op->type == SREG || op->type == DREG
                    || (op->type == INT_REG && arg_n == 1))
                {
                    if(a >= 512 || a < 0)
                        range_error(line_n, REG, tok);
                    op->args[arg_n] = a;
                }

                if(op->type == SINT || (op->type == INT_REG && arg_n==0))
                {
                    if(a >= SHRT_MAX || a <= SHRT_MIN)
                        range_error(line_n, INT, tok);
                    op->args[arg_n] = a;
                }

                // Place arguments where they should be                
                arg_n++;

            }
            else
                break;
                //if(op == NULL)
                //    unknown_error(line_n, tok);
                //otherwise ignore, aka comment
                //else
                //    break;

            tok = (char *) strtok_r(NULL, " \t\n,", &tmp);
        }
        arg_n = 0;
        line_buf = NULL;
        if(op != NULL) // it's not an empty pass, aka final newline or sth
            write_op(f_out, op);
    }
    write_op(f_out, &STOP);
    free(line_buf);
    free(tok);
    printf("OK.\n");
    return 0;
}

