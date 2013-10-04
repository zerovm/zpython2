#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

#define DEFAULT_WORKING_DIRPATH "/"

int chdir(const char *path) {
	// allow chdir only to '/' according to getcwd.c
	if (strcmp(path, DEFAULT_WORKING_DIRPATH)) {
		errno = ENOSYS;
		return -1;
	}  	
  	return 0; 	
}

pid_t waitpid(pid_t pid, int *status, int options) {
  	errno = ENOSYS;
  	return -1;
}

#define UNAME_SYSNAME "zerovm"
#define UNAME_RELEASE ""
#define UNAME_VERSION ""
#define UNAME_MACHINE "zerovm"

/* Put information about the system in NAME.  */
int uname (struct utsname *buf)
{
  if (buf == NULL)
    {
      errno = (EFAULT);
      return -1;
    }

  strncpy (buf->sysname, UNAME_SYSNAME, sizeof (buf->sysname));
  strncpy (buf->release, UNAME_RELEASE, sizeof (buf->release));
  strncpy (buf->version, UNAME_VERSION, sizeof (buf->version));
  strncpy (buf->machine, UNAME_MACHINE, sizeof (buf->machine));

  return 0;
}