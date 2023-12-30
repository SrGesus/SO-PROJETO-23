#ifndef COMMON_OP_CODE_H
#define COMMON_OP_CODE_H

enum Operation {
    // OP_CODE = '1'
    SETUP = '1',
    // OP_CODE = '2'
    QUIT,
    // OP_CODE = '3'
    CREATE,
    // OP_CODE = '4'
    RESERVE,
    // OP_CODE = '5'
    SHOW,
    // OP_CODE = '6'
    LIST,
};

#endif // COMMON_OP_CODE_H
