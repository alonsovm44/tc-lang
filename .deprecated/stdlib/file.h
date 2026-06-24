#pragma once

typedef void *FILE;

FILE *fopen_read(const char *path);
FILE *fopen_write(const char *path);
FILE *fopen_append(const char *path);
int fclose(FILE *file);
size_t fread(void *ptr, size_t size, size_t count, FILE *file);
size_t fwrite(void *ptr, size_t size, size_t count, FILE *file);
int fseek(FILE *file, long offset, int whence);
long ftell(FILE *file);
int feof(FILE *file);
int ferror(FILE *file);
