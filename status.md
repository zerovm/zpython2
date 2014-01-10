#ZPython 2.7.3 status

##Basic features

No known issues.

##Modules

Modules testing is based on regression tests included in python distribution. 
See available tests in _python-lib-dir/python2.7/test_.

### Production

+ time 5/15 failures
	+ No system `sleep` implementation. `Function not implemented` error.
	+ Two failing tests (test\_gmtime\_without\_arg, test\_localtime\_without\_arg) due to zerovm/zrt time implementation mechanism.  
	+ Skipped test (test_clock) due to absense `clock()` function.
+ datetime: 2/240 failures
	+ No system `sleep` implementation. `Function not implemented` error.
+ cpickle 4/195 failures
	+ No `statvfs` implementation.
+ quopri 2/10 failures
	+ No `popen` implementation.
+ random 2/60 failures
	+ No `sleep` implementation.
+ argparse 72/1596 failures
	+ No `chdir` implementation.
+ unicodedata 1/22 failures
	+ No `pipe` implementation.
+ 2to3 1/577 failures
	+	No `os.system` routine.
+ runpy 4/17 failures
	+ `Unlink/rmdir` bug.
+ cookielib 1/55 failures
	+ One failing test due to time implementation mechanism.
+ py_compile 1/3 failures
	+ No `chdir` implementation.
+ sys 3/25 failures
	+ No `pipe` impplementation.
+ platform 2/20
	+	No `symlink` implementation.
	+ No executable availiable under zerovm (only through /dev/self)
+ pickle 2/104 failures
	+ No `statvfs` implementation
+ pickletools 1/53 failures
	+ No `statvfs` implementation

### Experimental

+ posix: 18/38 failures

	Relies heavily on zrt. Most of features is experimental.

	+ Bunch of `Function not implemented` errors. Have to stub them in glibc to remove from testing or replace with simple implementations.
	+ `chown` not raising `OSError`. Experimental ZRT feature.
+ os: 16/64 failures
	+ No `dup`, `utime`, `tcsetpgrp`, `datasync`, `pipe`, `symlink` implementations.
	+ No `/dev/null`, `/dev/urandom` devices.
+ os.path 9/33 failures
	+ No `symlink`, `chdir` implementation.
+ dummy_threading: 1/15 failures
	+ No `sleep` support.
+ mmap: 13/19 failures
	
	Very limited functionality. Almost unsupported feature. 

	+ No large file support 

+ hotshot 1/6 failures
	+ No `getrusage` implementation.
+ tarfile 24/241 failures
	+ No `symlink`, `chdir` implementation.
+ gzip 10/18 failures
	+ No `fsync` implementation.
+ uuid 2/14 failures
	+ No `pipe`, `gethostbyname` implementation.
+ import 6/23 failures
	+ No `pipe`, `utime` implementation.
	+ Won't import *.pyc files without atime/ctime/utime support.
+ shutil 17/33 failures
	+ No `mkfifo`, `chdir`, `symlink` implementations.
+ trace 2/14 failures
	+ `fcntl` zrt issue
+ hash 12/24 failures
	+ No `pipe` implementation.
+ warning 10/71 failures
	+ No `pipe` implementation.
+ sysconfig 1/10 failures
	+ No `symlink` implementation.
+ pkgutil 1/5 failures
	+ ZIP does not support timestamps before 1980
+ pydoc 5/15 failures
	+ No `pipe` support.
+ traceback 1/11 failures
	+ No `sleep` implementation.

### Unsupported

+ subprocess
+ popen, popen2
+ curses
+ signal
+ pipes
+ Tkinter
+ mailbox
	+ Tests fail due to zrt issue #67. 
	+ No `gethostname` implementation.
+ aetools/aepack/aetypes
+ telnetlib
+ threading
+ httpservers
+ queue
+ poplib
+ winreg
+ docxmlrpc
+ imaplib
+ ftplib
+ urllib2/urllib
+ httplib
+ socket
+ smtplib
+ ctypes
	+	Not yet. `libffi` needs to pe ported first.
+ cd
+ readline
+ cl
+ sv
+ al
+ bsddb
+ gl
+ ssl
	+ No `sockets`.
+ sunaudiodev
+ winsound
+ multiprocessing
+ nis
+ macos
+ dbm/gdbm/anydbm
+ imgfile
+ msilib
+ ossaudiodev
+ linuxaudiodev
+ dl
	+ No dynamic linking support in ZeroVM.
+ commands
+ asyncore
+ zipimport
	+ ZIP does not support timestamps before 1980
+ pdb 
+ macpath
+ resource
+ pty
+ gdb
+ ntpath
+ termios
+ dircache
	+ Deprecated since 2.6
+ glob
	+ No `symlink` implementation.
+ mhlib
+ mimetools
+ pwd
+ zip
+ zipfile
+ select

##Failing regression test list

We've run all tests available under ZeroVM. Test list with brief description follows.

```
# failed or skipped tests for cpython2.7 on zerovm/zrt
# current progress: 144/392 failed tests

io module testing:
# test_largefile # function signal not implemented
# test_fileio # no pipe support, seek issues
# test_file # 99% working, AssertionError: File pos after ftruncate wrong 11 
# test_io # seek/ftruncate issues, no signal, pipe support
# test_grp # unknown error
# test_zipfile64 # test requires loads of disk-space bytes and a long time to run
# test_tempfile.py # no pipe, unlink, remove support
```