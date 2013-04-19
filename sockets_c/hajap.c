#include "hajap.h"

// define static function
static void err_doit(int errnoflag, const char *fmt, va_list ap);

/* Handle errors. */
void err_sys(const char *fmt, ...) {
   va_list ap;
   
   va_start(ap, fmt);
   err_doit(1, fmt, ap);
   va_end(ap);
   
   exit(1);
}


/* Handle errors. */
void err_quit(const char *fmt, ...) {
   va_list   ap;
   
   va_start(ap, fmt);
   err_doit(0, fmt, ap);
   va_end(ap);
   
   exit(1);
}


/* Handle errors. */
static void err_doit(int errnoflag, const char *fmt, va_list ap) {
   int   errno_save;
   char  buf[MAXLINE];
   
   errno_save = errno;   /* value caller might want printed */
   vsprintf(buf, fmt, ap);
   
   if(errnoflag)
     sprintf(buf+strlen(buf), ": %s", strerror(errno_save));
   
   strcat(buf, "\n");
   fflush(stdout);   /* in case stdout and stderr are the same */
   fputs(buf, stderr);
   fflush(stderr);   /* SunOS 4.1.* doesn't grok NULL argument */
   
   return;
}

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n) {
   size_t    nleft;
   ssize_t   nwritten;
   const char  *ptr;
   
   ptr = vptr;
   nleft = n;
   while (nleft > 0) {
		 if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			 if (errno == EINTR)
				 nwritten = 0;   /* and call write() again */
			 else
				 return(-1);     /* error */
			 }

			 nleft -= nwritten;
			 ptr   += nwritten;
   }
   
   return(n);
}

void Writen(int fd, void *ptr, size_t nbytes) {
	if (writen(fd, ptr, nbytes) != (int)nbytes)
		err_sys("writen error");
}

/* UDP uses this. */
void Write(int fd, void *ptr, size_t nbytes)
{
	if (write(fd, ptr, nbytes) != (ssize_t)nbytes)
		err_sys("write error");
}

/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *vptr, size_t n) {
	size_t  nleft;
	ssize_t nread;
	char    *ptr;

	ptr = vptr;
	nleft = n;

	while(nleft > 0) {
		if( (nread = read(fd, ptr, nleft)) < 0) {
			if(errno == EINTR)
				nread = 0; // and call read() again
			else
				return(-1);
		}
		else if(nread == 0)
			break; // EOF

		nleft -= nread;
		ptr += nread;
	}

	return(n - nleft); // return >= 0
}

ssize_t Readn(int fd, void *ptr, size_t nbytes) {
	ssize_t   n;

	if ( (n = readn(fd, ptr, nbytes)) < 0)
		err_sys("readn error");

	return(n);
}

ssize_t Read(int fd, void *ptr, size_t nbytes) {
	ssize_t   n;

	if ( (n = read(fd, ptr, nbytes)) == -1)
		err_sys("read error %d\t", errno);

	return(n);
}
