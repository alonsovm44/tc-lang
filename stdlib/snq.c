#include "snq.h"

Queue queue_create(int capacity) {
    Queue q = {0};
    if (capacity <= 0) capacity = 8;
    q.capacity = capacity;
    q.items = NULL;
    q.head = 0;
    q.tail = 0;
    q.count = 0;
    q.elem_size = 0;
    return q;
}

void queue_destroy(Queue *q) {
    if (!q) return;
    free(q->items);
}

void queue_push(Queue *q, void *item, int elem_size) {
    if (!q || !item || elem_size <= 0) return;
    if (!q->items) {
        q->elem_size = elem_size;
        q->items = calloc((size_t)q->capacity, (size_t)elem_size);
        if (!q->items) return;
    }
    if (q->elem_size != elem_size) return;
    if (q->count >= q->capacity) {
        int new_capacity = q->capacity * 2;
        void *new_items = calloc((size_t)new_capacity, (size_t)elem_size);
        if (!new_items) return;
        for (int i = 0; i < q->count; i++) {
            char *src = (char *)q->items + (size_t)(((q->head + i) % q->capacity) * q->elem_size);
            char *dst = (char *)new_items + (size_t)(i * q->elem_size);
            memcpy(dst, src, (size_t)q->elem_size);
        }
        free(q->items);
        q->items = new_items;
        q->capacity = new_capacity;
        q->head = 0;
        q->tail = q->count;
    }
    char *dst = (char *)q->items + (size_t)(q->tail * q->elem_size);
    memcpy(dst, item, (size_t)q->elem_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
}

void *queue_pop(Queue *q) {
    if (!q || q->count == 0 || !q->items) return NULL;
    char *src = (char *)q->items + (size_t)(q->head * q->elem_size);
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return src;
}

void *queue_peek(Queue *q) {
    if (!q || q->count == 0 || !q->items) return NULL;
    return (char *)q->items + (size_t)(q->head * q->elem_size);
}

int queue_size(Queue *q) {
    if (!q) return 0;
    return q->count;
}

void queue_clear(Queue *q) {
    if (!q) return;
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int queue_isEmpty(Queue *q) {
    if (!q) return 1;
    return q->count == 0;
}

Stack stack_create(int capacity) {
    Stack s = {0};
    if (capacity <= 0) capacity = 8;
    s.capacity = capacity;
    s.items = NULL;
    s.top = 0;
    s.elem_size = 0;
    return s;
}

void stack_destroy(Stack *s) {
    if (!s) return;
    free(s->items);
}

void stack_push(Stack *s, void *item, int elem_size) {
    if (!s || !item || elem_size <= 0) return;
    if (!s->items) {
        s->elem_size = elem_size;
        s->items = calloc((size_t)s->capacity, (size_t)elem_size);
        if (!s->items) return;
    }
    if (s->elem_size != elem_size) return;
    if (s->top >= s->capacity) {
        int new_capacity = s->capacity * 2;
        void *new_items = calloc((size_t)new_capacity, (size_t)elem_size);
        if (!new_items) return;
        memcpy(new_items, s->items, (size_t)s->capacity * (size_t)s->elem_size);
        free(s->items);
        s->items = new_items;
        s->capacity = new_capacity;
    }
    char *dst = (char *)s->items + (size_t)(s->top * s->elem_size);
    memcpy(dst, item, (size_t)s->elem_size);
    s->top++;
}

void *stack_pop(Stack *s) {
    if (!s || s->top == 0 || !s->items) return NULL;
    s->top--;
    return (char *)s->items + (size_t)(s->top * s->elem_size);
}

void *stack_peek(Stack *s) {
    if (!s || s->top == 0 || !s->items) return NULL;
    return (char *)s->items + (size_t)((s->top - 1) * s->elem_size);
}

int stack_size(Stack *s) {
    if (!s) return 0;
    return s->top;
}

void stack_clear(Stack *s) {
    if (!s) return;
    s->top = 0;
}

int stack_isEmpty(Stack *s) {
    if (!s) return 1;
    return s->top == 0;
}
