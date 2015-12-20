#include <unistd.h>
#include <syscall.h> // read, write?
#include <stdarg.h> // va_arg etc
#include <stdio.h> //for checking
#include <stdlib.h> // *alloc()
#include <math.h> // num length

typedef unsigned char bool;

unsigned int stringLen(char *str);
bool iswhitespace(int c);
bool isbdigit(int c);
bool isddigit(int c);
bool ishdigit(int c);

void clearBuffer(char *buf, int length);
// number conversion
long strToInt(char *str, unsigned short base);
char *intToStr(long num, unsigned short base);
// simple wrapper for write();
void print(char *str);
void printc(char ch);

int readc();
// return number of non-null characters in st
// (so str[stringLen(str)] == '\0')
unsigned int stringLen(char *str)
{
    int length = 0;
    while(str[length] != '\0')
        length++;
    return length;
}
#define MYSCANF_BUF_SIZE 1024
#define FORMAT_TAG '%'
// read input (chunks separated by \n or EOF) and parse it
void myscanf(char* format, ...)
{
    va_list ap;
    int fmt_i;
    int fmt_len = stringLen(format);
    char *s;
    va_start(ap, format);
    while (*format)
    {
        // try to find format specifiers
        if (*format == '%')
        {
            *format++;
            switch(*format)
            {
                case 'b':
                {
                    char cur_buf[MYSCANF_BUF_SIZE]={0};
                    int ch,i=0;
                    int j = 0;
                    while((ch=readc())!=EOF && !iswhitespace(ch) && i < MYSCANF_BUF_SIZE)
                        if(isbdigit(ch))
                        {
                            cur_buf[i]=(char)ch;
                            i++;
                        }
                    if (i==0)
                        break;
                    *va_arg(ap, long*) = strToInt(cur_buf, 2);
                    break;
                }
                case 'd':
                {
                    char cur_buf[MYSCANF_BUF_SIZE]={0};
                    int ch, i = 0;
                    int j = 0;
                    while((ch = readc())!= EOF && !iswhitespace(ch))
                        if(isddigit(ch))
                        {
                            cur_buf[i] = (char)ch;
                            i++;
                            if(i >= MYSCANF_BUF_SIZE)
                                break;
                        }
                        
                    if (i==0)
                        break;
                    *va_arg(ap, long*) = strToInt(cur_buf, 10);

                    break;
                }
                case 's':
                {
                    // read a string
                    s = (char *) va_arg(ap, char *);
                    read(0,s,MYSCANF_BUF_SIZE);
                    break;
                    /*
                    char *cur_buf = malloc(MYSCANF_BUF_SIZE *sizeof(char));
                    int ch, i = 0;
                    int j = 0;
                    while((ch = readc())!=EOF && !iswhitespace(ch)  && i < MYSCANF_BUF_SIZE)
                    {
                        cur_buf[i] = (char)ch;
                        i++;
                    }
                    if (i==0)
                        break;
                    *(va_arg(ap, char*)) =  cur_buf;
                    break;*/
                }
                case 'x':
                {
                    char cur_buf[MYSCANF_BUF_SIZE]={0};
                    int ch, i = 0;
                    int j = 0;
                    while((ch = readc())!= EOF && !iswhitespace(ch))
                        if(ishdigit(ch))
                        {
                            cur_buf[i] = (char)ch;
                            i++;
                            if(i >= MYSCANF_BUF_SIZE)
                                break;
                        }
                        
                    if (i==0)
                        break;
                    *va_arg(ap, long*) = strToInt(cur_buf, 16);

                    break;
                }
                
            }
        }
        *format++;
    }
    va_end(ap);
}



