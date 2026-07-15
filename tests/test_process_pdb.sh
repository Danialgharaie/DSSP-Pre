#!/bin/bash
set -e

# Setup clean test environment
rm -rf tests/run_out tests/tmp_inplace
mkdir -p tests/tmp_inplace
cp tests/samples/test_simple.pdb tests/tmp_inplace/file1.pdb
cp tests/samples/test_simple.pdb tests/tmp_inplace/file2.pdb

echo "Building project..."
make clean && make

echo "Running Test 1: Single file in-place (regression)..."
./process_pdb tests/tmp_inplace/file1.pdb
if ! grep -q "HEADER" tests/tmp_inplace/file1.pdb; then
    echo "FAIL: Test 1 - HEADER missing"
    exit 1
fi
echo "PASS: Test 1"

echo "Running Test 2: Multiple files in-place..."
# Reset file1.pdb
cp tests/samples/test_simple.pdb tests/tmp_inplace/file1.pdb
./process_pdb tests/tmp_inplace/file1.pdb tests/tmp_inplace/file2.pdb
if ! grep -q "HEADER" tests/tmp_inplace/file1.pdb || ! grep -q "HEADER" tests/tmp_inplace/file2.pdb; then
    echo "FAIL: Test 2 - Bulking in-place failed"
    exit 1
fi
echo "PASS: Test 2"

echo "Running Test 3: Multiple files to output directory..."
# Reset
cp tests/samples/test_simple.pdb tests/tmp_inplace/file1.pdb
cp tests/samples/test_simple.pdb tests/tmp_inplace/file2.pdb
./process_pdb -o tests/run_out tests/tmp_inplace/file1.pdb tests/tmp_inplace/file2.pdb
if [ ! -f tests/run_out/file1.pdb ] || [ ! -f tests/run_out/file2.pdb ]; then
    echo "FAIL: Test 3 - Files not outputted to directory"
    exit 1
fi
if ! grep -q "HEADER" tests/run_out/file1.pdb; then
    echo "FAIL: Test 3 - Preprocessing failed"
    exit 1
fi
echo "PASS: Test 3"

echo "Running Test 4: Exit codes and missing files..."
set +e
./process_pdb tests/tmp_inplace/file1.pdb non_existent.pdb 2>/dev/null
EXIT_CODE=$?
set -e
if [ $EXIT_CODE -ne 1 ]; then
    echo "FAIL: Test 4 - Did not return 1 on partial failure"
    exit 1
fi
echo "PASS: Test 4"

echo "All tests passed successfully!"
