#!/usr/bin/python

import os
import sys
import subprocess
import socket
import tempfile
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-f', '--file', help='file containing tests list')
args = parser.parse_args()
# will use it as return code for script
test_result = 0

devnull = open(os.devnull, "w")

PATH = os.path.abspath(os.path.dirname(__file__))
TEST_DIR = os.path.join(PATH, 'Lib', 'test')

NVRAM_TMPLT = """[args]
args = python /dev/1.test.py
[fstab]
channel=/dev/1.python.tar,mountpoint=/,access=ro,removable=no
"""

MANIFEST_TMPLT = """Job = %(socket)s
Node = 1
Version = 20130611
Timeout = 50
Memory = 4294967296,0
Program = %(path)s/python
Channel = /dev/stdin,/dev/stdin,0,0,4294967296,4294967296,0,0
Channel = /dev/null,/dev/stdout,0,0,0,0,4294967296,4294967296
Channel = /dev/null,/dev/stderr,0,0,0,0,4294967296,4294967296
Channel = %(path)s/python.tar,/dev/1.python.tar,3,0,4294967296,4294967296,4294967296,4294967296
Channel = %(path)s/nvram.1,/dev/nvram,3,0,4294967296,4294967296,4294967296,4294967296
Channel = %(test_path)s/%(test)s.py,/dev/1.test.py,3,0,4294967296,4294967296,4294967296,4294967296
"""

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

if args.file:
    tests = [l for l in open(args.file, 'r').readlines()]


def client(server_address, input):
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    data = ''
    try:
        sock.connect(server_address)
        sdata = input
        size = '0x%06x' % (len(sdata))
        sock.sendall(size + sdata)
        resp = sock.makefile()
        sdata = resp.read(8)
        size = int(sdata, 0)
        data = resp.read(size)
    except IOError, e:
        print str(e)
        raise
    finally:
        sock.close()
    return data


class Daemon(object):
    def __enter__(self):
        # self.socket = os.path.join(PATH, 'tmp1234')
        # self.fd, self.socket = 0, '/tmp/tmp.Mkba0cwcdk'
        self.fd, self.socket = tempfile.mkstemp()
        self._start_daemon()
        return self

    def __exit__(self, type, value, traceback):
        self._stop_daemon()
        os.remove(self.socket)
        return False

    def send(self, test):
        params = {'socket': self.socket, 'path': PATH, 'test_path': TEST_DIR,
                  'test': test}
        self.manifest = MANIFEST_TMPLT % params
        return client(self.socket, self.manifest)

    def _start_daemon(self):
        with open(os.path.join(PATH, 'manifest.1'), 'w') as mfile:
            params = {'socket': self.socket, 'path': PATH,
                      'test_path': TEST_DIR, 'test': ''}
            self.manifest = MANIFEST_TMPLT % params
            mfile.write(self.manifest)

        with open(os.path.join(PATH, 'nvram.1'), 'w') as nfile:
            params = {'test': ''}
            nfile.write(NVRAM_TMPLT % params)

        subprocess.call(['zerovm', os.path.join(PATH, 'manifest.1')],
                        stdout=devnull, stderr=devnull)

    def _stop_daemon(self):
        subprocess.call(['pkill', 'zvm'])

with Daemon() as daemon:
    for test in tests:
        print("%s.." % test.strip()[5:]),
        sys.stdout.flush()
        try:
            ret = daemon.send(test.strip())
            retcode = int(ret.splitlines()[2])
            if retcode:
                test_result = 1
                print('\033[1;31mfail\033[1;m')
            else:
                print('\033[1;32mok\033[1;m')

        except KeyboardInterrupt:
            break


devnull.close()
sys.exit(test_result)
