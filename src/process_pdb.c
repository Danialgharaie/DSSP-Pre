#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libgen.h>
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

bool process_file(const char *input_path, const char *output_dir) {
    FILE *fp = fopen(input_path, "r");
    if (!fp) {
        perror(input_path);
        return false;
    }

    char *lines[MAX_LINES];
    int nlines = 0;
    char buffer[MAX_LINE_LEN];

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (nlines >= MAX_LINES) {
            fprintf(stderr, "Error: File %s too large to process\n", input_path);
            fclose(fp);
            for (int i = 0; i < nlines; i++) free(lines[i]);
            return false;
        }
        lines[nlines++] = strdup(buffer);
    }
    fclose(fp);

    fix_header_and_model(lines, &nlines);
    insert_cryst1(lines, &nlines);

    char output_path[MAX_LINE_LEN * 2];
    if (output_dir == NULL) {
        strncpy(output_path, input_path, sizeof(output_path) - 1);
        output_path[sizeof(output_path) - 1] = '\0';
    } else {
        // Extract basename securely
        char temp_path[MAX_LINE_LEN];
        strncpy(temp_path, input_path, sizeof(temp_path) - 1);
        temp_path[sizeof(temp_path) - 1] = '\0';
        char *base = basename(temp_path);
        snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, base);
    }

    fp = fopen(output_path, "w");
    if (!fp) {
        perror(output_path);
        for (int i = 0; i < nlines; i++) free(lines[i]);
        return false;
    }

    for (int i = 0; i < nlines; i++) {
        fputs(lines[i], fp);
        free(lines[i]);
    }
    fclose(fp);
    return true;
}

int main(int argc, char **argv) {
    const char *output_dir = NULL;
    const char *input_files[MAX_LINES];
    int num_files = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_dir = argv[i+1];
                i++;
            } else {
                fprintf(stderr, "Error: -o option requires an argument\n");
                return 1;
            }
        } else {
            if (num_files >= MAX_LINES) {
                fprintf(stderr, "Error: Too many input files\n");
                return 1;
            }
            input_files[num_files++] = argv[i];
        }
    }

    if (num_files == 0) {
        fprintf(stderr, "Usage: %s [-o <output_dir>] <pdb_file_1> [<pdb_file_2> ...]\n", argv[0]);
        return 1;
    }
    
    // Temporary check to compile and verify parsing
    (void)input_files;
    printf("Output dir: %s, File count: %d\n", output_dir ? output_dir : "None", num_files);
    return 0;
}

