#ZPython 2.7.3 status

##Basic features

No known issues.

##Modules

Modules testing is based on regression tests included in python distribution. 
See available tests in _python-lib-dir/python2.7/test_.

### Production

+ time 5/15 failures
	+ No system `sleep` implementation. `Function not implemented` error.
	+ Two failing tests (test_gmtime_without_arg, test_localtime_without_arg) due to zerovm/zrt time implementation mechanism.  
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


### Experimental

+ posix: 18/38 failures

	Relies heavily on zrt. Most of features is experimental.

	+ Bunch of `Function not implemented` errors. Have to stub them in glibc to remove from testing or replace with simple implementations.
	+ `chown` not raising `OSError`. Experimental ZRT feature.
+ os: 16/64 failures
	+ No `dup`, `utime`, `tcsetpgrp`, `datasync`, `pipe`, `symlink` implementations.
	+ No `/dev/null`, `/dev/urandom` devices.
+ dummy_threading: 1/15 failures
	+ No `sleep` support.
+ mmap: 13/19 failures
	
	Very limited functionality. Almost unsupported feature. 

	+ No large file support 

+ hotshot
	+ No `getrusage` implementation.

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
+ resource



##Failing regression test list

We've run all available tests under ZeroVM. Test list with brief description follows.

```
# failed or skipped tests for cpython2.7 on zerovm/zrt
# current progress: 144/392 failed tests

# test_wait4 test_fork1 test_wait3 # no module named thread
# test_codecmaps_hk test_codecmaps_tw test_codecmaps_jp test_codecmaps_cn test_codecmaps_kr # couldn't retreive ...

io module testing:
# test_largefile # function signal not implemented
# test_fileio # no pipe support, seek issues
# test_file # 99% working, AssertionError: File pos after ftruncate wrong 11 
# test_io # memory/MemNode.cc:105: void MemNode::ReallocData(int): Assertion `len > 0' failed.


# test_cmd_line_script # no pipe support
# test_compileall # unknown error, research needed
# test_poll # no pipe support



# test_grp # unknown error
# test_tarfile # 90% works, fs issues
# test_py3kwarhn # must be run with -3 flag
# test_zipfile64 # test requires loads of disk-space bytes and a long time to run
# test_gzip # no fsync, fileno support
# test_lib2to3 # 99% works, unknown errors
# test_macpath # no chdir support
# test_pty # no pipe, signal support
# test_select # no select, popen support
# test_epoll # test works only on linux 2.6
# test_gdb # no gdb on path
# test_ntpath # no chdir support
# test_runpy # fs issues
# test_unittest # no signal support, unknown errors
# test_uuid # no gethostbyname gethostname support
# test_ascii_formatd # no module named _ctypes
# test_startfile # module os has no attribute statfile
# test_unicode_file # No Unicode filesystem semantics on this platform
# test_cgi.py # fs issues with tmpfiles, unlink
# test_cookielib # 1 tests failied: datetime issue
# test_import # no pipe, utime support, unknown errors
# test_openpty # no ideas
# test_posixpath # no chdir, symlink support
# test_py_compile # no chdir support
# test_shutil # no PATH environment var
# test_smtplib # most test skipped due to no threading support, no gethostname support
# test_socket # no _realsocket getaddrinfo support
# test_sys # no pipe support, unknown error
# test_trace # rmdir error, unknown error
# test_ioctl # no /dev/tty 
# test_cmd_line # no pipe support, no /dev/null
# test_dircache # seems tmp dir issue
# test_hash # no pipe support
# test_platform # no symlink support, unknown error
# test_warnings # no pipe support
# test_glob # no symlink support

# test_mhlib # no utime support, unknown error
# test_mimetools # no support gethostbyname gethostname
# test_pickle test_pickletools # almost working, no statvfs support
# test_pwd # unknown error
# test_sysconfig # no symlink support
# test_pep277 # only NT+ and systems with Unicode-friendly filesystem encoding 
# test_pkgutil # ZIP does not support timestamps before 1980, no remove support
# test_pydoc # no pipe support
# test_traceback # no sleep support
# test_zipimport_support @ no pipe, remove support
# test_tempfile.py # no pipe, unlink, remove support
# test_zipfile.py # no unlink support
```