
#include "GUSIInternal.h"
#include "GUSIDescriptor.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>


static FILE * findfile()
{
	FILE * 		stream;
	
	for (stream = _iob; stream < _iob+_NFILE; ++stream)
		if (!(stream->_flag & (_IOREAD | _IOWRT | _IORW)))
			return stream;
	
	return NULL;
}

static FILE *fdreopen(int fd, short flags, FILE* stream)
{
	stream->_cnt 	= 0;
	stream->_ptr 	= NULL;
	stream->_base 	= NULL;
	stream->_end 	= NULL;
	stream->_size 	= NULL;
	stream->_flag 	= flags;
	stream->_file 	= fd;
	
	return stream;
}

static int mode2flags(const char * mode, int * openflags, int * stdioflags)
{
	bool	read_write 	= mode[1] == '+' || mode[2] == '+';
	
	*openflags	= 0;
	*stdioflags	= 0;
	
	switch (mode[0]) {
	case 'r':
		*openflags	|= read_write ? O_RDWR : O_RDONLY;
		*stdioflags	|= read_write ? _IORW : _IOREAD;
		break;
	case 'w':
		*openflags	|= (read_write ? O_RDWR : O_WRONLY) | O_CREAT | O_TRUNC;
		*stdioflags	|= read_write ? _IORW : _IOWRT;
		break;
	case 'a':
		*openflags	|= (read_write ? O_RDWR : O_WRONLY) | O_CREAT | O_APPEND;
		*stdioflags	|= read_write ? _IORW : _IOWRT;
		break;
	default:
		return -1;
	}
	
	return 0;
}

FILE *fopen(const char *filename, const char *mode)
{
	FILE *	stream;
	int	 	flags;
	int		ioflags;
	int 	fd;
	
	if ((stream = findfile()) 
	 && mode2flags(mode, &flags, &ioflags) >= 0 
	 && (fd = open(filename, flags)) >= 0
	) 
		return fdreopen(fd, ioflags, stream);
	else
		return NULL;
}

FILE *freopen(const char *filename, const char *mode, FILE *stream)
{
	int	 	flags;
	int		ioflags;
	int 	fd;
	
	flags = errno;
	fclose(stream);
	errno = flags;
	
	if (mode2flags(mode, &flags, &ioflags) >= 0 
	 && (fd = open(filename, flags)) >= 0
	) 
		return fdreopen(fd, ioflags, stream);
	else
		return NULL;
}

FILE *fdopen(int fd, const char *mode)
{
	FILE *	stream;
	int	 	flags;
	int		ioflags;

	if ((stream = findfile()) 
	 && mode2flags(mode, &flags, &ioflags) >= 0  
	)
		return fdreopen(fd, ioflags, stream);
	else
		return NULL;
}


void GUSIStdioClose()
{
	for (FILE * f = _iob; f<_iob+_NFILE; ++f)
		if (f->_flag & (_IOREAD|_IOWRT))
			fclose(f);
}

void GUSIStdioFlush() { fflush(NULL);	}

static void * sDontStrip;

void GUSISetupConsoleStdio()
{
	sDontStrip = ioctl;
}

