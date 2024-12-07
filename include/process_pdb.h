#ifndef PROCESS_PDB_H
#define PROCESS_PDB_H

#include <stdbool.h>

#define MAX_LINES 10000
#define MAX_LINE_LEN 256

bool starts_with(const char *line, const char *prefix);
bool line_equals_trimmed(const char *line, const char *ref);
void fix_header_and_model(char **lines, int *nlines);
void insert_cryst1(char **lines, int *nlines);

#endif // PROCESS_PDB_H

