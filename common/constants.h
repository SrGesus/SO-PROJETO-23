#define MAX_RESERVATION_SIZE 256
#define STATE_ACCESS_DELAY_US 500000  // 500ms
#define MAX_JOB_FILE_NAME_SIZE 256
#define MAX_SESSION_COUNT 1

#define MAX_PIPE_PATH_SIZE 40

#define true 1
#define false 0

// Debug information
#define DEBUG true
// Show Register Fifo messages
#define DEBUG_REGISTER DEBUG && true
// Show Request messages
#define DEBUG_REQUEST DEBUG && true
// Show I/O opening and closing
#define DEBUG_IO DEBUG && true
// Show server operations
#define DEBUG_OP DEBUG && true
