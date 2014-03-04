#!/usr/bin/python

import os
import sys
import subprocess

FILE = 'tests.txt'
devnull = open(os.devnull, "w")

with open(FILE, 'r') as rfile:
    for test in rfile:
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
