
#ifndef __ERRORS_H__
#define __ERRORS_H__

#define ERROR_HANDLER_AND_DIE(err)                           \
    {                                                        \
        printf("[ERR] The error description is: %s\n", err); \
        exit(1);                                             \
    }

#endif