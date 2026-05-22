#include "io.h"

#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) die("cannot open %s: %s", path, strerror(errno));
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = xmalloc((size_t)n + 1);
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) die("cannot read %s", path);
    buf[n] = 0;
    fclose(f);
    return buf;
}

void write_file(const char *path, const char *data) {
    FILE *f = fopen(path, "wb");
    if (!f) die("cannot write %s: %s", path, strerror(errno));
    fwrite(data, 1, strlen(data), f);
    fclose(f);
}
