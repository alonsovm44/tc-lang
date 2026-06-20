#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <setjmp.h>
#define TC_ALLOC(type, count) ((type *)calloc((count), sizeof(type)))
#define TC_LENOF(x) (sizeof(x) / sizeof((x)[0]))
#define TC_FAT_LENOF(x) ((x).len)


typedef struct { char * *ptr; size_t len; } tc_fat_ptr;

struct ParsedResult {
    Queue cmds;
    Stack vars;
    Shell shell;
};

typedef enum {
    SH_BASH,
    SH_ZSH,
    SH_FISH,
    SH_POWERSHELL
} Shell;

struct Command {
    char *name;
    tc_fat_ptr body;
    Shell shell;
};

struct Variable {
    char *name;
    char *value;
};

void error_E001(int32_t *line) {
    printf("Error E001 in line %d: expected whitespace after 'var'\n", (*line));
}

void error_E002(int32_t *line) {
    printf("Error E002 in line %d: expected '=' after variable name\n", (*line));
}

void error_E003(int32_t line) {
    printf("Error E003 in line %d: expected \" after string value\n", line);
}

void error_E004(int32_t line) {
    printf("Error E004 in line %d: unknown shell found\n", line);
}

void error_E005(int32_t line) {
    printf("Error E005 in line %d: command name cannot be blank\n", line);
}

void error_E006(int32_t line) {
    printf("Syntax Error: On line %d Expected ':' after command name\n", line);
}

struct Variable parse_variable(Queue q, int32_t *line_ptr);
Shell parse_shell(Queue q, int32_t *line_ptr);
struct Command parse_command(Queue tokens, int32_t *line_ptr, Shell shell);
struct ParsedResult parse(Queue tokens);


struct Variable parse_variable(Queue q, int32_t *line_ptr) {
    struct Variable var = {0};
    if ((*(char **)queue_peek(&q) == " ")) {
        *(char **)queue_pop(&q);
    } else {
        error_E001(line_ptr);
        tc_throw("E001");
    }
    (var.name = *(char **)queue_pop(&q));
    if ((*(char **)queue_peek(&q) == " ")) {
        *(char **)queue_pop(&q);
    } else {
        error_E002(line_ptr);
        tc_throw("E002");
    }
    if ((*(char **)queue_peek(&q) == "=")) {
        *(char **)queue_pop(&q);
    } else {
        error_E002(line_ptr);
        tc_throw("E002");
    }
    if ((*(char **)queue_peek(&q) == "\"")) {
        *(char **)queue_pop(&q);
    } else {
        int32_t numericValue = *(char **)queue_pop(&q);
    }
    char *varValue = *(char **)queue_pop(&q);
    if ((*(char **)queue_peek(&q) == "\"")) {
        *(char **)queue_pop(&q);
    } else {
        error_E003(line_ptr);
        tc_throw("E003");
    }
    
    return var;
}

Shell parse_shell(Queue q, int32_t *line_ptr) {
    Shell shell = {0};
    if ((*(char **)queue_peek(&q) == "bash")) {
        (shell = 0);
    } else if ((*(char **)queue_peek(&q) == "zsh")) {
        (shell = 1);
    } else if ((*(char **)queue_peek(&q) == "fish")) {
        (shell = 2);
    } else if ((*(char **)queue_peek(&q) == "powershell")) {
        (shell = 3);
    } else {
        error_E004(line_ptr);
        tc_throw("E004");
    }
    
    return shell;
}

struct Command parse_command(Queue tokens, int32_t *line_ptr, Shell shell) {
    struct Command c = {0};
    (c.shell = shell);
    (c.name = *(char **)queue_pop(&tokens));
    if ((c.name == "")) {
        error_E005(line_ptr);
        tc_throw("E005");
    }
    char *colon = *(char **)queue_peek(&tokens);
    if ((colon == ":")) {
        *(char **)queue_pop(&tokens);
    } else {
        error_E006(line_ptr);
        tc_throw("E006");
    }
    tc_fat_ptr LineArray = {0};
    int32_t k = 0;
    while ((*(char **)queue_peek(&tokens) != ">")) {
        char *tok = *(char **)queue_pop(&tokens);
        if (((tok == ";") || (tok == "\n"))) {
            (k++);
        }
        scat(LineArray.ptr[k], tok);
    }
    (c.body = LineArray);
    
    return c;
}

struct ParsedResult parse(Queue tokens) {
    struct ParsedResult result = {0};
    Shell shell = {0};
    (result.vars = stack_create(0));
    (result.cmds = queue_create(0));
    int32_t len = queue_size(&tokens);
    int32_t line = 1;
    int32_t i = 0;
    while ((i < len)) {
        {
            jmp_buf _try_buf;
            jmp_buf *_old_buf = tc_get_try_buf();
            tc_set_try_buf(&_try_buf);
            int _try_result = setjmp(_try_buf);
            if (_try_result == 0) {
                (shell = parse_shell(tokens, &line));
            } else {
                if (strcmp(tc_error_name(), "E004") == 0) {
                    return 1;
                }
                else {
                    printf("Unknown shell parsing error at line %d\n", line);
                }
            }
            tc_set_try_buf(_old_buf);
        }
        char *token = *(char **)queue_pop(&tokens);
        if ((token == "\n")) {
            (line++);
        }
        if ((token == "#")) {
            while ((token != "\n")) {
                *(char **)queue_pop(&tokens);
                (i++);
            }
        }
        if (((token == "var") && (inGlobalScope() == 1))) {
            struct Variable var = {0};
            {
                jmp_buf _try_buf;
                jmp_buf *_old_buf = tc_get_try_buf();
                tc_set_try_buf(&_try_buf);
                int _try_result = setjmp(_try_buf);
                if (_try_result == 0) {
                    (var = parse_variable(tokens, &line));
                } else {
                    if (strcmp(tc_error_name(), "E001") == 0) {
                        return 1;
                    }
                    if (strcmp(tc_error_name(), "E002") == 0) {
                        return 1;
                    }
                    if (strcmp(tc_error_name(), "E003") == 0) {
                        return 1;
                    }
                    else {
                        printf("Unknown parse error in line: &d", line);
                        
                        return (-99);
                    }
                }
                tc_set_try_buf(_old_buf);
            }
            stack_push(&result.vars, &var, sizeof(struct Variable));
        }
        if ((token == "<")) {
            struct Command c = {0};
            {
                jmp_buf _try_buf;
                jmp_buf *_old_buf = tc_get_try_buf();
                tc_set_try_buf(&_try_buf);
                int _try_result = setjmp(_try_buf);
                if (_try_result == 0) {
                    (c = parse_command(tokens, &line, shell));
                } else {
                    if (strcmp(tc_error_name(), "E005") == 0) {
                        return 1;
                    }
                    if (strcmp(tc_error_name(), "E006") == 0) {
                        return 1;
                    }
                    else {
                        printf("Unknown command parsing error at line %d\n", line);
                        
                        return (-99);
                    }
                }
                tc_set_try_buf(_old_buf);
            }
            queue_push(&result.cmds, &c, sizeof(struct Command));
        }
        (i++);
    }
    
    return result;
}
