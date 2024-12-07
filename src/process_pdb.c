#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "process_pdb.h"

// Default PDB records - must maintain exact spacing
static const char *default_header = "HEADER    GENERATED FILE                            01-DEC-24\n";
static const char *default_model  = "MODEL        1\n";
static const char *default_endmdl = "ENDMDL\n";
static const char *default_end    = "END\n";
static const char *default_cryst1 = "CRYST1    1.000    1.000    1.000  90.00  90.00  90.00 P 1           1\n";

bool starts_with(const char *line, const char *prefix) {
    while (*prefix && *line && (*line == *prefix)) {
        line++;
        prefix++;
    }
    return *prefix == '\0';
}

bool line_equals_trimmed(const char *line, const char *ref) {
    char temp[MAX_LINE_LEN];
    size_t len = strlen(line);
    while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' ')) {
        len--;
    }
    strncpy(temp, line, len);
    temp[len] = '\0';
    return strcmp(temp, ref) == 0;
}

void fix_header_and_model(char **lines, int *nlines) {
    bool has_header = false;
    for (int i = 0; i < *nlines; i++) {
        if (starts_with(lines[i], "HEADER")) {
            has_header = true;
            break;
        }
    }

    // Insert HEADER if not present
    if (!has_header && *nlines < MAX_LINES) {
        memmove(lines + 1, lines, sizeof(char*) * (*nlines));
        lines[0] = strdup(default_header);
        (*nlines)++;
    }

    // Check for MODEL
    bool has_model = false;
    for (int i = 0; i < *nlines; i++) {
        if (starts_with(lines[i], "MODEL")) {
            has_model = true;
            break;
        }
    }

    // If no MODEL, insert MODEL before first ATOM and append ENDMDL
    if (!has_model) {
        int atom_index = -1;
        for (int i = 0; i < *nlines; i++) {
            if (starts_with(lines[i], "ATOM")) {
                atom_index = i;
                break;
            }
        }

        if (atom_index != -1 && *nlines + 1 < MAX_LINES) {
            memmove(lines + atom_index + 1, lines + atom_index, sizeof(char*) * (*nlines - atom_index));
            lines[atom_index] = strdup(default_model);
            (*nlines)++;
            if (*nlines < MAX_LINES) {
                lines[*nlines] = strdup(default_endmdl);
                (*nlines)++;
            }
        }
    }

    // Ensure END and ENDMDL order
    bool has_end = false;
    bool has_endmdl = false;
    int end_index = -1;
    int endmdl_index = -1;

    for (int i = 0; i < *nlines; i++) {
        if (line_equals_trimmed(lines[i], "END")) {
            has_end = true;
            end_index = i;
        }
        if (line_equals_trimmed(lines[i], "ENDMDL")) {
            has_endmdl = true;
            endmdl_index = i;
        }
    }

    // If no END, append it
    if (!has_end) {
        if (*nlines < MAX_LINES) {
            lines[*nlines] = strdup(default_end);
            (*nlines)++;
            has_end = true;
            end_index = (*nlines) - 1;
        }
    }

    if (!has_endmdl) {
        // Insert ENDMDL before END if END exists
        if (has_end && end_index != -1 && *nlines < MAX_LINES) {
            memmove(lines + end_index + 1, lines + end_index, sizeof(char*) * (*nlines - end_index));
            lines[end_index] = strdup(default_endmdl);
            (*nlines)++;
            // Re-locate END
            for (int i = 0; i < *nlines; i++) {
                if (line_equals_trimmed(lines[i], "END")) {
                    end_index = i;
                    break;
                }
            }
        }
    } else {
        // Ensure ENDMDL is before END
        if (has_end && has_endmdl && end_index != -1 && endmdl_index != -1 && endmdl_index > end_index) {
            char *temp = lines[endmdl_index];
            memmove(lines + endmdl_index, lines + endmdl_index + 1, sizeof(char*) * (*nlines - endmdl_index - 1));
            (*nlines)--;
            if (*nlines < MAX_LINES) {
                memmove(lines + end_index + 1, lines + end_index, sizeof(char*) * (*nlines - end_index));
                lines[end_index] = temp;
                (*nlines)++;
            }
        }
    }
}

void insert_cryst1(char **lines, int *nlines) {
    bool has_cryst1 = false;
    for (int i = 0; i < *nlines; i++) {
        if (starts_with(lines[i], "CRYST1")) {
            has_cryst1 = true;
            break;
        }
    }

    if (!has_cryst1) {
        int model_index = -1;
        for (int i = 0; i < *nlines; i++) {
            if (starts_with(lines[i], "MODEL")) {
                model_index = i;
                break;
            }
        }

        if (model_index != -1 && *nlines < MAX_LINES) {
            memmove(lines + model_index + 1, lines + model_index, sizeof(char*) * (*nlines - model_index));
            lines[model_index] = strdup(default_cryst1);
            (*nlines)++;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pdb_file_path>\n", argv[0]);
        return 1;
    }

    const char *file_path = argv[1];
    FILE *fp = fopen(file_path, "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    char *lines[MAX_LINES];
    int nlines = 0;
    char buffer[MAX_LINE_LEN];

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (nlines >= MAX_LINES) {
            fprintf(stderr, "File too large to process\n");
            fclose(fp);
            return 1;
        }
        lines[nlines] = strdup(buffer);
        nlines++;
    }
    fclose(fp);

    // Process lines
    fix_header_and_model(lines, &nlines);
    insert_cryst1(lines, &nlines);

    // Write back to file
    fp = fopen(file_path, "w");
    if (!fp) {
        perror("Failed to open file for writing");
        for (int i = 0; i < nlines; i++) {
            free(lines[i]);
        }
        return 1;
    }

    for (int i = 0; i < nlines; i++) {
        fputs(lines[i], fp);
        free(lines[i]);
    }

    fclose(fp);

    return 0;
}

