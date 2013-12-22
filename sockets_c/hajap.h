#ifndef HAJAP_H
#define HAJAP_H

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>

#define MAXLINE 512

/* Handle errors. */
void err_quit(const char *fmt, ...);

/* Handle errors. */
void err_sys(const char *fmt, ...);

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n);

/* Handle errors of writen here. */
void Writen(int fd, void *ptr, size_t nbytes);

void Write(int fd, void *ptr, size_t nbytes);

/* Read "n" bytes from a descriptor. */
ssize_t readn(int fd, void *vptr, size_t n);

/* Handle errors of readn here. */
ssize_t Readn(int fd, void *ptr, size_t nbytes);

/* read for UDP with error handle. */
ssize_t Read(int fd, void *ptr, size_t nbytes);

#endif
