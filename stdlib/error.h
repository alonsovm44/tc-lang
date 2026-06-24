// Error handling functions
/// try catch runtime
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>


// Error handling state
static jmp_buf *g_current_try_buf = NULL;
static char g_current_error_name[256] = {0};

void tc_set_try_buf(jmp_buf *buf) {
    g_current_try_buf = buf;
}

jmp_buf *tc_get_try_buf(void) {
    return g_current_try_buf;
}

void tc_throw(const char *error_name) {
    if (g_current_try_buf) {
        strncpy(g_current_error_name, error_name, sizeof(g_current_error_name) - 1);
        g_current_error_name[sizeof(g_current_error_name) - 1] = '\0';
        longjmp(*g_current_try_buf, 1);
    } else {
        fprintf(stderr, "Uncaught error: %s\n", error_name);
        exit(1);
    }
}

const char *tc_error_name(void) {
    return g_current_error_name;
}