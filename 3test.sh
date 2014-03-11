#!/usr/bin/python

import os
import sys
import subprocess

FILE = 'tests.txt'
devnull = open(os.devnull, "w")

# tests from external file
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
        print('\033[1;31mfail\033[1;m')
    else:
        print('\033[1;32mok\033[1;m')

devnull.close()
