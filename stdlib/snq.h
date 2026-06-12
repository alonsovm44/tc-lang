#ifndef SNQ_H
#define SNQ_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Queue structure (no threads)
typedef struct Queue {
    void *items;
    int capacity;
    int head;
    int tail;
    int count;
    int elem_size;
} Queue;

// Stack structure (no threads)
typedef struct Stack {
    void *items;
    int capacity;
    int top;
    int elem_size;
} Stack;

// Queue functions (no threads)
Queue queue_create(int capacity);
void queue_destroy(Queue *q);
void queue_push(Queue *q, void *item, int elem_size);
void *queue_pop(Queue *q);
void *queue_peek(Queue *q);
int queue_size(Queue *q);
void queue_clear(Queue *q);
int queue_isEmpty(Queue *q);

// Stack functions (no threads)
Stack stack_create(int capacity);
void stack_destroy(Stack *s);
void stack_push(Stack *s, void *item, int elem_size);
void *stack_pop(Stack *s);
void *stack_peek(Stack *s);
int stack_size(Stack *s);
void stack_clear(Stack *s);
int stack_isEmpty(Stack *s);

#endif