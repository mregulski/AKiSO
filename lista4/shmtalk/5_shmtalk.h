#define EMPTIED 0
#define FULL 1
#define MAX_CLIENTS 3
#define _XOPEN_SOURCE 700
typedef struct {
    int client_id;
    int status;
    char data[1024];
} Message;
