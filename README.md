# Process PDB

This tool processes PDB files to ensure that certain records (`HEADER`, `CRYST1`, `MODEL`, `ENDMDL`, and `END`) are present and correctly ordered. It supports bulk processing of multiple PDB files, either modifying the inputs in-place or outputting the results to a specified directory.

## Features

- Adds a `HEADER` record if missing.
- Adds a `CRYST1` record before the first `MODEL` record if missing.
- Ensures a `MODEL` record precedes `ATOM` records.
- Ensures `ENDMDL` and `END` records are present and in the correct order.

## Requirements

- A C compiler supporting the specified optimization flags.
- The project uses `-O3 -flto -march=native` for improved performance.

## Building

Run `make` in the project directory:

```sh
make
```

This will produce an executable named `process_pdb`.

### makefile Flags

- **-O3:** High-level optimization for speed.
- **-flto:** Link-time optimization.
- **-march=native:** Optimizes the code for your CPU’s features.

## Usage

```sh
./process_pdb [-o output_dir] file1.pdb [file2.pdb ...]
```

The tool processes one or more specified PDB files. By default, it processes them in-place, ensuring all required records are present and properly ordered.

### Options

- **`-o output_dir`**: Specifies an output directory.
  - **Auto-directory creation**: If the output directory does not exist, it is created automatically.
  - **Path flattening**: All output files are written directly into this directory, flattening the input directory structure (e.g., `dir/subdir/file.pdb` becomes `output_dir/file.pdb`).
  - **Overwrite warning**: The tool will overwrite files in the output directory if they share a name with the input file's basename, without displaying a warning.

### Warnings

- **In-place modifications**: Running the tool without the `-o` option modifies/overwrites the input PDB files directly. It is highly recommended to keep a backup of your original PDB files.

### Usage Examples

- **Single file in-place**:
  ```sh
  ./process_pdb input.pdb
  ```
- **Multiple files in-place**:
  ```sh
  ./process_pdb file1.pdb file2.pdb file3.pdb
  ```
- **Shell wildcard / bulk processing (in-place)**:
  ```sh
  ./process_pdb path/to/pdbs/*.pdb
  ```
- **Process files and output to a target directory**:
  ```sh
  ./process_pdb -o processed_files file1.pdb path/to/file2.pdb
  ```
- **Combine wildcard and output directory**:
  ```sh
  ./process_pdb -o output_dir *.pdb
  ```

### Exit Status and Error Handling

The tool returns the following exit codes:
- **`0`**: All files were processed successfully.
- **`1`**: One or more errors occurred (e.g., file not found, directory creation failed, invalid argument, or input file exceeded size limits).

All errors and failure logs are written to standard error (`stderr`). The tool continues processing subsequent files in the list even if a prior file fails to process.

## Project Structure

```
DSSP-Pre/
├── makefile
├── README.md
├── include/
│   └── process_pdb.h
└── src/
    └── process_pdb.c
```

- **makefile:** Builds the `process_pdb` executable.
- **README.md:** Instructions and usage details.
- **include/process_pdb.h:** Declarations of functions and constants.
- **src/process_pdb.c:** Implementation of the logic to process the PDB file.

## Notes

- Adjust compilation flags in the `makefile` if you need different optimization settings or broader compatibility.

## To-Do

- [x] Add support for bulk processing of multiple PDB files.
