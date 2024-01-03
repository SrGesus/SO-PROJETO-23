#ifndef SERVER_PARSER_H
#define SERVER_PARSER_H

#include "session.h"

/// @brief Attempts to initialize register fifo pipe
/// @param register_fifo pointer where pipe file descriptor will be written
/// @param register_pipe_path path to pipe file
/// @return 0 if sucessful, 1 otherwise
int initialize_pipe(int* register_fifo, const char* register_pipe_path);

/// @brief Reads from register fifo into session struct
/// @param session Pointer to struct to store session data.
/// @param register_fifo pipe file descriptor
/// @return 0 if sucessful, 1 otherwise
int initiate_session(session_t* session, int register_fifo);

/// @brief Parses and runs create operation.
/// @param session
/// @return 0 if sucessful, 1 otherwise
int parse_create(session_t* session);

/// @brief Parses and runs reserve operation.
/// @param session
/// @return 0 if sucessful, 1 otherwise
int parse_reserve(session_t* session);

/// @brief Parses and runs show operation.
/// @param session
/// @return 0 if sucessful, 1 otherwise
int parse_show(session_t* session);

/// @brief Parses and executes a single operation
/// @param session pointer to current session
/// @return -1 if operation was QUIT, 0 if operation sucessful, 1 otherwise;
int parse_operation(session_t* session);

#endif  // SERVER_PARSER_H
