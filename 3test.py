#!/usr/bin/python

import os
import sys
import subprocess
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-f', '--file', help='file containing tests list')
args = parser.parse_args()
# will use it as return code for script
test_result = 0


FILE = args.file
devnull = open(os.devnull, "w")

# predefined tests
tests = [
    'test_grammar',
    'test_opcodes',
    'test_dict',
    'test_builtin',
    'test_exceptions',
    'test_types',
    'test_unittest',
    'test_doctest',
    'test_doctest2',
]


# tests from external file
if FILE:
    with open(FILE, 'r') as rfile:
        tests = rfile.read().splitlines()

for test in tests:
    print("%s..." % test.strip()),
    sys.stdout.flush()
    try:
        ret = subprocess.call(['zvsh', '--zvm-image', 'python.tar',
                               'python', '-mtest.regrtest', test],
                              stdout=devnull, stderr=devnull)
    except KeyboardInterrupt:
        break
    if ret:
        test_result = 1
        print('\033[1;31mfail\033[1;m')
    else:
        print('\033[1;32mok\033[1;m')

devnull.close()
sys.exit(test_result)
