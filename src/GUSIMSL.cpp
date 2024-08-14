/*
#include "GUSIInternal.h"
#include "GUSIMSL.h"
#include "GUSIDescriptor.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

GUSI_USING_STD_NAMESPACE

extern __file_modes __temp_file_mode;

extern "C"
{

	FILE *__find_unopened_file();
	// __handle_reopen is declared in <stdio.h>

	void __flush_all(void);

	int __open_file(const char *name, __file_modes mode, __file_handle *handle);

	int __open_temp_file(__file_handle *handle);

	int __read_file(
		__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc);

	int __read_console(
		__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc);

	int __write_file(
		__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc);

	int __write_console(
		__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc);

	int __position_file(
		__file_handle handle, unsigned long *position, int mode, __idle_proc idle_proc);

	int __close_file(__file_handle handle);

	int __close_console(__file_handle handle);
}

int __open_file(const char *name, __file_modes mode, __file_handle *handle)
{
	int fd;
	int posixmode;

	<< Translate [[mode]] to [[posixmode]] >>

		fd = open(name, posixmode);

	if (fd == -1)
		return __io_error;

	*handle = (unsigned)fd;

	return __no_io_error;
}
int __open_temp_file(__file_handle *handle)
{
	char temp_name[L_tmpnam];
	FSSpec spec;
	int ioresult;

	__temp_file_name(temp_name, &spec);

	ioresult = __open_file(temp_name, __temp_file_mode, handle);

	if (ioresult == __no_io_error)
	{
		unlink(temp_name);
	}

	return (ioresult);
}
int __read_file(
	__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc)
{
	int result = read((int)handle, (char *)buffer, (int)*count);

	if (result < 0)
	{
		*count = 0;

		return __io_error;
	}
	else
	{
		*count = result;

		return result ? __no_io_error : __io_EOF;
	}
}
int __read_console(
	__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc)
{
	return __read_file(handle, buffer, count, idle_proc);
}
int __write_file(
	__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc)
{
	int result = write((int)handle, (char *)buffer, (int)*count);

	if (result < 0)
	{
		*count = 0;

		return __io_error;
	}
	else
	{
		*count = result;

		return __no_io_error;
	}
}
static __file_handle GuessHandle(__file_handle handle, unsigned char *buffer)
{
	if (handle)
		return handle;
	if (buffer > stderr->buffer && (buffer - stderr->buffer) < stderr->buffer_size)
		return 2;
	return 1;
}

int __write_console(
	__file_handle handle, unsigned char *buffer, size_t *count, __idle_proc idle_proc)
{
	return __write_file(GuessHandle(handle, buffer), buffer, count, idle_proc);
}
int __position_file(
	__file_handle handle, unsigned long *position, int mode, __idle_proc)
{
	long result = lseek((int)handle, *position, mode);

	if (result < 0)
		if (errno == ESPIPE && !*position && mode != SEEK_END)
			*position = 0;
		else
			return __io_error;
	else
		*position = result;

	return __no_io_error;
}
int __close_file(__file_handle handle)
{
	return close((int)handle) < 0 ? __io_error : __no_io_error;
}
int __close_console(__file_handle handle)
{
	return __close_file(handle);
}

#if __MSL__ >= 0x6000
#define fdopen _fdopen
#endif
#if __MSL__ >= 0x7000
#define FDOPEN_TYPE_CONST const
#else
#define FDOPEN_TYPE_CONST
#endif
FILE *fdopen(int fildes, FDOPEN_TYPE_CONST char *type)
{
	FILE *str;

	if (!(str = __find_unopened_file()))
		return (0);

	return (__handle_reopen(fildes, type, str));
}

static void MSLSetupStdio(int fd, FILE *f)
{
	f->handle = fd;
	f->position_proc = __position_file;
	f->read_proc = __read_file;
	f->write_proc = __write_file;
	f->close_proc = __close_file;
}

void GUSISetupConsoleStdio()
{
	MSLSetupStdio(0, stdin);
	MSLSetupStdio(1, stdout);
	MSLSetupStdio(2, stderr);
}
*/