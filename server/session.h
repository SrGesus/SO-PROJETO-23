#ifndef SERVER_SESSION_H
#define SERVER_SESSION_H

typedef struct {
    unsigned int id;
    int request_pipe;
    int response_pipe;
} session_t;

#endif // SERVER_SESSION_H
