#include "error.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// rip globals
int error_counter = 0;

void inner_nlog(int line, char* file, char* string, ...) {
    va_list args;
    va_start(args, string);

    printf("[%s:%i] => ", file, line);
    vprintf(string, args);
    printf("\n");
    fflush(stdout);

    va_end(args);
}

int inner_error(int line, char* file, int err, char* string, ...) {
    va_list args;
    va_start(args, string);

    printf("ERROR [%s:%i] => ", file, line);
    vprintf(string, args);
    printf("\n");
    fflush(stdout);

    va_end(args);
    
    error_counter += 1;
    return error_counter;
}

// TODO: map error ID to associated error message.
char* get_error_msg(int errorid) {
    assert(0);
}

// TODO: map error ID to associated error type.
char get_error_type(int errorid) {
    assert(0);
}

