#! /bin/csh -f
set TEST_HOME = ${PWD}/tests
set source_file = pzip.c
set binary_file = pzip
set bin_dir = ${TEST_HOME}/bin
set test_dir = ${TEST_HOME}/tests-pzip

${bin_dir}/serialized_runner.py -1 ${bin_dir}/generic-tester.py -c -s $source_file -b $binary_file -t $test_dir -f="'-pthread -O'"
