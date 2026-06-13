#ifndef TIGHTC_IO_H
#define TIGHTC_IO_H

char *read_file(const char *path);
char *try_read_file(const char *path);
void write_file(const char *path, const char *data);

#endif