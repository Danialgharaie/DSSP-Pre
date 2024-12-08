# Process PDB

This tool processes a PDB file to ensure that certain records (`HEADER`, `CRYST1`, `MODEL`, `ENDMDL`, and `END`) are present and correctly ordered. It modifies the input PDB file in-place.

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
./process_pdb path_to_pdb_file
```

The tool will process the specified PDB file in place, ensuring all required records are present and properly ordered.

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

- It’s recommended to keep a backup of your original PDB file before running the tool, as it overwrites the input file.
- Adjust compilation flags in the `makefile` if you need different optimization settings or broader compatibility.
