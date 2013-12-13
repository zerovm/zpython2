#ZPython 2.7.3 status

##Basic features

No known issues.

##Modules

Modules testing is based on regression tests included in python distribution. 
See available tests in _python-lib-dir/python2.7/test_.

+ time 5/15 failures
	+ No system `sleep` implementation. `Function not implemented` error.
	+ Two failing tests (test_gmtime_without_arg, test_localtime_without_arg) due to zerovm/zrt time implementation mechanism.  
	+ Skipped test (test_clock) due to absense `clock()` function.
+ datetime: 2/240 failures
	+ No system `sleep` implementation. `Function not implemented` error.

##Failing regression test list

We've run all available tests under ZeroVM. Test list with brief description follows.

```
# failed or skipped tests for cpython2.7 on zerovm/zrt
# current progress: 144/392 failed tests

# test_posix # zrt fs feature lacking
# test_cpickle # 99% tests working, failinf due to zrt fs feature lacking
# test_mailbox # zrt fs feature lacking (tmp files)
# test_time # fails due to zerovm platform constraints, some unknown failures
# test_aepack test_scriptpackages # no moudle named aetools
# test_telnetlib test_threading_local test_wait4 test_thread test_httpservers 
	test_queue test_fork1 test_poplib test_winreg test_threadsignals test_docxmlrpc
	test_wait3 test_imaplib test_threaded_import test_threadedtempfile 
	test_threading test_ftplib test_urllib2_localnet # no module named thread
# test_cd test_cl test_readline test_al test_bsddb185 test_ssl test_ctypes 
	test_gl test_sunaudiodev test_winsound test_crypt test_multiprocessing test_mmap
	test_nis test_future_builtins test_bsddb test_bsddb3 test_macos test_applesingle 
	test_macostools test_hotshot test_dbm test_imgfile test_msilib test_dl 
	test_ossaudiodev test_linuxaudiodev # no modules
# test_quopri # no pipe support
# test_smtpnet # no SSL_SMTP attribute
# test_urllib2net test_urllibnet #  no sleep support, non-recoverable failure in name resolution.
# test_timeout # no _realsocket support
# test_file2k # no pipe support
# test_os # no module name mmap
# test_urllib2 # 90% working, non-recoverable failure in name resolution
# test_codecmaps_hk test_codecmaps_tw test_codecmaps_jp test_codecmaps_cn test_codecmaps_kr # couldn't retreive ...
# test_gdbm # no module named gdb
# test_commands # no popen support
# test_httplib # function _realsocket not implemented
# test_largefile # function signal not implemented
# test_random # sleep not supported
# test_strtod # correctly-rounded string->float conversions not available on this system
# test_ttk_guionly test_tcl test_ttk_textonly test_tk # no module named _tkinter 

# test_argparse # no tmpfile support
# test_asyncore # no _realsocket support, no dup support
# test_cmd_line_script # no pipe support
# test_compileall # unknown error, research needed
# test_poll # no pipe support
# test_unicodedata # no pipe support
# test_xpickle # no statvfs support
# test_zipimport #  ZIP does not support timestamps before 1980
# test_file # 99% working, AssertionError: File pos after ftruncate wrong 11 
# test_fileio # no pipe support, zrt fs feature lacking, unknown errors
# test_grp # unknown error
# test_pdb # no pipe support
# test_resource # no getrlimit support
# test_tarfile # 90% works, fs issues
# test_py3kwarn # must be run with -3 flag
# test_zipfile64 # test requires loads of disk-space bytes and a long time to run
# test_datetime # 99% works, no sleep support
# test_dummy_thread # no sleep support
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
# test_pipes # no popen support, unknown errors
# test_platform # no symlink support, unknown error
# test_popen # no popen support
# test_popen2 # no pipe support
# test_signal # no signal support
# test_warnings # no pipe support
# test_curses # curses resource not enabled  
# test_glob # no symlink support
# test_io # memory/MemNode.cc:105: void MemNode::ReallocData(int): Assertion `len > 0' failed.
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
# test_subprocess.py # sigsegv
```