#define MYPRINTF_BUF_SIZE 1024
void myprintf(char* format, ...)
{
    va_list ap;
    char buffer[MYPRINTF_BUF_SIZE+1] = {'\0'};
    int buf_i = 0;
    int fmt_i = 0;
    int fmt_len = stringLen(format);
    // values that will be printed
    int arg_int;
    char *arg_str;
    char *converted_num;

    va_start(ap, format);
    // loop directly through the format string until we hit null/'\0'
    for (fmt_i = 0; fmt_i < fmt_len; fmt_i++)
    { 
        if (format[fmt_i] != FORMAT_TAG)
        {    
            // Don't write characters one-by-one, group by buffer size
            // Buffer is written to stdout & cleared when:
            // a) next character is terminator (\0)
            // b) next character isa FORMAT_TAG
            // c) it's full
            buffer[buf_i] = format[fmt_i];
            buf_i++;
            if (buf_i >= MYPRINTF_BUF_SIZE || format[fmt_i+1] == '\0' ||
                    format[fmt_i+1] == FORMAT_TAG)
            {
                print(buffer);
                clearBuffer(buffer, MYPRINTF_BUF_SIZE);
                buf_i = 0;
            }
            continue;
        }
        else
        { 
            print(buffer);
            clearBuffer(buffer, MYPRINTF_BUF_SIZE);
            // format[fmt] is '%', check what it wants and print it
            switch(format[fmt_i+1])
            { 
                // print next argument from list as binary number
                case 'b':
                    arg_int = va_arg(ap, int);
                    converted_num = intToStr(arg_int, 2);
                    print("0b");
                    print(converted_num);
                    free(converted_num);
                    break;
                // decimal
                case 'd':
                    arg_int = va_arg(ap, int);
                    converted_num = intToStr(arg_int, 10);
                    print(converted_num);
                    free(converted_num);
                    break;
                // string
                case 's':
                    arg_str = va_arg(ap, char*);
                    print(arg_str);
                    break;
                // hex
                case 'x':
                    arg_int = va_arg(ap, int);
                    converted_num = intToStr(arg_int, 16);
                    print("0x");
                    print(converted_num);
                    free(converted_num);
                    break;
                // unknown format specified
                default:
                    printc(format[fmt_i+1]);
                    break;
            }
            fmt_i++;
        }

    }
    va_end(ap);
}
int main(int argc, char *argv[])
{
    myprintf("This is output of myprintf() without formats\n");
    
    myprintf("\nNow using myprintf() with single format\n");
    myprintf("dec: %d\n",91236);
    myprintf("bin: %b\n",91236);
    myprintf("hex: %x\n",91236);

    myprintf("\nNow more formats (at once)!\n");
    myprintf("dec: %d, bin: %b, hex: %x\n", 91236, 91236, 91236);
    myprintf("\nString format test:\n");
    char *word = "word ";
    myprintf("%s%s%s%s%s","each ",word,"is ","printed ","separately\n");
    myprintf("unknown format: %q more text\n");
    
    printf("\n==============================\n");
    
    myprintf("myscanf() test\n");
    
    long d = 0;
    long b = 0;
    myprintf("Single format: decimal\n");
    myscanf("%d",&d);
    myprintf("dec: %d\n", d);

    myprintf("binary\n");
    myscanf("%b",&b);
    myprintf("bin: %b\n", b);

    myprintf("hex\n");
    long hx;
    myscanf("%x", &hx);
    myprintf("hex: %x\n", hx);
    
    myprintf("str\n");
    char *str = malloc(sizeof(char)*MYSCANF_BUF_SIZE);
    myscanf("%s", str);
    myprintf("str: %s\n", str);

    myprintf("Two dec\n");
    long a1,a2 = 0;
    myscanf("%d%d",&a1,&a2);
    myprintf("a1: %d, a2: %d\n",a1,a2);
    return 0;
}

// converts num to base b representqation (as string), in reverse order
char *intToStr(long num, unsigned short base)
{
    if(base > 36 || base < 2)
        return "base must be in range {2..36}";

    // no number possible with over 64 chars, + 1 char for terminator
    char *buffer = calloc(65, sizeof(char));
    // ascii digits are (dec) codes 48-57 [0-9]
    // max base = 36
    const char * digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int pos = 63;
    int digit = 0;
    while(num != 0)
    {
        digit = num % base;
        num = num / base;
        buffer[pos] = digits[digit];
        pos--;
    }
    // trim & reverse the buffer for easy printing
    // and for making sense
    // and not being empty
    int numLen = 64-pos;
    char *result = calloc(numLen, sizeof(char));
    int i = 0;
    while(numLen - i >= 1)
    {
        result[numLen - 1 - i] = buffer[64-i];
        i++;
    }
    free(buffer);
    return result;
}

short getDigitValue(char digit)
{ 
    int i = 0;
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while(digits[i] != '\0')
    { 
        if(digits[i] == digit || digits[i] == digit-32)
            return i;
        i++;
    }
    return -1;

}

long strToInt(char *str, unsigned short base)
{
    
    int exp = 0;
    int res = 0;
    int i;
    for(int i = stringLen(str) - 1; i >= 0; i--)
    { 
        res += getDigitValue(str[i]) * pow((double)base, (double)exp);
        exp++;
    }
    return res;
}

bool iswhitespace(int c)
{
    return c == ' '
        || c == '\t'
        || c == '\n'
        || c == '\r'
        || c == '\v'
        || c == '\f';
}

bool isbdigit(int c)
{
    return c == '0' || c == '1';
}
bool isddigit(int c)
{
    return c == '1'
        || c == '2'
        || c == '3'
        || c == '4'
        || c == '5'
        || c == '6'
        || c == '7'
        || c == '8'
        || c == '9'
        || c == '0';
}

bool ishdigit(int c)
{
    return isddigit(c) || c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' 
    || c == 'F'|| c == 'a' || c == 'b' || c == 'c' || c == 'd' 
    || c == 'e' || c == 'f';
}
void clearBuffer(char *buf, int length)
{
    for(int i = 0; i <= length;i++)
        buf[i] = '\0';
}


// write entire string at once
void print(char *str)
{   
    int length = stringLen(str);   
    write(1, str, length*sizeof(char));
}
// write a single char
void printc(char ch)
{
    write(1, &ch, 1);
}

int readc()
{
    char res;
    read(0, &res, 1);
    return res;
}
